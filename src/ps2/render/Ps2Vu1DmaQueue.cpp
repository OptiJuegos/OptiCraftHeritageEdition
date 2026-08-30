#include "ps2/render/Ps2Vu1DmaQueue.h"

#ifdef PS2_PLATFORM

#include <algorithm>
#include <cstdint>
#include <cstring>

#include <dma.h>
#include <ee_regs.h>

#include "platform/Log.h"

// Defined in Ps2TerrainRenderer.cpp: how far the opaque replay had got when the
// transfer was submitted, or -1 outside a replay.
extern "C" int ps2_dbg_terrain_section();

namespace
{
    static const unsigned int kDmaTagEnd = 7;

    // DMA channel control: bit 8 (STR) stays set while the transfer runs.
    static const unsigned int kChcrRunning = 0x100u;

    // Upper bound on a single VIF1 transfer, in EE core cycles. The core runs
    // at 294.912MHz, so 294912 cycles is one millisecond and this is ~300ms:
    // two orders of magnitude above the longest legitimate terrain packet, and
    // still short enough that a stall reads as one dropped frame instead of a
    // dead console. It stays well inside the ~14.6s wrap of COP0 Count, so the
    // unsigned delta below needs no wrap handling.
    static const unsigned int kVifWaitCycles = 294912u * 300u;

    // Raw COP0 Count. Unlike eeCycles() below this is needed in every build,
    // because the timeout is a correctness guard rather than instrumentation.
    static inline unsigned int eeCount()
    {
        unsigned int cycles;
        __asm__ __volatile__("mfc0 %0, $9" : "=r"(cycles));
        return cycles;
    }

    static inline unsigned long eeCycles()
    {
#ifdef PS2_RENDER_STATS
        return eeCount();
#else
        return 0;
#endif
    }

    static Ps2Vu1ResidentProgram s_residentProgram = PS2_VU1_PROGRAM_NONE;
}

Ps2Vu1ResidentProgram ps2_vu1_resident_program()
{
    return s_residentProgram;
}

void ps2_vu1_set_resident_program(Ps2Vu1ResidentProgram program)
{
    s_residentProgram = program;
}

void ps2_vu1_dma_recover()
{
    // Abandoning the transfer on the EE side is not enough, and getting this
    // wrong is worse than not recovering at all.
    //
    // A VU1 microprogram that never reaches its [E] bit keeps running, and a
    // running VU1 owns GIF Path1. The GIF will not hand Path3 to anyone while
    // Path1 is held, so the very next gsKit_queue_exec() -> dmaKit_wait_fast()
    // in Display_ps2.cpp spins forever -- the renderer falls back to VU0
    // correctly and then dies one frame later on the GIF instead, showing a few
    // half-drawn frames on the way down.
    //
    // So the hardware has to be put back: halt the channel, force-break VU1,
    // reset VIF1 (which clears its FIFO, its stall state and the error latch),
    // then hand the channel back to libdma in a known state.
    *R_EE_D1_CHCR = 0;
    *R_EE_VIF1_FBRST = 0x2; // FBK: force-break the running microprogram
    *R_EE_VIF1_FBRST = 0x1; // RST: reset VIF1
    *R_EE_VIF1_ERR = 0;

    // VIF1's reset also clears VU1 micro memory, so whatever program a backend
    // last uploaded is gone along with it.
    s_residentProgram = PS2_VU1_PROGRAM_NONE;

    // Breaking VU1 mid-XGKICK is not a clean stop: the GIF was part-way through
    // a Path1 packet and is still counting down that GIFtag's NLOOP. It has no
    // idea the sender is gone, so every qword gsKit pushes down Path3 afterwards
    // is swallowed as the missing payload of the abandoned packet -- the frame
    // loop keeps running and nothing reaches the GS. On screen that is a sky
    // colour with no terrain and no HUD, which is strictly worse than the hang
    // it replaced because it looks like a rendering bug rather than a stall.
    //
    // Resetting the GIF drops the partial packet and its path arbitration. GS
    // registers live in the GS, not the GIF, so the frame/z buffer setup and
    // everything gsKit programmed survives this. Path3 is idle by construction
    // here: acquire_path1() drains the gsKit queue before taking Path1.
    *R_EE_GIF_CTRL = 0x1; // RST

    // Write-one-to-clear: drop the channel-1 interrupt the aborted transfer
    // leaves latched, or the next dma_channel_* call inherits it.
    *R_EE_D_STAT = 0x2;
    __asm__ __volatile__("sync.l; sync.p" ::: "memory");
    dma_channel_initialize(DMA_CHANNEL_VIF1, nullptr, 0);
}

