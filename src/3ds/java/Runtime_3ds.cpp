// Runtime_3ds.cpp -- 3DS implementation of java/Runtime.h.
//
// The three figures follow the Wii's definitions (src/wii/java/Runtime_wii.cpp)
// because the questions are the same: how much may be allocated at most, how
// much of the heap has been committed, and how much of that is free.
//
// The ceiling comes from the application memregion -- the budget malloc grows
// into, and the number the DsBringup report prints (the datasheet's 64 MB /
// 124 MB is the chip, not what the process can actually use; see the
// reportMemory comment in src/3ds/tools/DsBringup.cpp). Committed and free
// come from newlib's mallinfo(), the same allocator statistics the Wii reads:
// arena is what sbrk has taken from the OS, fordblks what is free inside it.
#ifdef CTR_PLATFORM

#include "java/Runtime.h"

#include <3ds.h>

#include <malloc.h>

Runtime Runtime::instance;

Runtime &Runtime::getRuntime()
{
	return instance;
}

long_t Runtime::maxMemory()
{
	// The whole application memregion: everything the sbrk heap can still
	// grow into, sampled the same way the bring-up report does.
	const u32 ceiling = osGetMemRegionSize(MEMREGION_APPLICATION);
	return ceiling > 0 ? static_cast<long_t>(ceiling) : 1;
}

long_t Runtime::totalMemory()
{
	// Committed heap: bytes newlib's malloc has actually taken.
	const struct mallinfo mi = mallinfo();
	return static_cast<long_t>(mi.arena);
}

long_t Runtime::freeMemory()
{
	const struct mallinfo mi = mallinfo();
	const std::uint32_t committed = static_cast<std::uint32_t>(mi.arena);
	const std::uint32_t allocatorFree = static_cast<std::uint32_t>(mi.fordblks);
	// Same defensive clamp as the Wii: Java freeMemory must never exceed
	// totalMemory, even if the two statistics are sampled across an unusual
	// allocator transition.
	return static_cast<long_t>(allocatorFree < committed ? allocatorFree : committed);
}

#endif // CTR_PLATFORM
