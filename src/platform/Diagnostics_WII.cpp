#include "platform/Diagnostics.h"
#include "wii/WiiHeap.h"

const char* platformOomDiagnosticLine(int index)
{
    return wiiHeapLastBadAllocLine(index);
}

void platformMemoryCheckpoint(const char* tag)
{
    wiiHeapReport(tag);
}

void platformHardwareCheckpoint(const char* tag)
{
    (void)tag;
}

long platformHeapFreeKb()
{
    return -1;
}

void platformCaptureBadAlloc()
{
    wiiHeapCaptureBadAlloc();
}
