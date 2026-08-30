#pragma once

#ifdef PS2_PLATFORM

#include "ps2/render/Ps2NativeDraw.h"
#include "ps2/render/Ps2RenderBackend.h"
#include "ps2/render/Ps2TerrainMesh.h"
#include "ps2/render/Ps2Tuning.h"
#include "ps2/render/Ps2Vu1DmaQueue.h"

#if PS2_VU1_CLIPPED_PARTIALS || PS2_VU1_CLIPPED_PROBE_BATCHES_PER_FRAME > 0
static const int PS2_VU1_TERRAIN_CONTEXT_QWORDS = 20;
#else
static const int PS2_VU1_TERRAIN_CONTEXT_QWORDS = 15;
#endif
static const int PS2_VU1_TERRAIN_MAX_VERTICES = 80;
static const int PS2_VU1_TERRAIN_CLIPPED_MAX_VERTICES = 20;


struct Ps2Vu1TerrainSlice
{
    int firstVertex;
    int vertexCount;
};

struct Ps2Vu1TerrainCommonState
{
    Ps2VifQword qwords[PS2_VU1_TERRAIN_CONTEXT_QWORDS];
};

void ps2_vu1_terrain_build_common_state(Ps2Vu1TerrainCommonState& out,
                                        const Ps2NativeFrameContext& frame,
                                        const Ps2TerrainGpuState& gpu);
bool ps2_vu1_terrain_append_common_state(Ps2Vu1DmaQueue& queue,
                                         const Ps2Vu1TerrainCommonState& state);
bool ps2_vu1_terrain_append_sliced_batch(Ps2Vu1DmaQueue& queue,
                                         const Ps2TerrainMesh& mesh,
                                         const Ps2Vu1TerrainSlice* slices,
                                         int sliceCount,
                                         int totalCount,
                                         float translateX,
                                         float translateY,
                                         float translateZ,
                                         const Ps2TerrainGpuState& gpu,
                                         int tileX,
                                         int tileY,
                                         int bufferIndex,
                                         int microAddress);
bool ps2_vu1_terrain_append_batch(Ps2Vu1DmaQueue& queue,
                                  const Ps2TerrainMesh& mesh,
                                  int firstVertex,
                                  int count,
                                  float translateX,
                                  float translateY,
                                  float translateZ,
                                  const Ps2TerrainGpuState& gpu,
                                  int tileX,
                                  int tileY,
                                  int bufferIndex,
                                  int microAddress,
                                  bool clipped);

#endif // PS2_PLATFORM
