#include "platform/Profiler.h"

std::uint32_t platformProfileRenderPhaseBegin() { return 0; }
void platformProfileRenderPhaseEnd(std::uint32_t, PlatformRenderPhase) {}
void platformProfileTickPhase(const char*, long long) {}
void platformProfileChunkBuild(long long, int) {}
void platformProfileChunkMeshPass(int, long long, int) {}
void platformProfileSnowColumn(bool, bool, int) {}
void platformProfilePopulatePhase(PlatformPopulatePhase, long long) {}
void platformProfileChunkLoad(long long) {}
void platformProfilePopulate(long long) {}
void platformProfileGenerate(long long) {}
void platformProfileMesh(long long) {}
void platformProfileUnloadSave(long long) {}
void platformProfileTickUpdates(long long) {}
void platformProfileTickQueue(long long) {}
void platformProfileMobSpawn(long long) {}
void platformProfileSaveWorldInfo(long long) {}
void platformProfileMapStorage(long long) {}
void platformProfileChunkEvict(long long) {}
