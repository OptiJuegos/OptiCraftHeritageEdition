#include "ps2/render/Ps2Vu1Path1.h"

#ifdef PS2_PLATFORM

#if defined(PS2_ENABLE_VU1_TERRAIN)

#include <algorithm>
#include <cstdint>
#include <cstring>

#include <dma.h>
#include <dmaKit.h>
#include <gsKit.h>

#include "platform/Log.h"
#include "ps2/render/Ps2Graphics.h"
#include "ps2/render/Ps2RenderBackend.h"
#include "ps2/render/Ps2Tuning.h"
#include "ps2/render/Ps2Vu1TerrainRuntime.h"

extern "C" {
extern unsigned char ps2Vu1Terrain_CodeStart[];
extern unsigned char ps2Vu1Terrain_CodeEnd[];
extern unsigned char ps2Vu1TerrainFast_CodeEnd[];
}

namespace
{
    static const unsigned int kVifNop = 0x00;
    static const unsigned int kVifFlusha = 0x13;
    static const unsigned int kVifMpg = 0x4a;

    static unsigned int s_microPacket[1024] __attribute__((aligned(64)));
    static unsigned int s_barrierPacket[4] __attribute__((aligned(64)));
    static bool s_vu1Disabled = false;
    static bool s_path1Owned = false;

    static inline unsigned int vifCode(unsigned int command, unsigned int num,
                                       unsigned int immediate)
    {
        return (command << 24) | ((num & 0xffu) << 16) | (immediate & 0xffffu);
    }

    static inline unsigned long eeCycles()
    {
#ifdef PS2_RENDER_STATS
        unsigned int cycles;
        __asm__ __volatile__("mfc0 %0, $9" : "=r"(cycles));
        return cycles;
#else
        return 0;
#endif
    }

    static bool waitDirectDma(const char* site)
    {
#ifdef PS2_RENDER_STATS
        const unsigned long begin = eeCycles();
#endif
        if (!ps2_vu1_dma_wait(site))
        {
            ps2_vu1_path1_disable(site);
            return false;
        }
#ifdef PS2_RENDER_STATS
        Ps2Vu1TerrainStats& stats = ps2_vu1_terrain_runtime_stats();
        stats.waitCycles += eeCycles() - begin;
        ++stats.waits;
#endif
        return true;
    }
}

bool ps2_vu1_path1_disabled()
{
    return s_vu1Disabled;
}

void ps2_vu1_path1_disable(const char* reason)
{
    if (s_vu1Disabled)
        return;

    s_vu1Disabled = true;
    ps2_vu1_set_resident_program(PS2_VU1_PROGRAM_NONE);
    s_path1Owned = false;
    ps2_vu1_terrain_runtime_invalidate_after_path_failure();
    ps2_render_invalidate_path1_state();
    MC_LOG_ERROR("render",
        "[PS2] direct VU1 terrain disabled for this session (%s);"
        " falling back to the VU0 path\n",
        reason ? reason : "?");
}

bool ps2_vu1_path1_send_normal_and_wait(const void* data, int qwords,
                                         const char* site)
{
    Ps2Vu1DmaQueue& queue = ps2_vu1_terrain_runtime_queue();
    if (qwords <= 0 || !queue.drain())
        return false;

    ps2_vu1_dma_flush_ucab();
    if (dma_channel_send_normal_ucab(DMA_CHANNEL_VIF1,
            const_cast<void*>(data), (unsigned int)qwords, 0) < 0)
        return false;
    return waitDirectDma(site);
}

