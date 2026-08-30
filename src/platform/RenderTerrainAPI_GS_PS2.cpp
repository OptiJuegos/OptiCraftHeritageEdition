#include "platform/RenderTerrainAPI.h"
#include "platform/RenderAPI.h"
#include "ps2/render/Ps2TerrainRenderer.h"
#include "ps2/render/Ps2MeshSort.h"
#include "ps2/render/Ps2TerrainMesh.h"
#include "ps2/render/Ps2GreedyMesh.h"
#include "ps2/render/Ps2MeshStagingPool.h"
#include <new>

namespace { int s_nextTerrainHandle = 1; }
int renderTerrainCreateChunkHandle() { return s_nextTerrainHandle++; }
void renderTerrainDestroyChunkHandle(int) {}
void renderTerrainClearChunkHandle(int) {}
void renderTerrainSwapChunkHandles(int, int) {}
bool renderTerrainBeginChunkBatch(int) { return false; }
bool renderTerrainAppendChunk(int) { return false; }
void renderTerrainEndChunkBatch() {}
void renderTerrainCaptureCamera() {}
// The PS2 publishes the same position through WorldRenderer::setTerrainViewerPosition(),
// which also snapshots projection*modelview for the section transform.
void renderTerrainSetViewerPosition(double, double, double) {}
void renderTerrainSetEarlyDepth(bool) {}
void renderTerrainSetFog(RenderFogMode mode, float density, float start, float end, float r, float g, float b, float a)
{
    renderFogi(RenderFogParameter::Mode, mode);
    renderFogf(RenderFogParameter::Density, density);
    renderFogf(RenderFogParameter::Start, start);
    renderFogf(RenderFogParameter::End, end);
    const float color[4] = { r, g, b, a };
    renderFogColor(color);
}
bool renderTerrainSortOpaqueFaces(const std::vector<int_t>&, std::vector<int_t>&, int, int) { return false; }
bool renderTerrainBeginPass(int texture, RenderTerrainPass pass)
{
    ps2_terrain_begin(static_cast<unsigned int>(texture), pass == RenderTerrainPass::Opaque ? PS2_TERRAIN_PASS_OPAQUE : PS2_TERRAIN_PASS_TRANSLUCENT);
    return true;
}
void renderTerrainEndPass(RenderTerrainPass pass)
{
    ps2_terrain_end(pass == RenderTerrainPass::Opaque ? PS2_TERRAIN_PASS_OPAQUE : PS2_TERRAIN_PASS_TRANSLUCENT);
}
std::size_t renderTerrainLiveBytes() { return 0; }
std::size_t renderTerrainStagingBytes() { return ps2_mesh_staging_ram_bytes(); }


bool renderTerrainCaptureFrame(RenderTerrainFrame& out)
{
    Ps2RendererFrame native{};
    if (!ps2_renderer_capture_frame(native)) { out = RenderTerrainFrame{}; return false; }
    for (int i = 0; i < 16; ++i) out.mvp[i] = native.native.mvp[i];
    out.viewW = native.native.viewW;
    out.viewH = native.native.viewH;
    out.valid = native.valid && native.native.valid;
    return out.valid;
}

RenderTerrainDrawResult renderTerrainDrawSection(const RenderTerrainFrame& frame,
                                                 const RenderTerrainSectionView& section,
                                                 const RenderTerrainFallbackDraw& fallback)
{
    Ps2RendererFrame nativeFrame{};
    for (int i = 0; i < 16; ++i) nativeFrame.native.mvp[i] = frame.mvp[i];
    nativeFrame.native.viewW = frame.viewW;
    nativeFrame.native.viewH = frame.viewH;
    nativeFrame.native.valid = frame.valid;
    nativeFrame.valid = frame.valid;

    Ps2TerrainSectionView nativeSection{};
    nativeSection.raw = section.raw;
    nativeSection.rawIntCount = section.rawIntCount;
    nativeSection.vertexCount = section.vertexCount;
    nativeSection.drawMode = section.drawMode;
    nativeSection.hasTexture = section.hasTexture;
    nativeSection.hasColor = section.hasColor;
    nativeSection.hasNormals = section.hasNormals;
    nativeSection.faceGroups = static_cast<const Ps2FaceGroups*>(section.faceGroups);
    nativeSection.opaqueMesh = static_cast<const Ps2TerrainMesh*>(section.opaqueMesh);
    nativeSection.translateX = section.translateX;
    nativeSection.translateY = section.translateY;
    nativeSection.translateZ = section.translateZ;
    nativeSection.eyeLocalX = section.eyeLocalX;
    nativeSection.eyeLocalY = section.eyeLocalY;
    nativeSection.eyeLocalZ = section.eyeLocalZ;
    nativeSection.fullyInside = section.fullyInside;
    nativeSection.nativeEnabled = section.nativeEnabled;

    Ps2TerrainFallbackDraw nativeFallback{fallback.draw, fallback.user};
    const Ps2TerrainDrawResult result = ps2_terrain_draw_section(nativeFrame, nativeSection, nativeFallback);
    return {result.nativeVertices, result.fallbackVertices, result.complete};
}


