#ifdef WII_PLATFORM

#include "platform/Log.h"
#include "wii/system/WiiEarlyMemory.h"
#include "wii/WiiEarlyInit.h"

#include <gccore.h>
#include <malloc.h>

extern "C" {
u32 MALLOC_MEM2 = 1;
}

namespace
{
u32 g_heapCeiling = 0;
u32 g_heapBaseCommitted = 0;
u32 g_initialArena1 = 0;
u32 g_initialArena2 = 0;
}

namespace WiiEarlyMemory
{

void captureInitialState()
{
	if (g_heapCeiling != 0)
		return;

	const struct mallinfo info = mallinfo();
	g_heapBaseCommitted = static_cast<u32>(info.arena);
	g_initialArena1 = static_cast<u32>(SYS_GetArena1Size());
	g_initialArena2 = static_cast<u32>(SYS_GetArena2Size());
	g_heapCeiling = g_heapBaseCommitted + g_initialArena1 + g_initialArena2;

	MC_LOG_INFO("wii", "heap ceiling %u KB; base committed %u KB; MEM1 left %u KB; MEM2 left %u KB\n",
	       g_heapCeiling / 1024u,
	       g_heapBaseCommitted / 1024u,
	       g_initialArena1 / 1024u,
	       g_initialArena2 / 1024u);
}

} // namespace WiiEarlyMemory

u32 wiiGetHeapCeiling()
{
	WiiEarlyMemory::captureInitialState();
	return g_heapCeiling;
}

u32 wiiGetHeapCommitted()
{
	WiiEarlyMemory::captureInitialState();

	const u32 currentArena1 = static_cast<u32>(SYS_GetArena1Size());
	const u32 currentArena2 = static_cast<u32>(SYS_GetArena2Size());
	const u32 used1 = g_initialArena1 > currentArena1 ? g_initialArena1 - currentArena1 : 0u;
	const u32 used2 = g_initialArena2 > currentArena2 ? g_initialArena2 - currentArena2 : 0u;
	const u64 committed = static_cast<u64>(g_heapBaseCommitted) + used1 + used2;
	return committed < g_heapCeiling ? static_cast<u32>(committed) : g_heapCeiling;
}

#endif // WII_PLATFORM
