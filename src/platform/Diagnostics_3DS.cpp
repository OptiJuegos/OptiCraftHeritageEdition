#include "platform/Diagnostics.h"

#include "platform/Log.h"

// The 3DS has no heap-tracker layer yet -- the Wii keeps one in WiiHeap, the
// PS2 in Ps2Heap -- so there is no bad_alloc site to quote. No 3DS call path
// dereferences this today: GuiErrorScreen's diagnostic block is #ifdef
// WII_PLATFORM and Ps2EarlyCrash is PS2-only.
const char* platformOomDiagnosticLine(int)
{
    return nullptr;
}

void platformMemoryCheckpoint(const char* tag)
{
    // No numbers to report yet, but the tag still marks WHERE the checkpoint
    // fired, which is what correlates the load-stage lines around it.
    MC_LOG_INFO("memory", "%-24s 3DS heap tracker not wired up yet\n",
                tag != nullptr ? tag : "(null)");
}

void platformHardwareCheckpoint(const char*)
{
    // No remote-debug channel on 3DS phase 1 (the PS2 routes this to
    // Ps2RemoteDebug); left a no-op so the shared call sites stay shared.
}

void platformCaptureBadAlloc()
{
    // Nothing to capture until a bad_alloc hook layer exists; see
    // platformOomDiagnosticLine above.
}

long platformHeapFreeKb()
{
    // The header's documented "no cheap answer" value, same as the Wii: there
    // is no free-heap figure here that WorldLoadTrace could read per stage
    // without walking the allocator, and a wrong number would be worse than
    // none.
    return -1;
}
