// Runtime_wii.cpp — Wii implementation of java/Runtime.h.
//
// libogc's Wii _sbrk_r() grows malloc from MEM1 first and, with
// MALLOC_MEM2!=0, transparently crosses into MEM2 when MEM1 is exhausted.
// Therefore SYS_GetArena2Size() alone is not a valid measure of either the
// committed heap or the heap's remaining capacity.
//
// Important Wii detail: MEM1 and MEM2 are not contiguous in the CPU address
// map. When libogc's sbrk transitions to MEM2, newlib mallinfo().arena can
// include the large address gap between both regions and report ~256 MB of
// phantom heap. WiiEarlyInit tracks real arena consumption instead.
//
// mallinfo().fordblks remains useful for free chunks inside the committed heap.
#ifdef WII_PLATFORM

#include "java/Runtime.h"

#include <malloc.h>

#include "wii/WiiEarlyInit.h"

Runtime Runtime::instance;

Runtime &Runtime::getRuntime()
{
	return instance;
}

long_t Runtime::maxMemory()
{
	// Sampled before normal game allocations. It includes the malloc arena that
	// already exists plus the still-unclaimed MEM1 and MEM2 arenas that libogc's
	// sbrk can grow into.
	const long_t ceiling = static_cast<long_t>(wiiGetHeapCeiling());
	return ceiling > 0 ? ceiling : 1;
}

long_t Runtime::totalMemory()
{
	return static_cast<long_t>(wiiGetHeapCommitted());
}

long_t Runtime::freeMemory()
{
	const u32 committed = wiiGetHeapCommitted();
	const struct mallinfo mi = mallinfo();
	const u32 allocatorFree = static_cast<u32>(mi.fordblks);
	// Be defensive if allocator statistics are sampled during an unusual
	// transition: Java freeMemory must never exceed totalMemory.
	return static_cast<long_t>(allocatorFree < committed ? allocatorFree : committed);
}

#endif // WII_PLATFORM