namespace
{
Ps2FaceGroups* cacheGroups(RenderTerrainBackendCache& cache) { return reinterpret_cast<Ps2FaceGroups*>(cache.faceGroupsStorage); }
const Ps2FaceGroups* cacheGroups(const RenderTerrainBackendCache& cache) { return reinterpret_cast<const Ps2FaceGroups*>(cache.faceGroupsStorage); }
Ps2TerrainMesh* cacheMesh(RenderTerrainBackendCache& cache) { return reinterpret_cast<Ps2TerrainMesh*>(cache.opaqueMeshStorage); }
const Ps2TerrainMesh* cacheMesh(const RenderTerrainBackendCache& cache) { return reinterpret_cast<const Ps2TerrainMesh*>(cache.opaqueMeshStorage); }
static_assert(sizeof(Ps2FaceGroups) <= sizeof(RenderTerrainBackendCache::faceGroupsStorage),
              "RenderTerrain face-group storage too small");
static_assert(sizeof(Ps2TerrainMesh) <= 128, "RenderTerrain opaque-mesh storage too small");
}

void renderTerrainCacheInit(RenderTerrainBackendCache& cache)
{
    if (cache.initialized) return;
    new (cache.faceGroupsStorage) Ps2FaceGroups();
    new (cache.opaqueMeshStorage) Ps2TerrainMesh();
    cacheGroups(cache)->reset();
    cache.initialized = true;
}
void renderTerrainCacheDestroy(RenderTerrainBackendCache& cache)
{
    if (!cache.initialized) return;
    cacheGroups(cache)->~Ps2FaceGroups();
    cacheMesh(cache)->~Ps2TerrainMesh();
    cache.initialized = false;
}
void renderTerrainCacheReset(RenderTerrainBackendCache& cache)
{
    if (!cache.initialized) renderTerrainCacheInit(cache);
    cacheGroups(cache)->reset();
    cacheMesh(cache)->clearKeepCapacity();
}
void renderTerrainCacheRelease(RenderTerrainBackendCache& cache)
{
    if (!cache.initialized) return;
    cacheGroups(cache)->release();
    cacheMesh(cache)->release();
}
std::size_t renderTerrainCacheRamBytes(const RenderTerrainBackendCache& cache)
{
    if (!cache.initialized) return 0;
    return cacheGroups(cache)->ramBytes() + cacheMesh(cache)->ramBytes();
}
void renderTerrainCacheRamBreakdown(const RenderTerrainBackendCache& cache,
                                    RenderTerrainCacheRamBreakdown& out)
{
    if (!cache.initialized) return;
    out.packedMeshBytes += cacheMesh(cache)->ramBytes();
    out.faceGroupBytes += cacheGroups(cache)->ramBytes();
}
bool renderTerrainCacheSortFaces(RenderTerrainBackendCache& cache, const int_t* src, int_t* dst, int_t quadCount)
{
    if (!cache.initialized) renderTerrainCacheInit(cache);
    return ps2_mesh_sort_faces(src, dst, quadCount, *cacheGroups(cache));
}
bool renderTerrainCacheBuildOpaque(RenderTerrainBackendCache& cache, const int_t* raw, std::size_t rawIntCount,
                                   int_t vertexCount, int_t drawMode, bool hasTexture, bool hasColor, bool hasNormals)
{
    if (!cache.initialized) renderTerrainCacheInit(cache);
    return cacheMesh(cache)->build(raw, rawIntCount, vertexCount, drawMode, hasTexture, hasColor, hasNormals);
}
bool renderTerrainCacheOpaqueValid(const RenderTerrainBackendCache& cache) { return cache.initialized && cacheMesh(cache)->valid(); }
int_t renderTerrainCacheOpaqueVertexCount(const RenderTerrainBackendCache& cache) { return cache.initialized ? cacheMesh(cache)->vertexCount() : 0; }
const void* renderTerrainCacheFaceGroups(const RenderTerrainBackendCache& cache) { return cache.initialized ? cacheGroups(cache) : nullptr; }
const void* renderTerrainCacheOpaqueMesh(const RenderTerrainBackendCache& cache) { return cache.initialized && cacheMesh(cache)->valid() ? cacheMesh(cache) : nullptr; }

bool renderTerrainIsGreedyCube(Block* block) { return ps2_is_greedy_cube(block); }
bool renderTerrainGreedyMeshFace(ChunkCache& cache, int face, int x0, int y0, int z0, int x1, int y1, int z1) { return ps2_greedy_mesh_face(cache, face, x0, y0, z0, x1, y1, z1); }