bool ps2_vu1_path1_ensure_terrain_program()
{
    if (ps2_vu1_resident_program() == PS2_VU1_PROGRAM_TERRAIN)
        return true;

    const unsigned char* begin = ps2Vu1Terrain_CodeStart;
#if PS2_VU1_CLIPPED_PARTIALS || PS2_VU1_SIDE_CLIPPED_PARTIALS || PS2_VU1_CLIPPED_PROBE_BATCHES_PER_FRAME > 0 || PS2_VU1_TERRAIN_CANARY
    const unsigned char* end = ps2Vu1Terrain_CodeEnd;
#else
    const unsigned char* end = ps2Vu1TerrainFast_CodeEnd;
#endif
    const int byteCount = (int)(end - begin);
    if (byteCount <= 0 || (byteCount & 7) != 0)
        return false;

    const int instructionCount = byteCount / 8;
    if (instructionCount <= 0 || instructionCount > 2048)
        return false;

    volatile unsigned int* packet = reinterpret_cast<volatile unsigned int*>(
        reinterpret_cast<std::uintptr_t>(s_microPacket) | 0x30000000u);
    const int packetCapacityWords =
        (int)(sizeof(s_microPacket) / sizeof(s_microPacket[0]));
    // A failed partial upload must not leave an old program marked resident.
    ps2_vu1_set_resident_program(PS2_VU1_PROGRAM_NONE);
    int uploadedInstructions = 0;
    while (uploadedInstructions < instructionCount)
    {
        // One MPG command per DMA transfer: at most 255 instructions plus
        // its command and padding (512 words). Wait before reusing the same
        // fixed buffer; a full 2048-instruction program need not fit in it.
        int words = 0;
        const int batchInstructions = std::min(255,
            instructionCount - uploadedInstructions);
        const int batchWords = batchInstructions * 2;
        if (words + 1 + batchWords > packetCapacityWords)
            return false;

        packet[words++] = vifCode(kVifMpg,
            (unsigned int)batchInstructions,
            (unsigned int)uploadedInstructions);
        for (int i = 0; i < batchWords; ++i)
        {
            unsigned int word;
            std::memcpy(&word,
                begin + ((std::size_t)uploadedInstructions * 2u +
                         (std::size_t)i) * 4u,
                sizeof(word));
            packet[words++] = word;
        }
        while ((words & 3) != 0)
        {
            if (words >= packetCapacityWords)
                return false;
            packet[words++] = vifCode(kVifNop, 0, 0);
        }

        if (!ps2_vu1_path1_send_normal_and_wait(
                (const void*)packet, words / 4, "mpg"))
            return false;

        uploadedInstructions += batchInstructions;
    }

    ps2_vu1_set_resident_program(PS2_VU1_PROGRAM_TERRAIN);
    return true;
}

bool ps2_vu1_path1_send_flusha()
{
    Ps2Vu1DmaQueue& queue = ps2_vu1_terrain_runtime_queue();
    if (!queue.drain())
        return false;

    volatile unsigned int* packet = reinterpret_cast<volatile unsigned int*>(
        reinterpret_cast<std::uintptr_t>(s_barrierPacket) | 0x30000000u);
    packet[0] = vifCode(kVifFlusha, 0, 0);
    packet[1] = vifCode(kVifNop, 0, 0);
    packet[2] = vifCode(kVifNop, 0, 0);
    packet[3] = vifCode(kVifNop, 0, 0);

    ps2_vu1_dma_flush_ucab();
    if (dma_channel_send_normal_ucab(DMA_CHANNEL_VIF1,
            (void*)packet, 1, 0) < 0)
        return false;
    return waitDirectDma("flusha");
}

bool ps2_render_acquire_path1()
{
    if (s_path1Owned)
        return true;
    if (s_vu1Disabled || !gsGlobal || !ps2_vu1_path1_ensure_terrain_program())
        return false;

    Ps2Vu1DmaQueue& queue = ps2_vu1_terrain_runtime_queue();
    if (!queue.waitForDma())
    {
        ps2_vu1_path1_disable("acquire");
        return false;
    }

    const bool path3Pending =
        (gsGlobal->Per_Queue != nullptr && gsGlobal->Per_Queue->tag_size > 0) ||
        (gsGlobal->Os_Queue != nullptr && gsGlobal->Os_Queue->tag_size > 0);
    gsKit_queue_exec(gsGlobal);
    dmaKit_wait_fast();
    if (path3Pending)
        gsKit_finish();
    gsKit_queue_reset(gsGlobal->Os_Queue);

    s_path1Owned = true;
#ifdef PS2_RENDER_STATS
    ++ps2_vu1_terrain_runtime_stats().pathTransitions;
#endif
    return true;
}

void ps2_render_barrier_path1()
{
    if (s_path1Owned)
        (void)ps2_vu1_path1_send_flusha();
}

void ps2_render_release_path1()
{
    if (!s_path1Owned)
        return;

    (void)ps2_vu1_path1_send_flusha();
    s_path1Owned = false;
    ps2_vu1_terrain_runtime_invalidate_common_state();
#ifdef PS2_RENDER_STATS
    ++ps2_vu1_terrain_runtime_stats().pathTransitions;
#endif
    ps2_render_invalidate_path1_state();
}

#else

bool ps2_vu1_path1_disabled() { return true; }
void ps2_vu1_path1_disable(const char*) {}
bool ps2_vu1_path1_ensure_terrain_program() { return false; }
bool ps2_vu1_path1_send_normal_and_wait(const void*, int, const char*) { return false; }
bool ps2_vu1_path1_send_flusha() { return false; }
bool ps2_render_acquire_path1() { return false; }
void ps2_render_barrier_path1() {}
void ps2_render_release_path1() {}

#endif // PS2_ENABLE_VU1_TERRAIN
#endif // PS2_PLATFORM
