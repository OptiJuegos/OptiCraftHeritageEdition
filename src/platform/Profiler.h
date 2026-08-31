#pragma once

#include <cstdint>

enum class PlatformRenderPhase
{
    Sky = 0,
    Frustum,
    Build,
    Opaque,
    Entities,
    Translucent,
    Hand,
    Hud
};

enum class PlatformPopulatePhase
{
    Total = 0,
    Lakes,
    Dungeons,
    Fillers,
    Ores,
    Decoration,
    Springs,
    Snow,
    // Appended, not inserted: PS2's static_cast dispatch (Profiler_PS2.cpp) and
    // Wii's (Profiler_WII.cpp, bounds-checked at 8) both index this enum
    // directly, so reordering existing values would silently relabel their
    // samples. Wii ignores this one (its bound check is `< 8`); it was never
    // wired to a distinct bucket there either, so nothing regresses.
    Structures
};

std::uint32_t platformProfileRenderPhaseBegin();
void platformProfileRenderPhaseEnd(std::uint32_t start, PlatformRenderPhase phase);
void platformProfileTickPhase(const char* name, long long ns);
void platformProfileChunkBuild(long long ns, int vertices);
void platformProfileChunkMeshPass(int pass, long long ns, int vertices);
void platformProfileSnowColumn(bool icePlaced, bool snowPlaced, int notifyCalls);
void platformProfilePopulatePhase(PlatformPopulatePhase phase, long long ns);

void platformProfileChunkLoad(long long ns);
void platformProfilePopulate(long long ns);
void platformProfileGenerate(long long ns);
void platformProfileMesh(long long ns);
void platformProfileUnloadSave(long long ns);
void platformProfileTickUpdates(long long ns);
void platformProfileTickQueue(long long size);
void platformProfileMobSpawn(long long ns);
void platformProfileSaveWorldInfo(long long ns);
void platformProfileMapStorage(long long ns);
void platformProfileChunkEvict(long long ns);