bool ps2_vu1_dma_wait(const char* site)
{
    const unsigned int begin = eeCount();
    while ((*R_EE_D1_CHCR & kChcrRunning) != 0u)
    {
        if (eeCount() - begin < kVifWaitCycles)
            continue;

        // Which half stopped is readable from this dump.
        //   CHCR running with QWC 0   -> the DMAC finished its list and VIF1 is
        //                                waiting on VU1: a microprogram that
        //                                never reached its [E] bit.
        //   QWC non-zero              -> the transfer itself is stuck; MADR/TADR
        //                                say where in the chain.
        //   VIF1_ERR / VIF1_STAT bits -> a malformed VIFcode; VIF1_CODE holds
        //                                the offending one and VIF1_NUM what is
        //                                left of its data count.
        //   GIF_STAT / D2_CHCR        -> whether Path1 is still held, which is
        //                                what would hang the next frame.
        MC_LOG_ERROR("render",
            "[PS2] VIF1 stalled in %s (opaque section %d):"
            " CHCR=%08x MADR=%08x QWC=%08x TADR=%08x"
            " STAT=%08x ERR=%08x CODE=%08x NUM=%08x TOPS=%08x ITOP=%08x"
            " GIF_STAT=%08x D2_CHCR=%08x D_STAT=%08x\n",
            site ? site : "?", ps2_dbg_terrain_section(),
            (unsigned int)*R_EE_D1_CHCR, (unsigned int)*R_EE_D1_MADR,
            (unsigned int)*R_EE_D1_QWC, (unsigned int)*R_EE_D1_TADR,
            (unsigned int)*R_EE_VIF1_STAT, (unsigned int)*R_EE_VIF1_ERR,
            (unsigned int)*R_EE_VIF1_CODE, (unsigned int)*R_EE_VIF1_NUM,
            (unsigned int)*R_EE_VIF1_TOPS, (unsigned int)*R_EE_VIF1_ITOP,
            (unsigned int)*R_EE_GIF_STAT, (unsigned int)*R_EE_D2_CHCR,
            (unsigned int)*R_EE_D_STAT);

        ps2_vu1_dma_recover();
        return false;
    }

    // The DMAC is done, but the EE may still hold buffered writes and stale
    // cache lines for the memory it just read. Every caller goes on to reuse
    // that memory, so the barrier belongs here rather than at each site.
    ps2_vu1_dma_flush_ucab();
    return true;
}

Ps2Vu1DmaQueue::Ps2Vu1DmaQueue()
    : m_used{ 0, 0 }, m_writePage(0), m_dmaInFlight(false), m_faulted(false),
      m_stats{ 0, 0, 0, 0, 0 }
{
}

Ps2VifQword* Ps2Vu1DmaQueue::writePage()
{
    const std::uintptr_t address = reinterpret_cast<std::uintptr_t>(m_pages[m_writePage]);
    return reinterpret_cast<Ps2VifQword*>(address | 0x30000000u);
}

Ps2VifQword* Ps2Vu1DmaQueue::reserve(int qwords)
{
    if (qwords <= 0 || qwords >= kPageQwords ||
        m_used[m_writePage] + qwords + 1 > kPageQwords)
        return nullptr;

    Ps2VifQword* result = writePage() + m_used[m_writePage];
    m_used[m_writePage] += qwords;
    return result;
}

bool Ps2Vu1DmaQueue::waitForDma()
{
    if (!m_dmaInFlight)
        return true;

#ifdef PS2_RENDER_STATS
    const unsigned long begin = eeCycles();
#endif
    if (!ps2_vu1_dma_wait("queue"))
    {
        m_faulted = true;
        return false;
    }
#ifdef PS2_RENDER_STATS
    m_stats.waitCycles += eeCycles() - begin;
    ++m_stats.waits;
#endif
    m_dmaInFlight = false;
    return true;
}

bool Ps2Vu1DmaQueue::submit()
{
    int& used = m_used[m_writePage];
    if (used <= 0)
        return true;
    if (!waitForDma())
        return false;

    Ps2VifQword* page = writePage();
    Ps2VifQword& endTag = page[used++];
    endTag.w[0] = kDmaTagEnd << 28;
    endTag.w[1] = 0;
    endTag.w[2] = 0;
    endTag.w[3] = 0;

    ps2_vu1_dma_flush_ucab();
    if (dma_channel_send_chain_ucab(DMA_CHANNEL_VIF1, page, 0, 0) < 0)
    {
        --used;
        return false;
    }

#ifdef PS2_RENDER_STATS
    ++m_stats.pages;
    m_stats.qwords += used;
    m_stats.maxPageQwords = std::max(m_stats.maxPageQwords, (long)used);
#endif
    m_dmaInFlight = true;
    used = 0;
    m_writePage ^= 1;
    return true;
}

bool Ps2Vu1DmaQueue::drain()
{
    return submit() && waitForDma();
}

void Ps2Vu1DmaQueue::discardPending()
{
    m_used[m_writePage] = 0;
}

void Ps2Vu1DmaQueue::takeStats(Ps2Vu1DmaQueueStats& out)
{
    out = m_stats;
    std::memset(&m_stats, 0, sizeof(m_stats));
}

#endif // PS2_PLATFORM
