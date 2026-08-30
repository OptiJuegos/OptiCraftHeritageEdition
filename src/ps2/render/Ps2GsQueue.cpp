#ifdef PS2_PLATFORM

#include "ps2/render/Ps2GsQueue.h"

#include "platform/Log.h"
#include <dmaKit.h>
#include <gsKit.h>

extern GSGLOBAL* gsGlobal;

namespace
{
constexpr unsigned int kPrimLineLoop = 0x0002;
constexpr unsigned int kPrimLineStrip = 0x0003;
constexpr unsigned int kPrimQuads = 0x0007;

long s_overflowPeakBytes = 0;
long s_overflows = 0;
long s_warnedAtFrame = 0;

#ifdef PS2_RENDER_STATS
long s_flushes = 0;
long s_peakBytes = 0;
long s_maxGuardWriteBytes = 0;
long s_guardOverruns = 0;
unsigned char* s_guardMark = nullptr;
unsigned int s_guardNeed = 0;
#endif
}

extern "C" void ps2_gs_queue_note_overflow(long bytes)
{
	++s_overflows;
	if (bytes > s_overflowPeakBytes)
		s_overflowPeakBytes = bytes;
	if (s_overflows == 1)
	{
		MC_LOG_ERROR("render",
			"[PS2] GS QUEUE OVERFLOW: wrote %ld bytes past pool_max. The C++ heap behind it is now corrupt; expect a hang in an unrelated allocation. Raise PS2_GSKIT_OS_QUEUE_BYTES.\n",
			bytes);
	}
}

extern "C" long ps2_gs_queue_overflow_count() { return s_overflows; }
extern "C" long ps2_gs_queue_overflow_peak_bytes() { return s_overflowPeakBytes; }

extern "C" void ps2_gs_queue_check_overflow()
{
	if (!gsGlobal || !gsGlobal->Os_Queue)
		return;
	GSQUEUE* queue = gsGlobal->Os_Queue;
	const unsigned char* cursor = (const unsigned char*)queue->pool_cur;
	const unsigned char* limit = (const unsigned char*)queue->pool_max[queue->dbuf];
	if (cursor > limit)
		ps2_gs_queue_note_overflow((long)(cursor - limit));
}

extern "C" long ps2_gs_queue_used_bytes()
{
	if (!gsGlobal || !gsGlobal->Os_Queue)
		return -1;
	GSQUEUE* queue = gsGlobal->Os_Queue;
	const unsigned char* cursor = (const unsigned char*)queue->pool_cur;
	const unsigned char* base = (const unsigned char*)queue->pool[queue->dbuf];
	return cursor >= base ? (long)(cursor - base) : -1;
}

extern "C" long ps2_gs_queue_capacity_bytes()
{
	if (!gsGlobal || !gsGlobal->Os_Queue)
		return -1;
	GSQUEUE* queue = gsGlobal->Os_Queue;
	const unsigned char* base = (const unsigned char*)queue->pool[queue->dbuf];
	const unsigned char* limit = (const unsigned char*)queue->pool_max[queue->dbuf];
	return limit >= base ? (long)(limit - base) : -1;
}

extern "C" void ps2_gs_queue_report(long frame)
{
	const long used = ps2_gs_queue_used_bytes();
	const long capacity = ps2_gs_queue_capacity_bytes();
	if (used < 0 || capacity <= 0)
		return;
	if (s_warnedAtFrame == 0 && used * 4 > capacity * 3)
	{
		s_warnedAtFrame = frame;
		MC_LOG_WARN("render",
			"[PS2] GS queue high water: frame %ld used %ldKB of %ldKB (%ld%%)."
			" The per-draw guard's margin is what stands between this and a"
			" write past the pool into the C++ heap.\n",
			frame, used / 1024, capacity / 1024, (used * 100) / capacity);
	}
	if ((frame % 60) == 0)
	{
		MC_LOG_DEBUG("render", "[PS2] gs queue: frame %ld used=%ldKB/%ldKB overflows=%ld peak=%ldB\n",
		             frame, used / 1024, capacity / 1024, s_overflows, s_overflowPeakBytes);
	}
}

extern "C" void ps2_gs_queue_flush_oneshot()
{
	if (!gsGlobal || !gsGlobal->Os_Queue)
		return;
	GSQUEUE* queue = gsGlobal->Os_Queue;
	const int beforeBuffer = queue->dbuf;
	const bool hadData = queue->tag_size > 0;
	gsKit_queue_exec(gsGlobal);
	if (hadData && queue->mode == GS_ONESHOT && queue->dbuf == beforeBuffer)
	{
		dmaKit_wait_fast();
		gsKit_queue_reset(queue);
	}
}

void ps2_gs_queue_guard(int vertexCount)
{
	if (!gsGlobal || !gsGlobal->Os_Queue)
		return;
	GSQUEUE* queue = gsGlobal->Os_Queue;
	if (gsGlobal->CurQueue != queue)
		return;
	const unsigned int needed = 8192u + (unsigned int)vertexCount * 128u;
#ifdef PS2_RENDER_STATS
	{
		unsigned char* cursor = (unsigned char*)queue->pool_cur;
		unsigned char* base = (unsigned char*)queue->pool[queue->dbuf];
		if (s_guardMark != nullptr && cursor >= s_guardMark)
		{
			const long actual = (long)(cursor - s_guardMark);
			if (actual > s_maxGuardWriteBytes) s_maxGuardWriteBytes = actual;
			if ((unsigned int)actual > s_guardNeed) ++s_guardOverruns;
		}
		if (cursor >= base && (long)(cursor - base) > s_peakBytes)
			s_peakBytes = (long)(cursor - base);
		s_guardMark = cursor;
		s_guardNeed = needed;
	}
#endif
	const unsigned char* cursor = (const unsigned char*)queue->pool_cur;
	const unsigned char* limit = (const unsigned char*)queue->pool_max[queue->dbuf];
	if (cursor > limit)
		ps2_gs_queue_note_overflow((long)(cursor - limit));
	if (cursor < limit && (unsigned int)(limit - cursor) > needed)
		return;
#ifdef PS2_RENDER_STATS
	++s_flushes;
#endif
	ps2_gs_queue_flush_oneshot();
#ifdef PS2_RENDER_STATS
	s_guardMark = (unsigned char*)gsGlobal->Os_Queue->pool_cur;
#endif
}

void ps2_gs_queue_guard_prim(unsigned int mode, int count)
{
	if (mode == kPrimQuads)
	{
		ps2_gs_queue_guard(count + count / 2);
		return;
	}
	if (mode == kPrimLineStrip || mode == kPrimLineLoop)
	{
		ps2_gs_queue_guard(count * 2);
		return;
	}
	ps2_gs_queue_guard(count);
}

void ps2_gs_queue_render_stats(Ps2GsQueueRenderStats& out, bool reset)
{
#ifdef PS2_RENDER_STATS
	out.flushes = s_flushes;
	out.peakBytes = s_peakBytes;
	out.maxGuardWriteBytes = s_maxGuardWriteBytes;
	out.guardOverruns = s_guardOverruns;
	if (reset)
	{
		s_flushes = 0;
		s_peakBytes = 0;
		s_maxGuardWriteBytes = 0;
		s_guardOverruns = 0;
	}
#else
	(void)reset;
	out = Ps2GsQueueRenderStats{};
#endif
}

#endif
