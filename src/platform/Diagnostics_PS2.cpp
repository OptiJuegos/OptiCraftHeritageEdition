#include "platform/Diagnostics.h"
#include "platform/Log.h"
#include "ps2/system/Ps2Heap.h"
#include "ps2/system/Ps2RemoteDebug.h"

extern "C" long ps2_dbg_ram_free_kb();
extern "C" long ps2_dbg_ram_used_kb();
extern "C" long ps2_dbg_malloc_used_kb();
extern "C" long ps2_dbg_malloc_free_kb();
extern "C" long ps2_dbg_malloc_free_blocks();

const char* platformOomDiagnosticLine(int index)
{
    return ps2HeapLastBadAllocLine(static_cast<unsigned>(index));
}

void platformMemoryCheckpoint(const char* tag)
{
    MC_LOG_DEBUG("memory", "%-24s used=%ldKB free=%ldKB mallocUsed=%ldKB mallocFree=%ldKB blocks=%ld\n",
                 tag != nullptr ? tag : "(null)",
                 ps2_dbg_ram_used_kb(), ps2_dbg_ram_free_kb(),
                 ps2_dbg_malloc_used_kb(), ps2_dbg_malloc_free_kb(),
                 ps2_dbg_malloc_free_blocks());
}

void platformHardwareCheckpoint(const char* tag)
{
    Ps2RemoteDebug::checkpoint(tag);
}

long platformHeapFreeKb()
{
    return ps2_dbg_ram_free_kb();
}

void platformCaptureBadAlloc()
{
    ps2HeapCaptureBadAlloc();
}
