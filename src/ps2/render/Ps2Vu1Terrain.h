#pragma once

#ifdef PS2_PLATFORM

#include "ps2/render/Ps2NativeDraw.h"
#include "ps2/render/Ps2TerrainMesh.h"
#include "ps2/render/Ps2Vu1TerrainPackets.h"

enum Ps2Vu1TerrainStatus
{
    PS2_VU1_TERRAIN_RETRY_NATIVE = 0,
    PS2_VU1_TERRAIN_SUBMITTED = 1,
    PS2_VU1_TERRAIN_FATAL = 2
};

struct Ps2Vu1TerrainDrawResult
{
    Ps2Vu1TerrainStatus status;
    int vertices;
};

struct Ps2Vu1TerrainStats
{
    long pages;
    long batches;
    long vertices;
    long qwords;
    long xgkicks;
    long buffer0Batches;
    long buffer1Batches;
    long clippedBatches;
    long clippedVertices;
    long clippedProbeBatches;
    long clippedProbeVertices;
    long clippedProbeRetries;
    long pathTransitions;
    long waits;
    long maxPageQwords;
    unsigned long waitCycles;
    long canarySubmitted;
    long canaryCompleted;
    long canaryFailures;
};

bool ps2_vu1_terrain_available();
void ps2_vu1_terrain_begin_pass();
bool ps2_vu1_terrain_pass_ready();
bool ps2_vu1_terrain_draw_canary();
bool ps2_vu1_terrain_clipped_probe_available();
Ps2Vu1TerrainDrawResult ps2_vu1_terrain_draw_slices(const Ps2TerrainMesh& mesh,
                                                    const Ps2Vu1TerrainSlice* slices,
                                                    int sliceCount,
                                                    int totalVertices,
                                                    int tileX, int tileY,
                                                    const Ps2NativeFrameContext& frame,
                                                    float translateX,
                                                    float translateY,
                                                    float translateZ);
Ps2Vu1TerrainDrawResult ps2_vu1_terrain_draw_range(const Ps2TerrainMesh& mesh,
                                                   int firstVertex,
                                                   int vertexCount,
                                                   const Ps2NativeFrameContext& frame,
                                                   float translateX,
                                                   float translateY,
                                                   float translateZ,
                                                   bool fullyInside);
Ps2Vu1TerrainDrawResult ps2_vu1_terrain_probe_clipped_range(
                                                   const Ps2TerrainMesh& mesh,
                                                   int firstVertex,
                                                   int vertexCount,
                                                   const Ps2NativeFrameContext& frame,
                                                   float translateX,
                                                   float translateY,
                                                   float translateZ);
void ps2_vu1_terrain_take_stats(Ps2Vu1TerrainStats& out);

// Vertices this backend has actually pushed through VIF1 since boot. Unlike the
// stats above it is not reset by a read and exists in every build, because the
// question it answers -- "did the direct path draw anything at all, ever" -- is
// the one a build with PS2_ENABLE_VU1_TERRAIN=ON cannot otherwise answer. A
// pass can report itself ready and still submit nothing: without
// PS2_VU1_CLIPPED_PARTIALS only clusters proved fully inside the frustum
// qualify, so a small render distance can leave every section on VU0.
long ps2_vu1_terrain_submitted_vertices();

#endif // PS2_PLATFORM
