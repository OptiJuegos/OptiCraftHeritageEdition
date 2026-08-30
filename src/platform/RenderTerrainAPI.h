#pragma once

#include <cstddef>
#include <vector>
#include "java/Type.h"
#include "platform/RenderAPI.h"

class Block;
class ChunkCache;

enum class RenderTerrainPass { Opaque = 0, Translucent = 1, TranslucentDepth = 2, TranslucentColor = 3 };

struct RenderTerrainBackendCache
{
#ifdef PS2_PLATFORM
    // 4x4x4 PS2 terrain clustering keeps 64 Ps2MeshCluster records in
    // Ps2FaceGroups. Keep this backend-local inline storage large enough for
    // that object without penalising the PC/Wii cache layout.
    alignas(16) unsigned char faceGroupsStorage[1664] = {};
#else
    alignas(16) unsigned char faceGroupsStorage[512] = {};
#endif
    alignas(16) unsigned char opaqueMeshStorage[128] = {};
    bool initialized = false;
};

void renderTerrainCacheInit(RenderTerrainBackendCache& cache);
void renderTerrainCacheDestroy(RenderTerrainBackendCache& cache);
void renderTerrainCacheReset(RenderTerrainBackendCache& cache);
void renderTerrainCacheRelease(RenderTerrainBackendCache& cache);
std::size_t renderTerrainCacheRamBytes(const RenderTerrainBackendCache& cache);

// The same total, split by what drives each half.
//
// packedMeshBytes scales with the section's vertex count. faceGroupBytes does
// not: it is one range per (atlas tile x face bucket x spatial cluster) the
// reorder produced, so it scales with how fragmented the section's texture use
// is. A summed figure cannot tell those apart, which is exactly the question
// the section-mesh RAM measurements ran into.
struct RenderTerrainCacheRamBreakdown
{
    std::size_t packedMeshBytes = 0;
    std::size_t faceGroupBytes = 0;
};
void renderTerrainCacheRamBreakdown(const RenderTerrainBackendCache& cache,
                                    RenderTerrainCacheRamBreakdown& out);

// All of a terrain grid's mesh storage, attributed.
//
// Lives here rather than inside WorldRenderer because RenderGlobal only
// forward-declares that class, and both of them have to name this to report the
// same quantity. The two live buffers are the AoS meshes a section owns; the
// staging figure is pool-wide (see Ps2MeshStagingPool.h), which is why it is a
// separate bucket rather than something a single renderer contributes to.
struct RenderTerrainMeshRam
{
    std::size_t liveOpaque = 0;
    std::size_t liveTranslucent = 0;
    std::size_t packedMesh = 0;
    std::size_t faceGroups = 0;
    std::size_t stagingPool = 0;

    std::size_t total() const
    {
        return liveOpaque + liveTranslucent + packedMesh + faceGroups + stagingPool;
    }
};
bool renderTerrainCacheSortFaces(RenderTerrainBackendCache& cache, const int_t* src, int_t* dst, int_t quadCount);
bool renderTerrainCacheBuildOpaque(RenderTerrainBackendCache& cache, const int_t* raw, std::size_t rawIntCount,
                                   int_t vertexCount, int_t drawMode, bool hasTexture, bool hasColor, bool hasNormals);
bool renderTerrainCacheOpaqueValid(const RenderTerrainBackendCache& cache);
int_t renderTerrainCacheOpaqueVertexCount(const RenderTerrainBackendCache& cache);
const void* renderTerrainCacheFaceGroups(const RenderTerrainBackendCache& cache);
const void* renderTerrainCacheOpaqueMesh(const RenderTerrainBackendCache& cache);

struct RenderTerrainFrame
{
    float mvp[16] = {};
    float viewW = 0.0f;
    float viewH = 0.0f;
    bool valid = false;
};

struct RenderTerrainSectionView
{
    const int_t* raw = nullptr;
    std::size_t rawIntCount = 0;
    int_t vertexCount = 0;
    int_t drawMode = 0;
    bool hasTexture = false;
    bool hasColor = false;
    bool hasNormals = false;
    const void* faceGroups = nullptr;
    const void* opaqueMesh = nullptr;
    float translateX = 0.0f, translateY = 0.0f, translateZ = 0.0f;
    float eyeLocalX = 0.0f, eyeLocalY = 0.0f, eyeLocalZ = 0.0f;
    bool fullyInside = false;
    bool nativeEnabled = false;
};

using RenderTerrainFallbackDrawFn = void (*)(void*, const int_t*, int_t, int_t, bool, bool, bool);
struct RenderTerrainFallbackDraw { RenderTerrainFallbackDrawFn draw = nullptr; void* user = nullptr; };
struct RenderTerrainDrawResult { int_t nativeVertices = 0; int_t fallbackVertices = 0; bool complete = false; };

bool renderTerrainCaptureFrame(RenderTerrainFrame& out);
RenderTerrainDrawResult renderTerrainDrawSection(const RenderTerrainFrame& frame,
                                                 const RenderTerrainSectionView& section,
                                                 const RenderTerrainFallbackDraw& fallback);


// High-level terrain/chunk operations. Minecraft code deals only in opaque
// integer handles; GX display lists and PS2 native terrain state stay in the backend.
int  renderTerrainCreateChunkHandle();
void renderTerrainDestroyChunkHandle(int handle);
void renderTerrainClearChunkHandle(int handle);
void renderTerrainSwapChunkHandles(int liveHandle, int stagingHandle);

struct RenderTerrainChunkHandles
{
    int live[2] = {};
    int staging[2] = {};
};

void renderTerrainChunkHandlesCreate(RenderTerrainChunkHandles& handles);
void renderTerrainChunkHandlesDestroy(RenderTerrainChunkHandles& handles);
void renderTerrainChunkHandlesClearLive(RenderTerrainChunkHandles& handles);
void renderTerrainChunkHandlesClearStaging(RenderTerrainChunkHandles& handles);
void renderTerrainChunkHandlesPublish(RenderTerrainChunkHandles& handles);

bool renderTerrainBeginChunkBatch(int firstHandle);
bool renderTerrainAppendChunk(int handle);
void renderTerrainEndChunkBatch();

void renderTerrainCaptureCamera();

// Interpolated eye position for the terrain about to be drawn, in world
// coordinates. Published once per pass, before the visible sections are
// submitted, so a backend that culls geometry against the eye is deciding on
// the same frame it is drawing. Backends without such a cull ignore it.
void renderTerrainSetViewerPosition(double x, double y, double z);
void renderTerrainSetFog(RenderFogMode mode, float density, float start, float end,
                         float r, float g, float b, float a);

// Declare that the chunk batches submitted next contain no texels the alpha
// test would reject, so a backend may drop the test and rely on early depth
// rejection instead. Applies to the opaque pass only, and the backend resets it
// when the pass ends. A backend without an early-depth path ignores it.
void renderTerrainSetEarlyDepth(bool enabled);
bool renderTerrainSortOpaqueFaces(const std::vector<int_t>& src, std::vector<int_t>& dst,
                                  int quadCount, int maxQuads);
bool renderTerrainBeginPass(int texture, RenderTerrainPass pass);
void renderTerrainEndPass(RenderTerrainPass pass);
std::size_t renderTerrainLiveBytes();
std::size_t renderTerrainStagingBytes();

// Optional backend-specific terrain meshing exposed through a neutral API.
bool renderTerrainIsGreedyCube(Block* block);
bool renderTerrainGreedyMeshFace(ChunkCache& cache, int face, int x0, int y0, int z0, int x1, int y1, int z1);
constexpr int RENDER_TERRAIN_GREEDY_FACE_COUNT = 6;
