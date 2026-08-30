#pragma once

#ifdef PS2_PLATFORM

#include <cstddef>

#include "java/Type.h"
#include "ps2/render/Ps2MeshSort.h"
#include "ps2/render/Ps2Renderer.h"

class Ps2TerrainMesh;

enum Ps2TerrainPass
{
    PS2_TERRAIN_PASS_OPAQUE = 0,
    PS2_TERRAIN_PASS_TRANSLUCENT = 1
};

struct Ps2TerrainSectionView
{
    const int_t* raw;
    std::size_t rawIntCount;
    int_t vertexCount;
    int_t drawMode;
    bool hasTexture;
    bool hasColor;
    bool hasNormals;
    const Ps2FaceGroups* faceGroups;
	// Canonical packed opaque mesh shared by the direct VU1 path and the
	// clipping-capable VU0 path. Translucent terrain retains the raw stream.
	const Ps2TerrainMesh* opaqueMesh;

    float translateX;
    float translateY;
    float translateZ;
    float eyeLocalX;
    float eyeLocalY;
    float eyeLocalZ;
    bool fullyInside;
    bool nativeEnabled;
};

typedef void (*Ps2TerrainFallbackDrawFn)(void* user,
                                         const int_t* raw,
                                         int_t vertexCount,
                                         int_t drawMode,
                                         bool hasTexture,
                                         bool hasColor,
                                         bool hasNormals);

struct Ps2TerrainFallbackDraw
{
    Ps2TerrainFallbackDrawFn draw;
    void* user;
};

struct Ps2TerrainDrawResult
{
    int_t nativeVertices;
    int_t fallbackVertices;
    bool complete;
};

struct Ps2TerrainClusterStats
{
    long sections;
    long insideClusters;
    long partialClusters;
    long outsideClusters;
    long outsideVertices;
    long guardSafePartialClusters;
    long guardSafePartialVertices;
    long guardNearRiskClusters;
    long guardSideRiskClusters;
    long vu1Ranges;
    long vu1Vertices;
    long vu0GatherBatches;
    long vu0GatherVertices;
};

void ps2_terrain_begin(unsigned int textureId, Ps2TerrainPass pass);
void ps2_terrain_end(Ps2TerrainPass pass);
Ps2TerrainDrawResult ps2_terrain_draw_section(const Ps2RendererFrame& frame,
                                              const Ps2TerrainSectionView& section,
                                              const Ps2TerrainFallbackDraw& fallback);
void ps2_terrain_take_cluster_stats(Ps2TerrainClusterStats& out);

#endif
