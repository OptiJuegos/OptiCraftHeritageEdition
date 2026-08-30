#include "platform/Diagnostics.h"

const char* platformOomDiagnosticLine(int index)
{
    (void)index;
    return "";
}

void platformMemoryCheckpoint(const char* tag)
{
    (void)tag;
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
}
