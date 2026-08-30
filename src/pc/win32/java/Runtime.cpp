#include "java/Runtime.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <Psapi.h>

Runtime Runtime::instance;

Runtime &Runtime::getRuntime()
{
	return instance;
}

long_t Runtime::maxMemory()
{
	// Get amount of system memory available
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	if (!GlobalMemoryStatusEx(&statex))
		return 1;
	return statex.ullTotalPhys;
}

long_t Runtime::totalMemory()
{
	// Show the memory the process is using right now, not PeakWorkingSetSize.
	// PeakWorkingSetSize is the maximum value reached by the process and by
	// definition it never goes down, which made the debug overlay look stuck.
	PROCESS_MEMORY_COUNTERS_EX pmc{};
	if (!GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS *>(&pmc), sizeof(pmc)))
		return 0;

	// PrivateUsage is closer to "memory owned by this process" than WorkingSetSize.
	// It also avoids reporting memory shared with system DLLs as if it was game RAM.
	return static_cast<long_t>(pmc.PrivateUsage);
}

long_t Runtime::freeMemory()
{
	// Java's Runtime.freeMemory() reports free bytes inside the Java heap.
	// This C++ port does not have a managed heap, so returning a fake value based
	// on the peak working set produced negative/misleading numbers. The overlay
	// will therefore show Used == Allocated for native memory.
	return 0;
}
