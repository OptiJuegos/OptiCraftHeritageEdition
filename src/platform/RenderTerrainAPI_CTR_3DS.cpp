// RenderTerrainAPI_CTR_3DS.cpp — Nintendo 3DS terrain backend surface.
//
// Phase 1 deliberately mirrors the plain PC shape (RenderTerrainAPI_PC.cpp):
// the 3DS is not opted into PLATFORM_NATIVE_TERRAIN_PIPELINE (that is Wii-only),
// so sections take the shared immediate-mode path in WorldRenderer and end up
// in RenderAPI_CTR_3DS.cpp's renderDrawInterleaved(), which is itself a stub
// until the citro3d renderer lands.
//
// The four incremental-cache entry points below go beyond the PC file, which
// leaves them out: only WorldRendererPs2.cpp drives them today, but they are
// declared unconditionally in RenderTerrainAPI.h, so defining them keeps this
// backend complete rather than one shared caller away from an undefined
// reference.
#include "platform/RenderTerrainAPI.h"
#include "platform/RenderAPI.h"

#include <utility>

namespace { int s_nextTerrainHandle = 1; }

int renderTerrainCreateChunkHandle() { return s_nextTerrainHandle++; }
void renderTerrainDestroyChunkHandle(int) {}
void renderTerrainClearChunkHandle(int) {}
void renderTerrainSwapChunkHandles(int, int) {}
bool renderTerrainBeginChunkBatch(int) { return false; }
bool renderTerrainAppendChunk(int) { return false; }
void renderTerrainEndChunkBatch() {}
void renderTerrainCaptureCamera() {}
void renderTerrainSetViewerPosition(double, double, double) {}
void renderTerrainSetFog(RenderFogMode, float, float, float, float, float, float, float) {}
void renderTerrainSetEarlyDepth(bool) {}
bool renderTerrainSortOpaqueFaces(const std::vector<int_t>&, std::vector<int_t>&, int, int) { return false; }
bool renderTerrainBeginPass(int texture, RenderTerrainPass) { renderBindTexture(texture); return true; }
void renderTerrainEndPass(RenderTerrainPass) {}
std::size_t renderTerrainLiveBytes() { return 0; }
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
RenderTerrainCacheBuildStatus renderTerrainCacheBeginOpaqueBuild(RenderTerrainBackendCache&, const int_t*, std::size_t, int_t, int_t, bool, bool, bool, bool& didWork)
{
	didWork = false;
	return RenderTerrainCacheBuildStatus::Failed;
}
RenderTerrainCacheBuildStatus renderTerrainCacheContinueOpaqueBuild(RenderTerrainBackendCache&, bool& didWork)
{
	didWork = false;
	return RenderTerrainCacheBuildStatus::Failed;
}
void renderTerrainCacheCancelOpaqueBuild(RenderTerrainBackendCache&) {}
void renderTerrainCacheSwap(RenderTerrainBackendCache& a, RenderTerrainBackendCache& b) { std::swap(a, b); }
bool renderTerrainCacheOpaqueValid(const RenderTerrainBackendCache&) { return false; }
int_t renderTerrainCacheOpaqueVertexCount(const RenderTerrainBackendCache&) { return 0; }
const void* renderTerrainCacheFaceGroups(const RenderTerrainBackendCache&) { return nullptr; }
const void* renderTerrainCacheOpaqueMesh(const RenderTerrainBackendCache&) { return nullptr; }

bool renderTerrainIsGreedyCube(Block*) { return false; }
bool renderTerrainGreedyMeshFace(ChunkCache&, int, int, int, int, int, int, int) { return false; }
