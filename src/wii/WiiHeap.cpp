// WiiHeap.cpp — see WiiHeap.h for what is reported and why the two mallinfo
// fields everyone reaches for first are deliberately absent.
#ifdef WII_PLATFORM

#include "platform/Log.h"
#include "wii/WiiHeap.h"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <new>

#include <malloc.h>

#include <gccore.h>

#include "wii/WiiEarlyInit.h"

#include "wii/render/WiiNativeDraw.h"

extern "C" void wiiAudioMemoryStats(u32 *cachedBytes, u32 *residentBytes,
                                     u32 *pendingBuffers, u32 *rejectedSounds,
                                     u32 *streamStarves, u32 *streamAddBusy);

namespace
{
// Static storage deliberately: this is read while rendering the OOM screen,
// precisely when allocating a std::string to format diagnostics might fail.
char s_badAllocLines[5][96] = {};
std::size_t s_lastBadAllocRequest = 0;
void *s_lastBadAllocCaller = nullptr;
u32 s_previousHeadroomKb = 0;
u32 s_lowHeadroomKb = 0xffffffffu;

void captureBadAllocLines()
{
	struct mallinfo mi = mallinfo();
	const u32 listBytes = wii_native_chunk_list_memory();
	const u32 listCount = wii_native_chunk_list_count();

	const u32 mem1Left = static_cast<u32>(SYS_GetArena1Size());
	const u32 mem2Left = static_cast<u32>(SYS_GetArena2Size());
	const u32 headroomKb =
		(static_cast<u32>(mi.fordblks) + mem1Left + mem2Left) / 1024u;

	std::snprintf(s_badAllocLines[0], sizeof(s_badAllocLines[0]),
		"Wii heap at allocation failure:");
	std::snprintf(s_badAllocLines[1], sizeof(s_badAllocLines[1]),
		"headroom %uKB; heap free %uKB/%d blocks",
		headroomKb, (unsigned)mi.fordblks / 1024u, mi.ordblks);
	std::snprintf(s_badAllocLines[2], sizeof(s_badAllocLines[2]),
		"heap top %uKB; M1 %uKB M2 %uKB left",
		(unsigned)mi.keepcost / 1024u, mem1Left / 1024u, mem2Left / 1024u);
	std::snprintf(s_badAllocLines[3], sizeof(s_badAllocLines[3]),
		"GX lists %uKB in %u buffers",
		listBytes / 1024u, listCount);
	if (s_lastBadAllocRequest != 0)
	{
		std::snprintf(s_badAllocLines[4], sizeof(s_badAllocLines[4]),
			"request %uKB (%u bytes), caller %p",
			(unsigned)(s_lastBadAllocRequest / 1024u),
			(unsigned)s_lastBadAllocRequest,
			s_lastBadAllocCaller);
	}
	else
	{
		std::snprintf(s_badAllocLines[4], sizeof(s_badAllocLines[4]),
			"failed C++ request: size unavailable");
	}
}
} // namespace

// The standard allocator only reports `bad_alloc`, not the requested size.
// On Wii that makes a fragmented 900 KB request look identical to a corrupt
// save asking for 20 MB.  Keep the last throwing request in static storage so
// the existing OOM screen can identify which case happened without allocating
// while the heap is already under pressure.
void *operator new(std::size_t size)
{
	const std::size_t request = size == 0 ? 1 : size;
	for (;;)
	{
		if (void *result = std::malloc(request))
			return result;

		std::new_handler handler = std::get_new_handler();
		if (handler == nullptr)
		{
			s_lastBadAllocRequest = request;
			s_lastBadAllocCaller =
				__builtin_extract_return_addr(__builtin_return_address(0));
			throw std::bad_alloc();
		}
		handler();
	}
}

void *operator new[](std::size_t size)
{
	return ::operator new(size);
}

