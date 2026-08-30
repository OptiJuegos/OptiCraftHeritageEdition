#include "platform/RenderTerrainAPI.h"
#include "platform/RenderAPI.h"
#include "wii/render/WiiNativeDraw.h"
#include "wii/render/WiiMeshSort.h"
#include "wii/render/WiiRenderTypes.h"

int renderTerrainCreateChunkHandle() { return wii_native_create_chunk_handle(); }
void renderTerrainDestroyChunkHandle(int handle) { wii_native_destroy_chunk_handle(handle); }
void renderTerrainClearChunkHandle(int handle) { wii_native_clear_chunk_list(handle); }
void renderTerrainSwapChunkHandles(int liveHandle, int stagingHandle) { wii_native_swap_chunk_lists(liveHandle, stagingHandle); }
bool renderTerrainBeginChunkBatch(int firstHandle) { return wii_native_begin_chunk_batch(firstHandle); }
bool renderTerrainAppendChunk(int handle) { return wii_native_append_chunk_list(handle); }
void renderTerrainEndChunkBatch() { wii_native_end_chunk_batch(); }
void renderTerrainCaptureCamera() { wii_native_capture_terrain_camera(); }
void renderTerrainSetViewerPosition(double x, double y, double z)
{
    wii_native_set_terrain_eye((float)x, (float)y, (float)z);
}
void renderTerrainSetEarlyDepth(bool enabled) { wii_native_set_terrain_early_depth(enabled); }
void renderTerrainSetFog(RenderFogMode mode, float density, float start, float end, float r, float g, float b, float a)
{
    const WiiRenderFogMode nativeMode = mode == RenderFogMode::Linear ? WiiRenderFogMode::Linear :
                                        mode == RenderFogMode::Exp2 ? WiiRenderFogMode::Exp2 : WiiRenderFogMode::Exp;
    wii_native_set_terrain_fog(wiiRenderValue(nativeMode), density, start, end, r, g, b, a);
}
bool renderTerrainSortOpaqueFaces(const std::vector<int_t>& src, std::vector<int_t>& dst, int quadCount, int maxQuads)
{
    return wii_sort_opaque_faces(src, dst, quadCount, maxQuads);
}
bool renderTerrainBeginPass(int texture, RenderTerrainPass pass)
{
    renderBindTexture(texture);
    WiiNativeTerrainPass nativePass = WII_NATIVE_TERRAIN_OPAQUE;
    switch (pass)
    {
        case RenderTerrainPass::Opaque: nativePass = WII_NATIVE_TERRAIN_OPAQUE; break;
        case RenderTerrainPass::Translucent: nativePass = WII_NATIVE_TERRAIN_TRANSLUCENT_COLOR; break;
        case RenderTerrainPass::TranslucentDepth: nativePass = WII_NATIVE_TERRAIN_TRANSLUCENT_DEPTH; break;
        case RenderTerrainPass::TranslucentColor: nativePass = WII_NATIVE_TERRAIN_TRANSLUCENT_COLOR; break;
    }
    return wii_native_begin_terrain_pass(nativePass);
}
void renderTerrainEndPass(RenderTerrainPass) { wii_native_end_terrain_pass(); }
std::size_t renderTerrainLiveBytes() { return static_cast<std::size_t>(wii_native_chunk_list_live_bytes()); }
std::size_t renderTerrainStagingBytes() { return 0; }

bool renderTerrainCaptureFrame(RenderTerrainFrame& out) { out = RenderTerrainFrame{}; return false; }
RenderTerrainDrawResult renderTerrainDrawSection(const RenderTerrainFrame&, const RenderTerrainSectionView&, const RenderTerrainFallbackDraw&) { return {}; }

void renderTerrainCacheInit(RenderTerrainBackendCache& cache) { cache.initialized = true; }
void renderTerrainCacheDestroy(RenderTerrainBackendCache& cache) { cache.initialized = false; }
void renderTerrainCacheReset(RenderTerrainBackendCache&) {}
void renderTerrainCacheRelease(RenderTerrainBackendCache&) {}
std::size_t renderTerrainCacheRamBytes(const RenderTerrainBackendCache&) { return 0; }
void renderTerrainCacheRamBreakdown(const RenderTerrainBackendCache&, RenderTerrainCacheRamBreakdown&) {}
bool renderTerrainCacheSortFaces(RenderTerrainBackendCache&, const int_t*, int_t*, int_t) { return false; }
bool renderTerrainCacheBuildOpaque(RenderTerrainBackendCache&, const int_t*, std::size_t, int_t, int_t, bool, bool, bool) { return false; }
bool renderTerrainCacheOpaqueValid(const RenderTerrainBackendCache&) { return false; }
int_t renderTerrainCacheOpaqueVertexCount(const RenderTerrainBackendCache&) { return 0; }
const void* renderTerrainCacheFaceGroups(const RenderTerrainBackendCache&) { return nullptr; }
const void* renderTerrainCacheOpaqueMesh(const RenderTerrainBackendCache&) { return nullptr; }

bool renderTerrainIsGreedyCube(Block*) { return false; }
bool renderTerrainGreedyMeshFace(ChunkCache&, int, int, int, int, int, int, int) { return false; }
