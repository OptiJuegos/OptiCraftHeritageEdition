#pragma once

#ifdef PS2_PLATFORM

struct Ps2GsQueueRenderStats
{
	long flushes = 0;
	long peakBytes = 0;
	long maxGuardWriteBytes = 0;
	long guardOverruns = 0;
};

void ps2_gs_queue_guard(int vertexCount);
void ps2_gs_queue_guard_prim(unsigned int mode, int count);
void ps2_gs_queue_render_stats(Ps2GsQueueRenderStats& out, bool reset);

extern "C" void ps2_gs_queue_note_overflow(long bytes);
extern "C" long ps2_gs_queue_overflow_count();
extern "C" long ps2_gs_queue_overflow_peak_bytes();
extern "C" void ps2_gs_queue_check_overflow();
extern "C" long ps2_gs_queue_used_bytes();
extern "C" long ps2_gs_queue_capacity_bytes();
extern "C" void ps2_gs_queue_flush_oneshot();
extern "C" void ps2_gs_queue_report(long frame);

#endif