void wiiHeapReport(const char *tag)
{
	struct mallinfo mi = mallinfo();

	const unsigned freeKb = (unsigned)mi.fordblks / 1024u;
	const unsigned topKb  = (unsigned)mi.keepcost / 1024u;
	const unsigned a1Kb   = (unsigned)SYS_GetArena1Size() / 1024u;
	const unsigned a2Kb   = (unsigned)SYS_GetArena2Size() / 1024u;

	// Who is holding the heap. Minecraft records one GX display list per chunk
	// section, so at SHORT render distance this is 648 buffers of 20-200 KB and
	// is by far the largest single consumer -- but the point of printing it is
	// that "by far the largest" stops being a hypothesis. avg = bytes/lists is
	// the figure that says what raising the render distance would cost.
	const u32 listBytes = wii_native_chunk_list_memory();
	const u32 liveListBytes = wii_native_chunk_list_live_bytes();
	const u32 listCount = wii_native_chunk_list_count();
	// Declared unsigned int rather than u32 so the out-param types match the
	// accessor exactly regardless of how gctypes.h spells u32.
	unsigned int poolHits = 0, poolMisses = 0;
	unsigned int poolOverflows = 0, poolBytes = 0;
	wii_native_chunk_buffer_pool_stats(&poolHits, &poolMisses,
	                                   &poolOverflows, &poolBytes);
	unsigned int faceDrawn = 0, faceCulled = 0;
	wii_native_face_cull_stats(&faceDrawn, &faceCulled);
	u32 audioCached = 0, audioResident = 0;
	u32 audioPending = 0, audioRejected = 0;
	u32 audioStarves = 0, audioAddBusy = 0;
	wiiAudioMemoryStats(&audioCached, &audioResident,
	                    &audioPending, &audioRejected,
	                    &audioStarves, &audioAddBusy);

	const u32 headroomKb = freeKb + a1Kb + a2Kb;
	const int headroomDeltaKb = s_previousHeadroomKb == 0
		? 0
		: (int)headroomKb - (int)s_previousHeadroomKb;
	s_previousHeadroomKb = headroomKb;
	if (headroomKb < s_lowHeadroomKb)
		s_lowHeadroomKb = headroomKb;

	// Routed through the unified logger so the existing diagnostic
	// channels keep receiving the same memory snapshots.
	MC_LOG_INFO("wii", "[WII][RAM] %-24s headroom=%uKB delta=%+dKB low=%uKB | "
	       "heapFree=%uKB/%d top=%uKB sbrk-left: MEM1=%uKB MEM2=%uKB | "
	       "gxlists=%uKB in %u (avg %uKB) pool=%uKB hit=%u miss=%u over=%u faceCull=%u/%u | "
	       "audioCache=%uKB resident=%uKB pending=%u rejected=%u starve=%u busy=%u\n",
	       tag, headroomKb, headroomDeltaKb, s_lowHeadroomKb,
	       freeKb, mi.ordblks, topKb, a1Kb, a2Kb,
	       listBytes / 1024u, listCount,
	       listCount ? (liveListBytes / 1024u) / listCount : 0u,
	       poolBytes / 1024u, poolHits, poolMisses, poolOverflows,
	       faceCulled, faceDrawn + faceCulled,
	       audioCached / 1024u, audioResident / 1024u,
	       audioPending, audioRejected, audioStarves, audioAddBusy);
}

void wiiHeapCaptureBadAlloc()
{
	captureBadAllocLines();
	MC_LOG_INFO("wii", "[WII][RAM] bad_alloc C++ request=%u bytes (%uKB) caller=%p\n",
	       (unsigned)s_lastBadAllocRequest,
	       (unsigned)(s_lastBadAllocRequest / 1024u),
	       s_lastBadAllocCaller);
	wiiHeapReport("bad_alloc pre-cleanup");
}

const char *wiiHeapLastBadAllocLine(unsigned index)
{
	return index < 5 ? s_badAllocLines[index] : "";
}

#endif // WII_PLATFORM
