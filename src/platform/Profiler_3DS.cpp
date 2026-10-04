#include "platform/Profiler.h"

#include "platform/PlatformCompat.h"

// The ds_perf_* counters live in ClientProfilerBackend_3DS.cpp, mirroring how
// the Wii keeps wii_perf_* in ClientProfilerBackend_WII.cpp: this file is the
// call-site-facing measurement side and must not own accumulation state. The
// PS2 variant instead measures with the EE cycle counter and hands cycles to
// ps2_perf_*; the 3DS has no cycle-count hook layer, so it times phases with
// the same monotonic microsecond clock every other budget in the port uses.
extern "C" void ds_perf_add_render_phase_us(int phase, unsigned int us);
extern "C" void ds_perf_note_tick_phase(const char* name, long long ns);
extern "C" void ds_perf_add_chunk_load_ns(long long ns);
extern "C" void ds_perf_add_populate_ns(long long ns);
extern "C" void ds_perf_add_populate_phase_ns(int phase, long long ns);
extern "C" void ds_perf_add_generate_ns(long long ns);
extern "C" void ds_perf_add_mesh_ns(long long ns);
extern "C" void ds_perf_add_unload_save_ns(long long ns);
extern "C" void ds_perf_add_tickupdates_ns(long long ns);
extern "C" void ds_perf_add_tickupdates_queue(long long size);
extern "C" void ds_perf_add_mobspawn_ns(long long ns);
extern "C" void ds_perf_add_saveworldinfo_ns(long long ns);
extern "C" void ds_perf_add_mapstorage_ns(long long ns);

// Low 32 bits of the monotonic microsecond counter: a render phase lasts
// milliseconds, so the truncated difference is exact for anything under
// ~71 minutes -- same reasoning as the Wii's time-base truncation.
std::uint32_t platformProfileRenderPhaseBegin()
{
	return static_cast<std::uint32_t>(PlatformCompat::getMonotonicMicros());
}
void platformProfileRenderPhaseEnd(std::uint32_t start, PlatformRenderPhase phase)
{
	const std::uint32_t now = static_cast<std::uint32_t>(PlatformCompat::getMonotonicMicros());
	ds_perf_add_render_phase_us(static_cast<int>(phase), now - start);
}
void platformProfileTickPhase(const char* name, long long ns) { ds_perf_note_tick_phase(name, ns); }
// Chunk-build/mesh-pass/snow-column spans stay unrecorded on 3DS phase 1,
// matching the Wii: the frame-level and world-step buckets are what the first
// frame-budget decisions get made against, and these call sites are hot.
void platformProfileChunkBuild(long long, int) {}
void platformProfileChunkMeshPass(int, long long, int) {}
void platformProfileSnowColumn(bool, bool, int) {}
void platformProfilePopulatePhase(PlatformPopulatePhase phase, long long ns) { ds_perf_add_populate_phase_ns(static_cast<int>(phase), ns); }
void platformProfileChunkLoad(long long ns) { ds_perf_add_chunk_load_ns(ns); }
void platformProfilePopulate(long long ns) { ds_perf_add_populate_ns(ns); }
void platformProfileGenerate(long long ns) { ds_perf_add_generate_ns(ns); }
void platformProfileMesh(long long ns) { ds_perf_add_mesh_ns(ns); }
void platformProfileUnloadSave(long long ns) { ds_perf_add_unload_save_ns(ns); }
void platformProfileTickUpdates(long long ns) { ds_perf_add_tickupdates_ns(ns); }
void platformProfileTickQueue(long long size) { ds_perf_add_tickupdates_queue(size); }
void platformProfileMobSpawn(long long ns) { ds_perf_add_mobspawn_ns(ns); }
void platformProfileSaveWorldInfo(long long ns) { ds_perf_add_saveworldinfo_ns(ns); }
void platformProfileMapStorage(long long ns) { ds_perf_add_mapstorage_ns(ns); }
// Not recorded in phase 1, as on the Wii: bounded-world eviction cost shows up
// in the tickUpdates/chunkLoad buckets it overlaps anyway.
void platformProfileChunkEvict(long long) {}
