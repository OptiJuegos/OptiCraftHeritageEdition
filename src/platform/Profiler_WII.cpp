#include "platform/Profiler.h"

extern "C" void wii_perf_add_chunk_load_ns(long long ns);
extern "C" void wii_perf_add_populate_ns(long long ns);
extern "C" void wii_perf_add_populate_phase_ns(int phase, long long ns);
extern "C" void wii_perf_add_generate_ns(long long ns);
extern "C" void wii_perf_add_mesh_ns(long long ns);
extern "C" void wii_perf_add_unload_save_ns(long long ns);
extern "C" void wii_perf_add_tickupdates_ns(long long ns);
extern "C" void wii_perf_add_tickupdates_queue(long long size);
extern "C" void wii_perf_add_mobspawn_ns(long long ns);
extern "C" void wii_perf_add_saveworldinfo_ns(long long ns);
extern "C" void wii_perf_add_mapstorage_ns(long long ns);

std::uint32_t platformProfileRenderPhaseBegin() { return 0; }
void platformProfileRenderPhaseEnd(std::uint32_t, PlatformRenderPhase) {}
void platformProfileTickPhase(const char*, long long) {}
void platformProfileChunkBuild(long long, int) {}
void platformProfileChunkMeshPass(int, long long, int) {}
void platformProfileSnowColumn(bool, bool, int) {}
void platformProfilePopulatePhase(PlatformPopulatePhase phase, long long ns) { wii_perf_add_populate_phase_ns(static_cast<int>(phase), ns); }
void platformProfileChunkLoad(long long ns) { wii_perf_add_chunk_load_ns(ns); }
void platformProfilePopulate(long long ns) { wii_perf_add_populate_ns(ns); }
void platformProfileGenerate(long long ns) { wii_perf_add_generate_ns(ns); }
void platformProfileMesh(long long ns) { wii_perf_add_mesh_ns(ns); }
void platformProfileUnloadSave(long long ns) { wii_perf_add_unload_save_ns(ns); }
void platformProfileTickUpdates(long long ns) { wii_perf_add_tickupdates_ns(ns); }
void platformProfileTickQueue(long long size) { wii_perf_add_tickupdates_queue(size); }
void platformProfileMobSpawn(long long ns) { wii_perf_add_mobspawn_ns(ns); }
void platformProfileSaveWorldInfo(long long ns) { wii_perf_add_saveworldinfo_ns(ns); }
void platformProfileMapStorage(long long ns) { wii_perf_add_mapstorage_ns(ns); }
void platformProfileChunkEvict(long long) {}
