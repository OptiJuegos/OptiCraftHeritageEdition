#pragma once

#ifdef PS2_PLATFORM

#include <cstdint>

struct Ps2VifQword
{
    std::uint32_t w[4];
};

static_assert(sizeof(Ps2VifQword) == 16, "VIF queue entries must be one quadword");

// ---- VIF1 submission primitives shared by every direct-terrain path ----
//
// Both of these exist because the EE behaves differently from every emulator
// this backend was developed against, and getting either one wrong produces the
// same symptom: VIF1 consumes something that is not what the EE wrote, executes
// it, and stops responding.
//
// ps2_vu1_dma_flush_ucab()
//   Every packet below is written through the uncached-accelerated mirror
//   (address | 0x30000000). UCAB stores do not go to memory directly: they
//   accumulate in the EE's 128-byte write-gather buffer, which drains only when
//   a full line is written, when a store crosses into a different line, or on
//   an explicit sync.l. A packet whose last line is partial -- a single-quadword
//   barrier, or the tail of a microprogram upload -- therefore sits in the WGB
//   while the DMAC reads stale memory underneath it. There is no write-gather
//   buffer in PCSX2, which is why this is invisible off-hardware.
//
// ps2_vu1_dma_wait()
//   A bounded replacement for dma_channel_wait(DMA_CHANNEL_VIF1, 0), whose zero
//   timeout means "spin forever". Nothing in this renderer can recover from that
//   and there is no watchdog anywhere, so a single VIF1 or VU1 stall used to
//   hang the console with no output at all. This returns false instead, after
//   printing the VIF1/DMAC state that identifies which of the two stopped.
static inline void ps2_vu1_dma_flush_ucab()
{
    __asm__ __volatile__("sync.l; sync.p" ::: "memory");
}

// site is a short literal naming the submission, printed with the register dump.
bool ps2_vu1_dma_wait(const char* site);

// Halt channel 1, force-break VU1 and reset VIF1. ps2_vu1_dma_wait() calls this
// on a timeout; it is the step that releases GIF Path1 so gsKit's Path3 frame
// submission can proceed. Exposed for a caller that aborts a submission for its
// own reasons and needs the same guarantee.
void ps2_vu1_dma_recover();

// ---- VU1 microprogram residency ----
//
// VU1 has exactly one microcode slot, shared by every backend that uploads a
// program to it (today, only the direct terrain path; a future direct-entity
// or generic-transform path would be another). A backend must not assume its
// own program stays resident once uploaded: whichever program some other
// backend last uploaded is the one VU1 will actually execute. Each backend's
// "did I already upload" check has to go through here instead of a private
// flag, and must re-upload whenever this reports anything other than its own
// program.
enum Ps2Vu1ResidentProgram
{
    PS2_VU1_PROGRAM_NONE = 0,
    PS2_VU1_PROGRAM_TERRAIN,
};

Ps2Vu1ResidentProgram ps2_vu1_resident_program();

// Called by a backend right after its own upload packet has been sent and
// waited on -- never before, so a failed upload cannot be mistaken for a
// resident one.
void ps2_vu1_set_resident_program(Ps2Vu1ResidentProgram program);

struct Ps2Vu1DmaQueueStats
{
    long pages;
    long qwords;
    long waits;
    long maxPageQwords;
    unsigned long waitCycles;
};

class Ps2Vu1DmaQueue
{
public:
    Ps2Vu1DmaQueue();

    Ps2VifQword* reserve(int qwords);
    bool submit();
    bool drain();
    bool waitForDma();
    void discardPending();

    int pendingQwords() const { return m_used[m_writePage]; }

    // Sticky: set the first time ps2_vu1_dma_wait() times out. submit() and
    // drain() also return false for ordinary reasons (a page that cannot hold
    // the request), so the caller needs this to tell a recoverable refusal from
    // hardware that has stopped answering.
    bool faulted() const { return m_faulted; }

    void takeStats(Ps2Vu1DmaQueueStats& out);

private:
    static const int kPageCount = 2;
    static const int kPageQwords = 512;

    Ps2VifQword* writePage();

    alignas(64) Ps2VifQword m_pages[kPageCount][kPageQwords];
    int m_used[kPageCount];
    int m_writePage;
    bool m_dmaInFlight;
    bool m_faulted;
    Ps2Vu1DmaQueueStats m_stats;
};

#endif // PS2_PLATFORM
