#include "platform/Log.h"
#include "wii/render/WiiNativeDraw.h"

#include "wii/render/WiiRenderTypes.h"
#include "wii/render/WiiNativeTexture.h"
#include "wii/render/WiiNativeStateSnapshot.h"
#include "wii/render/WiiProjection.h"
#include "wii/render/WiiMatrix.h"
#include "wii/WiiEarlyInit.h"
#include "platform/PlatformTuning.h"

#include <gccore.h>
#include <malloc.h>
#include <cstddef>
#include <cmath>
#include <cstring>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
    struct WiiNativeChunkList
    {
        void* data = nullptr;
        unsigned int size = 0;
        unsigned int capacity = 0;
        WiiRenderPrimitive primitive = WiiRenderPrimitive::Quads;
        int vertexCount = 0;
        bool texCoordEnabled = false;
        bool colorEnabled = false;
        bool brightnessEnabled = false;
        int brightnessOffset = 28;
        bool compactTerrainFormat = false;
        float translateX = 0.0f;
        float translateY = 0.0f;
        float translateZ = 0.0f;
        float sectionSize = 16.0f;
        // Face-direction split. groupCount is 0 for a section recorded as one
        // undivided list, which is what every non-opaque pass and every mesh the
        // sort refused still produces. Offsets are byte offsets into data, each
        // one a display list of its own.
        int groupCount = 0;
        unsigned int groupOffset[WiiNativeChunkFaceGroups::kGroupCount] = {};
        unsigned int groupSize[WiiNativeChunkFaceGroups::kGroupCount] = {};
        float groupPlaneMin[WiiNativeChunkFaceGroups::kGroupCount] = {};
        float groupPlaneMax[WiiNativeChunkFaceGroups::kGroupCount] = {};
        float worldOrigin[3] = {0.0f, 0.0f, 0.0f};
#if MC_LOG_LEVEL >= 2
        float localMin[3] = {0.0f, 0.0f, 0.0f};
        float localMax[3] = {0.0f, 0.0f, 0.0f};
        float firstPosition[3] = {0.0f, 0.0f, 0.0f};
        float firstUv[2] = {0.0f, 0.0f};
        unsigned char firstColor[4] = {255, 255, 255, 255};
#endif
    };

    struct WiiNativeStaticList
    {
        void* data = nullptr;
        unsigned int size = 0;
        unsigned int capacity = 0;
        WiiRenderPrimitive primitive = WiiRenderPrimitive::Quads;
        int vertexCount = 0;
        bool texCoordEnabled = false;
        bool colorEnabled = false;
        bool hasNormals = false;
        bool brightnessEnabled = false;
        int brightnessOffset = 28;
    };

    static std::unordered_map<int, WiiNativeChunkList> s_chunkLists;
    static std::unordered_map<int, WiiNativeStaticList> s_staticLists;

    struct WiiChunkBufferPoolEntry
    {
        void* data = nullptr;
        unsigned int capacity = 0;
    };

    static std::vector<WiiChunkBufferPoolEntry> s_chunkBufferPool;
    static unsigned int s_chunkBufferPoolBytes = 0;
    // Sized against the burst this recycler exists to absorb, not against a
    // round number. Crossing a chunk boundary repositions a whole renderer
    // plane at once (setDontDraw() -> releaseDisplayListsForCache() in
    // WorldRenderer.cpp); at the observed ~62 KB average that is far more than
    // the 512 KB this used to hold, so the tail of every crossing went straight
    // to free() and came back as a memalign() a few frames later.
    //
    // 1 MB is deliberately still short of a full crossing. The pool is charged
    // against the same PLATFORM_WII_GXLIST_EVICT_HIGH_WATER_BYTES budget as the
    // live sections only in the WiiHeap report, not in the eviction gate any
    // more (see wii_native_chunk_list_live_bytes below) -- but it is real
    // resident memory either way, and idle capacity that never gets reused is
    // strictly worse than not holding it. Read pool=/hit=/miss=/over= in the
    // [WII][RAM] line before moving this again.
    static constexpr unsigned int kChunkBufferPoolMaxBytes = 1024u * 1024u;

    // Recycling effectiveness, reported through wii_native_chunk_buffer_pool_stats().
    // Without these the pool is unfalsifiable: a hit and a miss cost the caller
    // the same call and leave the same visible state, so "the pool works" was an
    // assumption rather than a measurement.
    //
    //   hits/misses  -- how often a rebuild avoided memalign() entirely.
    //   overflows    -- releases the pool had to free() because
    //                   kChunkBufferPoolMaxBytes was already reached. This is the
    //                   number that says whether that cap is the binding
    //                   constraint: a chunk-boundary crossing can recycle a whole
    //                   renderer plane at once (see setDontDraw() in
    //                   WorldRenderer.cpp), which is far more than 512 KB of
    //                   buffers arriving together.
    static unsigned int s_chunkBufferPoolHits = 0;
    static unsigned int s_chunkBufferPoolMisses = 0;
    static unsigned int s_chunkBufferPoolOverflows = 0;
    static int s_nextChunkHandle = 1;
    static int s_nextStaticHandle = 1;

    static bool s_chunkBatchActive = false;
    static bool s_chunkBatchTexCoord = false;
    static bool s_chunkBatchColor = false;
    static bool s_chunkBatchBrightness = false;
    static bool s_chunkBatchCompactFormat = false;
    static WiiRenderPrimitive s_chunkBatchPrimitive = WiiRenderPrimitive::Quads;

    // Interpolated eye for the frame being drawn, world coordinates, published
    // by RenderGlobal through wii_native_set_terrain_eye(). Until something
    // publishes one there is no basis to cull a direction, so every group draws.
    static float s_terrainEyeWorld[3] = {0.0f, 0.0f, 0.0f};
    static bool s_terrainEyeValid = false;

    // Same reasoning as the chunk buffer pool's hit/miss counters: a culled and
    // a drawn group leave the same visible state, so without these "the cull
    // works" is an assumption. Reported through wii_native_face_cull_stats().
    static unsigned int s_faceGroupsDrawn = 0;
    static unsigned int s_faceGroupsCulled = 0;

    static bool s_terrainPassActive = false;
    static bool s_terrainPassPrepared = false;
    static bool s_terrainPassTexCoord = false;
    static bool s_terrainPassColor = false;
    static bool s_terrainPassBrightness = false;
    static bool s_terrainPassCompactFormat = false;
    static WiiRenderPrimitive s_terrainPassPrimitive = WiiRenderPrimitive::Quads;
    static WiiNativeTerrainPass s_terrainPassMode = WII_NATIVE_TERRAIN_OPAQUE;
    // Requested by the caller for the batches it is about to submit, and what
    // the currently prepared GX state was built with. They are separate because
    // the request changes between two groups of the same pass, and that is
    // exactly what has to force a re-prepare.
    static bool s_terrainEarlyDepth = false;
    static bool s_terrainPassEarlyDepth = false;

    struct WiiNativeFogState
    {
        bool valid = false;
        int mode = wiiRenderValue(WiiRenderFogMode::Linear);
        float density = 1.0f;
        float start = 0.0f;
        float end = 1.0f;
        float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    };
    static WiiNativeFogState s_nativeFog;

    static bool terrainPassSignatureMatches(const WiiNativeChunkList& list)
    {
        return s_terrainPassPrepared &&
               s_terrainPassEarlyDepth == s_terrainEarlyDepth &&
               list.texCoordEnabled == s_terrainPassTexCoord &&
               list.colorEnabled == s_terrainPassColor &&
               list.brightnessEnabled == s_terrainPassBrightness &&
               list.compactTerrainFormat == s_terrainPassCompactFormat &&
               list.primitive == s_terrainPassPrimitive;
    }

    static bool chunkBatchSignatureMatches(const WiiNativeChunkList& list)
    {
        return s_chunkBatchActive &&
               list.texCoordEnabled == s_chunkBatchTexCoord &&
               list.colorEnabled == s_chunkBatchColor &&
               list.brightnessEnabled == s_chunkBatchBrightness &&
               list.compactTerrainFormat == s_chunkBatchCompactFormat &&
               list.primitive == s_chunkBatchPrimitive;
    }

    static void recordTerrainPassSignature(const WiiNativeChunkList& list)
    {
        s_terrainPassPrepared = true;
        s_terrainPassEarlyDepth = s_terrainEarlyDepth;
        s_terrainPassTexCoord = list.texCoordEnabled;
        s_terrainPassColor = list.colorEnabled;
        s_terrainPassBrightness = list.brightnessEnabled;
        s_terrainPassCompactFormat = list.compactTerrainFormat;
        s_terrainPassPrimitive = list.primitive;
    }

    static void recordChunkBatchSignature(const WiiNativeChunkList& list)
    {
        s_chunkBatchTexCoord = list.texCoordEnabled;
        s_chunkBatchColor = list.colorEnabled;
        s_chunkBatchBrightness = list.brightnessEnabled;
        s_chunkBatchCompactFormat = list.compactTerrainFormat;
        s_chunkBatchPrimitive = list.primitive;
    }

    static unsigned int align32(unsigned int value)
    {
        return (value + 31u) & ~31u;
    }

    static unsigned int bytesPerVertex(bool hasTexCoord, bool hasColor, bool hasNormals = false,
                                       bool hasBrightness = false)
    {
        return 12u + (hasNormals ? 12u : 0u) + (hasColor ? 4u : 0u) +
               (hasTexCoord ? 8u : 0u) + (hasBrightness ? 8u : 0u);
    }

    // Terrain chunks are local to a 16x16x16-ish section, so fixed-point S16
    // preserves far more precision than they need while cutting the direct GX
    // payload substantially. Keep an F32 fallback for unexpected geometry.
    //
    // The colour attribute drops the alpha byte for the same reason: it carries
    // no information. Every terrain quad reaches the Tessellator through
    // RenderBlocks, which only ever calls setColorOpaque_* -- there is not one
    // setColorRGBA call in RenderBlocks.cpp -- so vertex alpha is 255 on every
    // vertex of both passes. GX supplies 255 for a colour format without alpha,
    // which is what GX_CA_RASA reads in setupTextureStage(), so the translucent
    // pass still blends on texture alpha exactly as before.
    //
    // Deliberately NOT taken further: GX_RGB565 would save another byte but
    // quantises the biome tint and the smooth-lighting gradient to 5/6/5 bits,
    // and 32 levels of grey is inside the range Minecraft's shading actually
    // uses. GX_RGB8 is the part of the saving that costs no image quality.
    static constexpr int kTerrainPositionFrac = 8;
    static constexpr int kTerrainTexCoordFrac = 12;
    static constexpr float kTerrainPositionScale = (float)(1 << kTerrainPositionFrac);
    static constexpr float kTerrainTexCoordScale = (float)(1 << kTerrainTexCoordFrac);
    static unsigned int bytesPerTerrainVertex(bool hasTexCoord, bool hasColor,
                                               bool hasBrightness, bool compact)
    {
        if (!compact)
            return bytesPerVertex(hasTexCoord, hasColor, false, hasBrightness);
        return 6u + (hasColor ? 3u : 0u) + (hasTexCoord ? 4u : 0u) +
               (hasBrightness ? 4u : 0u);
    }

    static s16 fixedS16(float value, float scale)
    {
        float scaled = value * scale;
        if (scaled > 32767.0f) scaled = 32767.0f;
        if (scaled < -32768.0f) scaled = -32768.0f;
        const float rounded = scaled >= 0.0f ? scaled + 0.5f : scaled - 0.5f;
        return static_cast<s16>(rounded);
    }

    static void* acquireChunkBuffer(unsigned int requiredCapacity, unsigned int& actualCapacity)
    {
        size_t bestIndex = s_chunkBufferPool.size();
        unsigned int bestCapacity = 0;
        for (size_t i = 0; i < s_chunkBufferPool.size(); ++i)
        {
            const unsigned int capacity = s_chunkBufferPool[i].capacity;
            if (capacity < requiredCapacity)
                continue;
            if (bestIndex == s_chunkBufferPool.size() || capacity < bestCapacity)
            {
                bestIndex = i;
                bestCapacity = capacity;
            }
        }

        if (bestIndex != s_chunkBufferPool.size())
        {
            WiiChunkBufferPoolEntry entry = s_chunkBufferPool[bestIndex];
            s_chunkBufferPool.erase(s_chunkBufferPool.begin() + (ptrdiff_t)bestIndex);
            s_chunkBufferPoolBytes -= entry.capacity;
            actualCapacity = entry.capacity;
            ++s_chunkBufferPoolHits;
            return entry.data;
        }

        ++s_chunkBufferPoolMisses;
        actualCapacity = requiredCapacity;
        return memalign(32, requiredCapacity);
    }

    static void releaseChunkBuffer(void* data, unsigned int capacity)
    {
        if (!data)
            return;
        if (capacity == 0 || capacity > kChunkBufferPoolMaxBytes ||
            s_chunkBufferPoolBytes + capacity > kChunkBufferPoolMaxBytes)
        {
            if (capacity != 0)
                ++s_chunkBufferPoolOverflows;
            free(data);
            return;
        }

        WiiChunkBufferPoolEntry entry;
        entry.data = data;
        entry.capacity = capacity;
        s_chunkBufferPool.push_back(entry);
        s_chunkBufferPoolBytes += capacity;
    }

    static void freeChunkList(WiiNativeChunkList& list)
    {
        if (list.data)
            releaseChunkBuffer(list.data, list.capacity);
        list = WiiNativeChunkList();
    }

    static void freeStaticList(WiiNativeStaticList& list)
    {
        if (list.data)
            free(list.data);
        list = WiiNativeStaticList();
    }

    static bool getGxPrimitive(WiiRenderPrimitive primitive, u8& gxMode, int& primitiveVertices,
                               bool& continuousPrimitive, bool& closePrimitive)
    {
        gxMode = 0xff;
        primitiveVertices = 0;
        continuousPrimitive = false;
        closePrimitive = false;
        switch (primitive)
        {
            case WiiRenderPrimitive::Points:
                gxMode = GX_POINTS;
                primitiveVertices = 1;
                return true;
            case WiiRenderPrimitive::Lines:
                gxMode = GX_LINES;
                primitiveVertices = 2;
                return true;
            case WiiRenderPrimitive::LineStrip:
                gxMode = GX_LINESTRIP;
                continuousPrimitive = true;
                return true;
            case WiiRenderPrimitive::LineLoop:
                gxMode = GX_LINESTRIP;
                continuousPrimitive = true;
                closePrimitive = true;
                return true;
            case WiiRenderPrimitive::Triangles:
                gxMode = GX_TRIANGLES;
                primitiveVertices = 3;
                return true;
            case WiiRenderPrimitive::TriangleStrip:
                gxMode = GX_TRIANGLESTRIP;
                continuousPrimitive = true;
                return true;
            case WiiRenderPrimitive::TriangleFan:
                gxMode = GX_TRIANGLEFAN;
                continuousPrimitive = true;
                return true;
            case WiiRenderPrimitive::Quads:
                gxMode = GX_QUADS;
                primitiveVertices = 4;
                return true;
            default:
                return false;
        }
    }

    static void emitMeshVertex(const WiiNativeMeshView& mesh, const unsigned char* v,
                               const unsigned char* colorSource = nullptr)
    {
        const float* pos = reinterpret_cast<const float*>(v);
        GX_Position3f32(pos[0], pos[1], pos[2]);
        if (mesh.hasNormals)
        {
            const signed char* normal = reinterpret_cast<const signed char*>(v + 24);
            const float nx = normal[0] == -128 ? -1.0f : (float)normal[0] / 127.0f;
            const float ny = normal[1] == -128 ? -1.0f : (float)normal[1] / 127.0f;
            const float nz = normal[2] == -128 ? -1.0f : (float)normal[2] / 127.0f;
            GX_Normal3f32(nx, ny, nz);
        }
        if (mesh.colorEnabled)
        {
            const unsigned char* color = colorSource ? colorSource : v;
            GX_Color4u8(color[20], color[21], color[22], color[23]);
        }
        if (mesh.texCoordEnabled)
        {
            const float* uv = reinterpret_cast<const float*>(v + 12);
            GX_TexCoord2f32(uv[0], uv[1]);
        }
        if (mesh.brightnessEnabled)
        {
            uint32_t packed = 0;
            std::memcpy(&packed, v + mesh.brightnessOffset, sizeof(packed));
            GX_TexCoord2f32(static_cast<float>(packed & 0xffffu),
                            static_cast<float>((packed >> 16) & 0xffffu));
        }
    }

    static void emitTerrainVertex(const WiiNativeMeshView& mesh, const unsigned char* v, bool compact,
                                  const unsigned char* colorSource = nullptr)
    {
        if (!compact)
        {
            emitMeshVertex(mesh, v, colorSource);
            return;
        }

        const float* pos = reinterpret_cast<const float*>(v);
        GX_Position3s16(fixedS16(pos[0], kTerrainPositionScale),
                        fixedS16(pos[1], kTerrainPositionScale),
                        fixedS16(pos[2], kTerrainPositionScale));
        if (mesh.colorEnabled)
        {
            const unsigned char* color = colorSource ? colorSource : v;
            // Byte 23 of the Tessellator vertex is the alpha RenderBlocks always
            // writes as 255; see the note on kTerrainPositionFrac.
            GX_Color3u8(color[20], color[21], color[22]);
        }
        if (mesh.texCoordEnabled)
        {
            const float* uv = reinterpret_cast<const float*>(v + 12);
            GX_TexCoord2s16(fixedS16(uv[0], kTerrainTexCoordScale),
                            fixedS16(uv[1], kTerrainTexCoordScale));
        }
        if (mesh.brightnessEnabled)
        {
            uint32_t packed = 0;
            std::memcpy(&packed, v + mesh.brightnessOffset, sizeof(packed));
            GX_TexCoord2s16(static_cast<s16>(packed & 0xffffu),
                            static_cast<s16>((packed >> 16) & 0xffffu));
        }
    }



    static WiiNativeRenderStateSnapshot s_nativeState;
    static bool s_nativeStateValid = false;
    static Mtx s_nativeNormalMatrix;
    static bool s_nativeCpuNormalizeNormals = false;
    static void normalizeEyeNormal(float nx, float ny, float nz,
                                   float& outX, float& outY, float& outZ);

    static void emitImmediateMeshVertex(const WiiNativeMeshView& mesh, const unsigned char* v,
                                        const WiiNativeRenderStateSnapshot& state,
                                        const unsigned char* colorSource = nullptr)
    {
        const float* pos = reinterpret_cast<const float*>(v);
        GX_Position3f32(pos[0], pos[1], pos[2]);
        if (mesh.hasNormals)
        {
            const signed char* normal = reinterpret_cast<const signed char*>(v + 24);
            float nx = normal[0] == -128 ? -1.0f : (float)normal[0] / 127.0f;
            float ny = normal[1] == -128 ? -1.0f : (float)normal[1] / 127.0f;
            float nz = normal[2] == -128 ? -1.0f : (float)normal[2] / 127.0f;
            if (state.lighting_enabled && s_nativeCpuNormalizeNormals)
                normalizeEyeNormal(nx, ny, nz, nx, ny, nz);
            GX_Normal3f32(nx, ny, nz);
        }
        else if (state.lighting_enabled)
        {
            float nx = state.current_normal[0];
            float ny = state.current_normal[1];
            float nz = state.current_normal[2];
            if (s_nativeCpuNormalizeNormals)
                normalizeEyeNormal(nx, ny, nz, nx, ny, nz);
            GX_Normal3f32(nx, ny, nz);
        }
        if (mesh.colorEnabled)
        {
            const unsigned char* color = colorSource ? colorSource : v;
            GX_Color4u8(color[20], color[21], color[22], color[23]);
        }
        if (mesh.texCoordEnabled)
        {
            const float* uv = reinterpret_cast<const float*>(v + 12);
            GX_TexCoord2f32(uv[0], uv[1]);
        }
        if (mesh.brightnessEnabled)
        {
            uint32_t packed = 0;
            std::memcpy(&packed, v + mesh.brightnessOffset, sizeof(packed));
            GX_TexCoord2f32(static_cast<float>(packed & 0xffffu),
                            static_cast<float>((packed >> 16) & 0xffffu));
        }
    }
    static bool emitDisplayList(const WiiNativeMeshView& mesh, void* buffer,
                                unsigned int capacity, unsigned int& outSize,
                                bool compactTerrainFormat = false)
    {
        u8 gxMode = 0xff;
        int primitiveVertices = 0;
        bool continuousPrimitive = false;
        bool closePrimitive = false;
        if (!getGxPrimitive(mesh.primitive, gxMode, primitiveVertices,
                            continuousPrimitive, closePrimitive))
            return false;
        if (continuousPrimitive && mesh.count + (closePrimitive ? 1 : 0) > 65535)
            return false;

        WiiNativeRenderStateSnapshot compileState{};
        const bool flatShading = primitiveVertices > 0 &&
            wii_gx_get_native_state_snapshot(&compileState) &&
            compileState.shade_model == wiiRenderValue(WiiRenderShadeModel::Flat);

        // GX_BeginDispList() writes the command stream through the write-gather
        // pipe directly to main memory.  The CPU data cache is not coherent with
        // those writes.  Invalidate the destination *before* recording, exactly
        // like the proven OpenGX path did; otherwise a dirty/stale CPU cache line
        // can later be written back over the freshly recorded GX commands.
        //
        // Do not DCFlushRange() this buffer after GX_EndDispList(): the CPU did
        // not produce the payload, GX did.  Flushing afterwards is precisely the
        // operation that can corrupt a list recorded into memory whose cache lines
        // were still owned by the CPU.
        DCInvalidateRange(buffer, capacity);
        GX_BeginDispList(buffer, capacity);

        const unsigned char* base = static_cast<const unsigned char*>(mesh.vertices);
        int done = 0;
        while (done < mesh.count)
        {
            int batch = mesh.count - done;
            if (batch > 65535)
                batch = 65535;
            if (primitiveVertices > 0 && batch < mesh.count - done)
                batch -= batch % primitiveVertices;
            if (batch <= 0)
                break;

            const bool closeThisBatch = closePrimitive && done == 0 && batch == mesh.count;
            GX_Begin(gxMode, GX_VTXFMT0, (u16)(batch + (closeThisBatch ? 1 : 0)));
            for (int i = 0; i < batch + (closeThisBatch ? 1 : 0); ++i)
            {
                const int sourceIndex = (i == batch) ? 0 : i;
                const unsigned char* v = base +
                    (size_t)(mesh.first + done + sourceIndex) * (size_t)mesh.vertexStride;
                const unsigned char* colorSource = nullptr;
                if (flatShading && sourceIndex < batch)
                {
                    const int primitiveStart = (sourceIndex / primitiveVertices) * primitiveVertices;
                    const int colorIndex = primitiveStart + primitiveVertices - 1;
                    colorSource = base +
                        (size_t)(mesh.first + done + colorIndex) * (size_t)mesh.vertexStride;
                }
                if (compactTerrainFormat)
                    emitTerrainVertex(mesh, v, true, colorSource);
                else
                    emitMeshVertex(mesh, v, colorSource);
            }
            GX_End();
            done += batch;
        }

        outSize = GX_EndDispList();
        return done == mesh.count && outSize > 0 && outSize <= capacity;
    }

    // Records a chunk as one display list per face direction inside a single
    // buffer, instead of one list for the whole section.
    //
    // A display list is a GP command stream, so a draw cannot skip a span in the
    // middle of one. What it can do is call a list that starts at a 32-byte
    // boundary, which is why each direction is recorded as an independent list
    // and the cursor is realigned between them. The quads were already grouped
    // by direction by wii_sort_opaque_faces(), so each group is one contiguous
    // vertex range and no vertex data is duplicated -- the split costs one extra
    // GX_Begin plus alignment padding per direction.
    static bool emitChunkDisplayLists(const WiiNativeMeshView& mesh, void* buffer,
                                      unsigned int capacity, bool compactTerrainFormat,
                                      const WiiNativeChunkFaceGroups& groups,
                                      unsigned int* outOffsets, unsigned int* outSizes,
                                      unsigned int& outTotalSize)
    {
        // The split is expressed in quads. Any other primitive would need its
        // own group-to-vertex-range mapping, and the sort never produces one.
        if (mesh.primitive != WiiRenderPrimitive::Quads)
            return false;

        int totalQuads = 0;
        for (int g = 0; g < WiiNativeChunkFaceGroups::kGroupCount; ++g)
        {
            if (groups.quadCount[g] < 0)
                return false;
            totalQuads += groups.quadCount[g];
        }
        if (totalQuads <= 0 || totalQuads * 4 != mesh.count)
            return false;

        unsigned int cursor = 0;
        int emittedVertices = 0;
        for (int g = 0; g < WiiNativeChunkFaceGroups::kGroupCount; ++g)
        {
            outOffsets[g] = cursor;
            outSizes[g] = 0u;

            const int vertices = groups.quadCount[g] * 4;
            if (vertices <= 0)
                continue;
            if (cursor >= capacity)
                return false;

            WiiNativeMeshView groupMesh = mesh;
            groupMesh.first = mesh.first + emittedVertices;
            groupMesh.count = vertices;

            unsigned int size = 0;
            if (!emitDisplayList(groupMesh, static_cast<unsigned char*>(buffer) + cursor,
                                 capacity - cursor, size, compactTerrainFormat))
                return false;

            outSizes[g] = size;
            cursor = align32(cursor + size);
            emittedVertices += vertices;
        }

        outTotalSize = cursor;
        return emittedVertices == mesh.count && cursor > 0u && cursor <= capacity;
    }

    // Replays a chunk, dropping the face directions that point away from the
    // eye. GX would reject those triangles at the backface cull anyway, but only
    // after their vertices have gone through the write-gather pipe and the
    // transform unit, which is the cost this avoids.
    static void callChunkGeometry(const WiiNativeChunkList& list)
    {
        if (list.groupCount <= 0)
        {
            GX_CallDispList(list.data, list.size);
            return;
        }

#if PLATFORM_FACE_BUCKET_CULL
        // The section's plane extents are local, so the eye has to come back to
        // the same space. worldOrigin is the section's true world corner, not
        // the modelview's translate, which has already been through the
        // 1024-block wrap in WorldRenderer::setPosition().
        const bool cullGroups = s_terrainEyeValid;
        const float eyeLocal[3] = {
            s_terrainEyeWorld[0] - list.worldOrigin[0],
            s_terrainEyeWorld[1] - list.worldOrigin[1],
            s_terrainEyeWorld[2] - list.worldOrigin[2]
        };
#endif

        for (int g = 0; g < list.groupCount; ++g)
        {
            if (list.groupSize[g] == 0u)
                continue;

#if PLATFORM_FACE_BUCKET_CULL
            // The last group holds everything that is not an axis-aligned plane
            // (stairs, fences, crossed plants). It has no facing direction and
            // is always drawn.
            if (cullGroups && g < WiiNativeChunkFaceGroups::kGroupCount - 1)
            {
                const int axis = g >> 1;
                const bool positive = (g & 1) == 0;
                const float margin = (float)PLATFORM_FACE_CULL_EYE_MARGIN;
                const bool visible = positive
                    ? eyeLocal[axis] + margin > list.groupPlaneMin[g]
                    : eyeLocal[axis] - margin < list.groupPlaneMax[g];
                if (!visible)
                {
                    ++s_faceGroupsCulled;
                    continue;
                }
            }
#endif

            ++s_faceGroupsDrawn;
            GX_CallDispList(static_cast<unsigned char*>(list.data) + list.groupOffset[g],
                            list.groupSize[g]);
        }
    }


    static bool s_loadedTexObjValid[2] = {false, false};
    static GXTexObj s_loadedTexObj[2];

    static void invalidateLoadedTextureCache()
    {
        s_loadedTexObjValid[0] = false;
        s_loadedTexObjValid[1] = false;
    }

    static void loadTextureIfNeeded(const GXTexObj& texobj, int unit)
    {
        if (unit < 0 || unit > 1)
            return;
        if (!s_loadedTexObjValid[unit] ||
            std::memcmp(&s_loadedTexObj[unit], &texobj, sizeof(GXTexObj)) != 0)
        {
            GX_LoadTexObj(const_cast<GXTexObj*>(&texobj), unit == 0 ? GX_TEXMAP0 : GX_TEXMAP1);
            s_loadedTexObj[unit] = texobj;
            s_loadedTexObjValid[unit] = true;
        }
    }

    static unsigned char colorByte(float v)
    {
        if (v <= 0.0f) return 0;
        if (v >= 1.0f) return 255;
        return (unsigned char)(v * 255.0f + 0.5f);
    }

#if MC_LOG_LEVEL >= 2
    static int diagMilli(float v)
    {
        if (!std::isfinite(v)) return 2147483000;
        const float scaled = v * 1000.0f;
        if (scaled > 2147483000.0f) return 2147483000;
        if (scaled < -2147483000.0f) return -2147483000;
        return (int)scaled;
    }
#endif

    // Rejected submissions are destructive on Wii because there is no fallback
    // renderer. Keep counters and rate-limited diagnostics so unsupported state
    // is visible in debug logs without turning every frame into an I/O stall.
    static volatile unsigned int s_droppedSubmissions = 0;
    static volatile unsigned int s_textureFallbacks = 0;

    static void noteDroppedSubmission(const char* reason)
    {
        ++s_droppedSubmissions;
#if MC_LOG_LEVEL >= 1
        static unsigned int loggedDrops = 0;
        if (loggedDrops < 24)
        {
            MC_LOG_WARN("wii.render", "[WII][GX] dropped native submission: %s (total=%u)\n",
                        reason ? reason : "unknown", (unsigned int)s_droppedSubmissions);
            ++loggedDrops;
        }
#else
        (void)reason;
#endif
    }

    static void noteTextureFallback(int textureId)
    {
        ++s_textureFallbacks;
#if MC_LOG_LEVEL >= 1
        static unsigned int loggedFallbacks = 0;
        if (loggedFallbacks < 12)
        {
            MC_LOG_WARN("wii.render", "[WII][GX] texture %d is not resident; drawing untextured (total=%u)\n",
                        textureId, (unsigned int)s_textureFallbacks);
            ++loggedFallbacks;
        }
#else
        (void)textureId;
#endif
    }

    static GXColor gxColorFromFloat4(const float* c)
    {
        GXColor out = { colorByte(c[0]), colorByte(c[1]), colorByte(c[2]), colorByte(c[3]) };
        return out;
    }

    static void applyViewport(const WiiNativeRenderStateSnapshot& state)
    {
        const int x = state.viewport[0];
        const int y = state.viewport[1];
        const int width = state.viewport[2] > 0 ? state.viewport[2] : 1;
        const int height = state.viewport[3] > 0 ? state.viewport[3] : 1;
        GX_SetViewport(static_cast<float>(x), static_cast<float>(y),
                       static_cast<float>(width), static_cast<float>(height),
                       0.0f, 1.0f);
        GX_SetScissor(static_cast<u32>(x < 0 ? 0 : x),
                      static_cast<u32>(y < 0 ? 0 : y),
                      static_cast<u32>(width), static_cast<u32>(height));
    }

    static void applyDepthState(const WiiNativeRenderStateSnapshot& state)
    {
        const u8 depthTest = state.ztest ? GX_TRUE : GX_FALSE;
        const u8 depthWrite = (state.ztest && state.zwrite) ? GX_TRUE : GX_FALSE;
        GX_SetZMode(depthTest, state.zfunc, depthWrite);
    }

    static void applyPixelEngineState(const WiiNativeRenderStateSnapshot& state)
    {
        // Clear operations temporarily repurpose the PE for Z-texture writes.
        // Native draws must therefore restore every PE state they depend on
        // instead of relying on the previous GX command stream.
        GX_SetZTexture(GX_ZT_DISABLE, GX_TF_Z24X8, 0);
        applyDepthState(state);

        const u8 alphaFunc = state.alpha_test_enabled ? state.alpha_func : GX_ALWAYS;
        const u8 alphaRef = state.alpha_test_enabled ? state.alpha_ref : 0;
        GX_SetZCompLoc(state.alpha_test_enabled ? GX_DISABLE : GX_ENABLE);
        GX_SetAlphaCompare(alphaFunc, alphaRef, GX_AOP_AND, GX_ALWAYS, 0);

        if (state.blend_enabled)
            GX_SetBlendMode(GX_BM_BLEND, state.src_blend, state.dst_blend, GX_LO_CLEAR);
        else
            GX_SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);

        GX_SetColorUpdate(state.color_update ? GX_TRUE : GX_FALSE);
        GX_SetAlphaUpdate(state.alpha_update ? GX_TRUE : GX_FALSE);
    }

    static void setupCull(const WiiNativeRenderStateSnapshot& state)
    {
        if (!state.cull_enabled)
        {
            GX_SetCullMode(GX_CULL_NONE);
            return;
        }

        switch (state.cull_mode)
        {
            case wiiRenderValue(WiiRenderFace::Front):
                GX_SetCullMode(state.front_cw ? GX_CULL_FRONT : GX_CULL_BACK);
                break;
            case wiiRenderValue(WiiRenderFace::Back):
                GX_SetCullMode(state.front_cw ? GX_CULL_BACK : GX_CULL_FRONT);
                break;
            case wiiRenderValue(WiiRenderFace::FrontAndBack):
                GX_SetCullMode(GX_CULL_ALL);
                break;
            default:
                GX_SetCullMode(GX_CULL_NONE);
                break;
        }
    }

    static bool colorRgbIsBlack(const float* color)
    {
        return color[0] == 0.0f && color[1] == 0.0f && color[2] == 0.0f;
    }

    static bool nativeLightingSupported(const WiiNativeRenderStateSnapshot& state)
    {
        if (!state.lighting_enabled)
            return true;

        // Minecraft's standard item/entity lighting uses color material for
        // ambient+diffuse, directional lights, no emission and no specular.
        // More general OpenGL lighting stays on the compatibility path.
        if (!state.color_material_enabled ||
            state.color_material_mode != wiiRenderValue(WiiRenderColorMaterialMode::AmbientAndDiffuse) ||
            !colorRgbIsBlack(state.material_emission) ||
            !colorRgbIsBlack(state.material_specular))
            return false;

        for (int i = 0; i < WII_NATIVE_MAX_LIGHTS; ++i)
        {
            const WiiNativeLightSnapshot& light = state.lights[i];
            if (!light.enabled)
                continue;
            if (light.position[3] != 0.0f ||
                !colorRgbIsBlack(light.ambient) ||
                !colorRgbIsBlack(light.specular))
                return false;
        }
        return true;
    }

    static bool snapshotSupportedNativeState(WiiNativeRenderStateSnapshot& state, bool hasNormals)
    {
        if (!wii_gx_get_native_state_snapshot(&state))
        {
            noteDroppedSubmission("state-snapshot");
            return false;
        }
        if ((state.texture_enabled & ~3u) != 0)
        {
            noteDroppedSubmission("state-texture-unit");
            return false;
        }
        if (state.lighting_enabled && !nativeLightingSupported(state))
        {
            noteDroppedSubmission("state-lighting");
            return false;
        }
        return true;
    }

    static void buildNormalMatrix(const WiiNativeRenderStateSnapshot& state, Mtx normal)
    {
        Mtx inverse;
        if (wiiMtxInverse(state.modelview, inverse) == 0)
            std::memcpy(normal, state.modelview, sizeof(Mtx));
        else
            wiiMtxTranspose(inverse, normal);

        normal[0][3] = 0.0f;
        normal[1][3] = 0.0f;
        normal[2][3] = 0.0f;

        if (state.rescale_normals && !state.normalize_normals)
        {
            const float scaleSquared = state.modelview[0][0] * state.modelview[0][0] +
                                       state.modelview[1][0] * state.modelview[1][0] +
                                       state.modelview[2][0] * state.modelview[2][0];
            if (scaleSquared > 1.0e-12f)
            {
                const float scale = sqrtf(scaleSquared);
                for (int row = 0; row < 3; ++row)
                    for (int column = 0; column < 3; ++column)
                        normal[row][column] *= scale;
            }
        }
    }

    static void normalizeEyeNormal(float nx, float ny, float nz,
                                   float& outX, float& outY, float& outZ)
    {
        outX = s_nativeNormalMatrix[0][0] * nx + s_nativeNormalMatrix[0][1] * ny +
               s_nativeNormalMatrix[0][2] * nz;
        outY = s_nativeNormalMatrix[1][0] * nx + s_nativeNormalMatrix[1][1] * ny +
               s_nativeNormalMatrix[1][2] * nz;
        outZ = s_nativeNormalMatrix[2][0] * nx + s_nativeNormalMatrix[2][1] * ny +
               s_nativeNormalMatrix[2][2] * nz;
        const float lengthSquared = outX * outX + outY * outY + outZ * outZ;
        if (lengthSquared <= 1.0e-12f)
            return;
        const float inverseLength = 1.0f / sqrtf(lengthSquared);
        outX *= inverseLength;
        outY *= inverseLength;
        outZ *= inverseLength;
    }

    static void loadNormalMatrix(const WiiNativeRenderStateSnapshot& state, bool immediateDraw)
    {
        buildNormalMatrix(state, s_nativeNormalMatrix);
        s_nativeCpuNormalizeNormals = immediateDraw && state.normalize_normals;
        if (s_nativeCpuNormalizeNormals)
        {
            Mtx identity;
            wiiMtxIdentity(identity);
            GX_LoadNrmMtxImm(identity, GX_PNMTX0);
            return;
        }

        Mtx normal;
        std::memcpy(normal, s_nativeNormalMatrix, sizeof(Mtx));
        if (state.normalize_normals)
        {
            // Static display lists cannot rewrite their normals per draw. Their
            // normals are axis-aligned in the retained Minecraft geometry, so
            // normalizing the transformed basis columns preserves GL_NORMALIZE
            // semantics for those faces without changing their direction.
            for (int column = 0; column < 3; ++column)
            {
                const float x = normal[0][column];
                const float y = normal[1][column];
                const float z = normal[2][column];
                const float lengthSquared = x * x + y * y + z * z;
                if (lengthSquared > 1.0e-12f)
                {
                    const float inverseLength = 1.0f / sqrtf(lengthSquared);
                    normal[0][column] = x * inverseLength;
                    normal[1][column] = y * inverseLength;
                    normal[2][column] = z * inverseLength;
                }
            }
        }
        GX_LoadNrmMtxImm(normal, GX_PNMTX0);
    }

    static u8 setupNativeLighting(const WiiNativeRenderStateSnapshot& state, bool colorEnabled)
    {
        u8 lightMask = GX_LIGHTNULL;
        int gxLightIndex = 0;
        static GXLightObj lightObjects[WII_NATIVE_MAX_LIGHTS];

        for (int i = 0; i < WII_NATIVE_MAX_LIGHTS && gxLightIndex < 8; ++i)
        {
            const WiiNativeLightSnapshot& light = state.lights[i];
            if (!light.enabled || colorRgbIsBlack(light.diffuse))
                continue;

            GXLightObj& object = lightObjects[gxLightIndex];
            GX_InitLightColor(&object, gxColorFromFloat4(light.diffuse));
            GX_InitLightPos(&object, light.position[0], light.position[1], light.position[2]);
            GX_InitLightAttn(&object, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
            const u8 id = (u8)(1u << gxLightIndex);
            GX_LoadLightObj(&object, id);
            lightMask |= id;
            ++gxLightIndex;
        }

        GX_SetNumChans(1);
        GX_SetChanAmbColor(GX_COLOR0A0, gxColorFromFloat4(state.global_ambient));
        if (colorEnabled)
        {
            GX_SetChanCtrl(GX_COLOR0A0, GX_ENABLE, GX_SRC_REG, GX_SRC_VTX,
                           lightMask, GX_DF_CLAMP, GX_AF_NONE);
        }
        else
        {
            GX_SetChanMatColor(GX_COLOR0A0, gxColorFromFloat4(state.current_color));
            GX_SetChanCtrl(GX_COLOR0A0, GX_ENABLE, GX_SRC_REG, GX_SRC_REG,
                           lightMask, GX_DF_CLAMP, GX_AF_NONE);
        }
        return lightMask;
    }

    static bool snapshotSupportedTerrainState(WiiNativeRenderStateSnapshot& state)
    {
        if (!wii_gx_get_native_state_snapshot(&state))
        {
            noteDroppedSubmission("terrain-snapshot");
            return false;
        }
        if (state.lighting_enabled)
        {
            noteDroppedSubmission("terrain-lighting");
            return false;
        }
        if ((state.texture_enabled & ~3u) != 0)
        {
            noteDroppedSubmission("terrain-texture-unit");
            return false;
        }
        return true;
    }


    static void applyFog(const WiiNativeRenderStateSnapshot& state)
    {
        // Fog parameters remain cached even after fog is disabled.  Applying
        // them unconditionally breaks orthographic GUI rendering because the
        // overlay depth range is far outside the world's fog start/end.
        if (!state.fog_enabled || !s_nativeFog.valid)
        {
            GX_SetFog(GX_FOG_NONE, 0.0f, 1.0f, 0.1f, 1.0f, GXColor{0, 0, 0, 0});
            return;
        }
        int fogMode = s_nativeFog.mode;
        float density = s_nativeFog.density;
        float start = s_nativeFog.start;
        float end = s_nativeFog.end;
        const float* color = s_nativeFog.color;

        u8 projectionType;
        float nearPlane, farPlane;
        wiiGetOpenGLProjectionInfo(state.projection, projectionType, nearPlane, farPlane);
        u8 mode = GX_FOG_NONE;
        switch (fogMode)
        {
            case wiiRenderValue(WiiRenderFogMode::Exp): mode = GX_FOG_EXP; break;
            case wiiRenderValue(WiiRenderFogMode::Exp2): mode = GX_FOG_EXP2; break;
            case wiiRenderValue(WiiRenderFogMode::Linear): mode = GX_FOG_LIN; break;
        }
        if (mode != GX_FOG_NONE && projectionType == GX_ORTHOGRAPHIC)
            mode += (GX_FOG_ORTHO_LIN - GX_FOG_PERSP_LIN);
        if (fogMode != wiiRenderValue(WiiRenderFogMode::Linear))
        {
            start = nearPlane;
            if (density <= 0.0f) end = farPlane;
            else if (fogMode == wiiRenderValue(WiiRenderFogMode::Exp2)) end = 2.0f / density;
            else end = 5.0f / density;
        }
        GX_SetFog(mode, start, end, nearPlane, farPlane, gxColorFromFloat4(color));
    }

    static void setupTextureMatrix(const WiiNativeRenderStateSnapshot& state, int unit,
                                   bool perVertexCoordinate)
    {
        const float (*source)[4] = unit == 0 ? state.texture0_matrix : state.texture1_matrix;
        const bool identity = unit == 0 ? state.texture0_identity != 0 : state.texture1_identity != 0;
        const u32 dttMatrix = unit == 0 ? GX_DTTMTX0 : GX_DTTMTX1;
        const u16 texCoord = unit == 0 ? GX_TEXCOORD0 : GX_TEXCOORD1;
        const u32 texSource = unit == 0 ? GX_TG_TEX0 : GX_TG_TEX1;

        Mtx textureMatrix;
        if (unit == 1 && !perVertexCoordinate)
        {
            // OpenGL applies the current secondary texture coordinate to every
            // vertex while the client TEX1 array is disabled. GX has no current
            // texcoord register, so generate the same constant from position
            // and a post-transform matrix with zero scale.
            const float u = state.texture1_coord[0];
            const float v = state.texture1_coord[1];
            const float transformedU = source[0][0] * u + source[0][1] * v + source[0][3];
            const float transformedV = source[1][0] * u + source[1][1] * v + source[1][3];
            std::memset(textureMatrix, 0, sizeof(textureMatrix));
            textureMatrix[0][3] = transformedU;
            textureMatrix[1][3] = transformedV;
            textureMatrix[2][2] = 1.0f;
            GX_LoadTexMtxImm(textureMatrix, dttMatrix, GX_MTX3x4);
            GX_SetTexCoordGen2(texCoord, GX_TG_MTX2x4, GX_TG_POS,
                               GX_IDENTITY, GX_FALSE, dttMatrix);
            GX_EnableTexOffsets(texCoord, GX_DISABLE, GX_DISABLE);
            return;
        }

        if (identity)
        {
            wiiMtxIdentity(textureMatrix);
        }
        else
        {
            std::memcpy(textureMatrix, source, sizeof(textureMatrix));
            textureMatrix[2][0] = 0.0f;
            textureMatrix[2][1] = 0.0f;
            textureMatrix[2][2] = 1.0f;
            textureMatrix[2][3] = 0.0f;
        }
        GX_LoadTexMtxImm(textureMatrix, dttMatrix, GX_MTX3x4);
        GX_SetTexCoordGen2(texCoord, GX_TG_MTX2x4, texSource,
                           GX_IDENTITY, GX_FALSE, dttMatrix);
        GX_EnableTexOffsets(texCoord, GX_DISABLE, GX_DISABLE);
    }

    static bool resolveBoundTexture(const WiiNativeRenderStateSnapshot& state,
                                    int unit, GXTexObj* texobj)
    {
        if (unit < 0 || unit > 1 || (state.texture_enabled & (1u << unit)) == 0)
            return false;
        const int textureId = unit == 0 ? state.texture0_id : state.texture1_id;
        if (wii_native_texture_get_texobj_c(textureId, texobj) != 0)
            return true;
        noteTextureFallback(textureId);
        return false;
    }

    static void setupTextureStage(const WiiNativeRenderStateSnapshot& state,
                                  bool hasTexCoord, bool hasBrightness,
                                  bool textured, bool lightmapped)
    {
        GX_SetNumIndStages(0);
        GX_SetTevDirect(GX_TEVSTAGE0);
        GX_SetTevDirect(GX_TEVSTAGE1);

        if (lightmapped)
        {
            // TEXCOORD1 is needed even for models that use OpenGL's current
            // secondary texture coordinate instead of a brightness array.
            GX_SetNumTexGens(2);
            if (hasTexCoord)
                setupTextureMatrix(state, 0, true);
            else
            {
                Mtx identity;
                wiiMtxIdentity(identity);
                GX_LoadTexMtxImm(identity, GX_DTTMTX0, GX_MTX3x4);
                GX_SetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_POS,
                                   GX_IDENTITY, GX_FALSE, GX_DTTMTX0);
            }
            setupTextureMatrix(state, 1, hasBrightness);
        }
        else if (hasTexCoord)
        {
            GX_SetNumTexGens(1);
            setupTextureMatrix(state, 0, true);
        }
        else
        {
            GX_SetNumTexGens(0);
        }

        GX_SetNumTevStages(lightmapped ? 2 : 1);
        if (textured && hasTexCoord)
        {
            GX_SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO);
            GX_SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO);
            GX_SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            GX_SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        }
        else
        {
            GX_SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
            GX_SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
            GX_SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            GX_SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_DISABLE, GX_COLOR0A0);
        }

        if (lightmapped)
        {
            // Fixed-function OpenGL modulates the previous fragment by the
            // lightmap texture on texture unit 1.
            GX_SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_TEXC, GX_CC_ZERO);
            GX_SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_APREV, GX_CA_TEXA, GX_CA_ZERO);
            GX_SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            GX_SetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            GX_SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLORNULL);
        }
    }

    static void applyTerrainPassState(WiiNativeRenderStateSnapshot& state)
    {
        // Terrain passes have stronger fixed-function guarantees than a normal
        // immediate draw. The previous Wii terrain path imposed these states
        // per pass instead of inheriting whatever a GUI or helper left behind.
        //
        // Fog is deliberately not one of them. EntityRenderer::setupFog() keeps
        // the tracked GL fog state correct for both the native and the fallback
        // path, and forcing it off here was the one thing that made terrain the
        // only geometry in the world drawn without distance fog.
        state.ztest = 1;
        state.zwrite = 1;

        switch (s_terrainPassMode)
        {
            case WII_NATIVE_TERRAIN_OPAQUE:
                state.blend_enabled = 0;
                state.cull_enabled = 1;
                state.color_update = 1;
                // The tracked GL state has the alpha test on for the whole world
                // render -- GuiIngame enables it every frame and nothing turns it
                // off before pass 0 -- and applyPixelEngineState() answers that
                // with GX_SetZCompLoc(GX_DISABLE), so every opaque fragment gets
                // textured before it can be rejected against Z. For a group of
                // sections that provably has no alpha-cut texels, dropping the
                // test is image-identical (GX_GREATER against an alpha of 255
                // always passes) and moves the depth compare back in front of
                // the TEV stages.
                if (s_terrainEarlyDepth)
                    state.alpha_test_enabled = 0;
                break;

            case WII_NATIVE_TERRAIN_TRANSLUCENT_DEPTH:
                state.blend_enabled = 1;
                state.src_blend = GX_BL_SRCALPHA;
                state.dst_blend = GX_BL_INVSRCALPHA;
                state.cull_enabled = 0;
                state.color_update = 0;
                break;

            case WII_NATIVE_TERRAIN_TRANSLUCENT_COLOR:
                state.blend_enabled = 1;
                state.src_blend = GX_BL_SRCALPHA;
                state.dst_blend = GX_BL_INVSRCALPHA;
                state.cull_enabled = 0;
                state.color_update = 1;
                break;
        }
    }

#if MC_LOG_LEVEL >= 2
    static void logTerrainStateOnce(const WiiNativeRenderStateSnapshot& state,
                                    bool hasTexCoord, bool hasColor, bool textured);
#endif

    static bool prepareNativeTerrainBatch(bool hasTexCoord, bool hasColor, bool hasBrightness,
                                          bool compactTerrainFormat)
    {
        WiiNativeRenderStateSnapshot state;
        if (!snapshotSupportedTerrainState(state))
            return false;

        // Chunk lists are compiled without the brightness attribute
        // (renderCompileTerrainMesh), so a lightmap unit left enabled by
        // RenderLiving or a GUI would take the constant secondary coordinate
        // path in setupTextureMatrix() and modulate the whole terrain by one
        // stale lightmap texel. Third person keeps that coordinate frozen --
        // ItemRenderer only refreshes it in first person -- which is what
        // turned the world black. Batches without per-vertex brightness ignore
        // the unit instead of inheriting it.
        if (!hasBrightness)
            state.texture_enabled &= ~2u;

        if (s_terrainPassActive)
            applyTerrainPassState(state);

        GXTexObj texobj0;
        GXTexObj texobj1;
        const bool textured = hasTexCoord && resolveBoundTexture(state, 0, &texobj0);
        const bool lightmapped = resolveBoundTexture(state, 1, &texobj1);
        if (textured)
            loadTextureIfNeeded(texobj0, 0);
        if (lightmapped)
            loadTextureIfNeeded(texobj1, 1);
#if MC_LOG_LEVEL >= 2
        logTerrainStateOnce(state, hasTexCoord, hasColor, textured);
#endif

        s_nativeState = state;
        s_nativeStateValid = true;
        applyViewport(state);
        wiiLoadProjectionFromOpenGL(state.projection, state.polygon_offset_fill != 0, state.polygon_offset_units);
        GX_LoadPosMtxImm(state.modelview, GX_PNMTX0);
        GX_SetCurrentMtx(GX_PNMTX0);

        // applyTerrainPassState() reduces the pass to one complete state
        // snapshot. Keep a single GX application path instead of layering
        // terrain overrides over the generic pixel-engine state afterwards.
        applyPixelEngineState(state);
        setupCull(state);
        applyFog(state);

        GX_SetNumChans(1);
        if (hasColor)
        {
            GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX,
                           GX_LIGHTNULL, GX_DF_NONE, GX_AF_NONE);
        }
        else
        {
            GX_SetChanMatColor(GX_COLOR0A0, gxColorFromFloat4(state.current_color));
            GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG,
                           GX_LIGHTNULL, GX_DF_NONE, GX_AF_NONE);
        }

        setupTextureStage(state, hasTexCoord, hasBrightness, textured, lightmapped);

        GX_ClearVtxDesc();
        GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ,
                         compactTerrainFormat ? GX_S16 : GX_F32,
                         compactTerrainFormat ? kTerrainPositionFrac : 0);
        if (hasColor)
        {
            GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
            // Must track emitTerrainVertex(): the compact path writes three
            // bytes, the F32 fallback path goes through emitMeshVertex() and
            // writes four.
            if (compactTerrainFormat)
                GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGB, GX_RGB8, 0);
            else
                GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        }
        if (hasTexCoord)
        {
            GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
            GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST,
                             compactTerrainFormat ? GX_S16 : GX_F32,
                             compactTerrainFormat ? kTerrainTexCoordFrac : 0);
        }
        if (hasBrightness)
        {
            GX_SetVtxDesc(GX_VA_TEX1, GX_DIRECT);
            GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST,
                             compactTerrainFormat ? GX_S16 : GX_F32, 0);
        }
        return true;
    }

    static void buildChunkModelview(Mtx modelview, float x, float y, float z, float sectionSize)
    {
        // RenderList changes the logical modelview between groups. Start from
        // that live matrix, then reproduce the transform that the old OpenGX
        // display list recorded around each local chunk mesh.
        wii_gx_get_native_modelview(modelview);
        const float half = sectionSize * 0.5f;
        const float scale = 1.000001f;
        wiiMtxApplyTrans(modelview, modelview, x, y, z);
        wiiMtxApplyTrans(modelview, modelview, -half, -half, -half);
        wiiMtxApplyScale(modelview, modelview, scale, scale, scale);
        wiiMtxApplyTrans(modelview, modelview, half, half, half);
    }

    static void loadChunkTransform(float x, float y, float z, float sectionSize)
    {
        if (!s_nativeStateValid)
            return;
        Mtx modelview;
        buildChunkModelview(modelview, x, y, z, sectionSize);
        GX_LoadPosMtxImm(modelview, GX_PNMTX0);
        GX_SetCurrentMtx(GX_PNMTX0);
    }

#if MC_LOG_LEVEL >= 2
    static void logTerrainStateOnce(const WiiNativeRenderStateSnapshot& state,
                                    bool hasTexCoord, bool hasColor, bool textured)
    {
        static bool loggedTerrainState = false;
        if (loggedTerrainState)
            return;
        loggedTerrainState = true;

        u8 projectionType = GX_PERSPECTIVE;
        float nearPlane = 0.0f;
        float farPlane = 0.0f;
        wiiGetOpenGLProjectionInfo(state.projection, projectionType, nearPlane, farPlane);
        MC_LOG_DEBUG("wii.render", "[WII][GX][3D][STATE] pass=%d tex=%d texId=%d resident=%d texIdentity=%d color=%d z=%d/%u/%d write=%d/%d alpha=%d/%u/%u blend=%d/%u/%u cull=%d/%d frontcw=%d fog=%d mode=%d start=%d end=%d near=%d far=%d proj=%s\n",
               (int)s_terrainPassMode, hasTexCoord ? 1 : 0, state.texture0_id,
               textured ? 1 : 0, state.texture0_identity ? 1 : 0, hasColor ? 1 : 0,
               state.ztest ? 1 : 0, (unsigned int)state.zfunc, state.zwrite ? 1 : 0,
               state.color_update ? 1 : 0, state.alpha_update ? 1 : 0,
               state.alpha_test_enabled ? 1 : 0, (unsigned int)state.alpha_func,
               (unsigned int)state.alpha_ref, state.blend_enabled ? 1 : 0,
               (unsigned int)state.src_blend, (unsigned int)state.dst_blend,
               state.cull_enabled ? 1 : 0, state.cull_mode, state.front_cw,
               state.fog_enabled ? 1 : 0, s_nativeFog.mode, diagMilli(s_nativeFog.start),
               diagMilli(s_nativeFog.end), diagMilli(nearPlane), diagMilli(farPlane),
               projectionType == GX_PERSPECTIVE ? "persp" : "ortho");
        for (int row = 0; row < 3; ++row)
            MC_LOG_DEBUG("wii.render", "[WII][GX][3D][MV%d] %d %d %d %d\n", row,
                   diagMilli(state.modelview[row][0]), diagMilli(state.modelview[row][1]),
                   diagMilli(state.modelview[row][2]), diagMilli(state.modelview[row][3]));
        for (int row = 0; row < 4; ++row)
            MC_LOG_DEBUG("wii.render", "[WII][GX][3D][PR%d] %d %d %d %d\n", row,
                   diagMilli(state.projection[row][0]), diagMilli(state.projection[row][1]),
                   diagMilli(state.projection[row][2]), diagMilli(state.projection[row][3]));
    }

    static void logChunkTransform(const WiiNativeChunkList& list, int handle)
    {
        static unsigned int loggedChunkTransforms = 0;
        if (loggedChunkTransforms >= 12)
            return;

        Mtx modelview;
        buildChunkModelview(modelview, list.translateX, list.translateY, list.translateZ, list.sectionSize);

        const float cx = (list.localMin[0] + list.localMax[0]) * 0.5f;
        const float cy = (list.localMin[1] + list.localMax[1]) * 0.5f;
        const float cz = (list.localMin[2] + list.localMax[2]) * 0.5f;
        const float eyeX = modelview[0][0] * cx + modelview[0][1] * cy + modelview[0][2] * cz + modelview[0][3];
        const float eyeY = modelview[1][0] * cx + modelview[1][1] * cy + modelview[1][2] * cz + modelview[1][3];
        const float eyeZ = modelview[2][0] * cx + modelview[2][1] * cy + modelview[2][2] * cz + modelview[2][3];
        const float clipX = s_nativeState.projection[0][0] * eyeX + s_nativeState.projection[0][1] * eyeY + s_nativeState.projection[0][2] * eyeZ + s_nativeState.projection[0][3];
        const float clipY = s_nativeState.projection[1][0] * eyeX + s_nativeState.projection[1][1] * eyeY + s_nativeState.projection[1][2] * eyeZ + s_nativeState.projection[1][3];
        const float clipZ = s_nativeState.projection[2][0] * eyeX + s_nativeState.projection[2][1] * eyeY + s_nativeState.projection[2][2] * eyeZ + s_nativeState.projection[2][3];
        const float clipW = s_nativeState.projection[3][0] * eyeX + s_nativeState.projection[3][1] * eyeY + s_nativeState.projection[3][2] * eyeZ + s_nativeState.projection[3][3];
        float ndcX = 9999.0f, ndcY = 9999.0f, ndcZ = 9999.0f;
        if (std::fabs(clipW) > 1.0e-6f)
        {
            ndcX = clipX / clipW;
            ndcY = clipY / clipW;
            ndcZ = clipZ / clipW;
        }
        MC_LOG_DEBUG("wii.render", "[WII][GX][3D][CHUNK] h=%d pos=(%d,%d,%d) local=(%d,%d,%d)..(%d,%d,%d) first=(%d,%d,%d) uv=(%d,%d) rgba=%u,%u,%u,%u eye=(%d,%d,%d) clip=(%d,%d,%d,%d) ndc=(%d,%d,%d)\n",
               handle, diagMilli(list.translateX), diagMilli(list.translateY), diagMilli(list.translateZ),
               diagMilli(list.localMin[0]), diagMilli(list.localMin[1]), diagMilli(list.localMin[2]),
               diagMilli(list.localMax[0]), diagMilli(list.localMax[1]), diagMilli(list.localMax[2]),
               diagMilli(list.firstPosition[0]), diagMilli(list.firstPosition[1]), diagMilli(list.firstPosition[2]),
               diagMilli(list.firstUv[0]), diagMilli(list.firstUv[1]),
               (unsigned int)list.firstColor[0], (unsigned int)list.firstColor[1],
               (unsigned int)list.firstColor[2], (unsigned int)list.firstColor[3],
               diagMilli(eyeX), diagMilli(eyeY), diagMilli(eyeZ),
               diagMilli(clipX), diagMilli(clipY), diagMilli(clipZ), diagMilli(clipW),
               diagMilli(ndcX), diagMilli(ndcY), diagMilli(ndcZ));
        for (int row = 0; row < 3; ++row)
            MC_LOG_DEBUG("wii.render", "[WII][GX][3D][CMV%d] h=%d %d %d %d %d\n", row, handle,
                   diagMilli(modelview[row][0]), diagMilli(modelview[row][1]),
                   diagMilli(modelview[row][2]), diagMilli(modelview[row][3]));
        ++loggedChunkTransforms;
    }
#endif

    // emitsConstantNormal says whether the caller's vertex emitter will supply a
    // normal for a mesh that does not carry one. Only the immediate path can: a
    // recorded display list has the layout it was built with, so asking the
    // descriptor for an attribute its bytes lack would misparse the stream.
    static bool prepareNativeMeshState(const WiiNativeMeshView& mesh, bool emitsConstantNormal)
    {
        WiiNativeRenderStateSnapshot state;
        if (!snapshotSupportedNativeState(state, mesh.hasNormals))
            return false;

        const bool constantNormal = emitsConstantNormal &&
                                    state.lighting_enabled && !mesh.hasNormals;
        if (constantNormal && mesh.vertices == nullptr)
        {
            noteDroppedSubmission("mesh-normal-source");
            return false;
        }

        GXTexObj texobj0;
        GXTexObj texobj1;
        const bool textured = mesh.texCoordEnabled && resolveBoundTexture(state, 0, &texobj0);
        const bool lightmapped = resolveBoundTexture(state, 1, &texobj1);
        if (textured)
            loadTextureIfNeeded(texobj0, 0);
        if (lightmapped)
            loadTextureIfNeeded(texobj1, 1);

        s_nativeState = state;
        s_nativeStateValid = true;
        applyViewport(state);
        wiiLoadProjectionFromOpenGL(state.projection, state.polygon_offset_fill != 0, state.polygon_offset_units);
        GX_LoadPosMtxImm(state.modelview, GX_PNMTX0);
        GX_SetCurrentMtx(GX_PNMTX0);
        if (state.lighting_enabled)
            loadNormalMatrix(state, emitsConstantNormal);
        else
            s_nativeCpuNormalizeNormals = false;

        applyPixelEngineState(state);
        setupCull(state);
        applyFog(state);

        if (state.lighting_enabled)
        {
            setupNativeLighting(state, mesh.colorEnabled);
        }
        else
        {
            GX_SetNumChans(1);
            if (mesh.colorEnabled)
            {
                GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX,
                               GX_LIGHTNULL, GX_DF_NONE, GX_AF_NONE);
            }
            else
            {
                GX_SetChanMatColor(GX_COLOR0A0, gxColorFromFloat4(state.current_color));
                GX_SetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG,
                               GX_LIGHTNULL, GX_DF_NONE, GX_AF_NONE);
            }
        }

        setupTextureStage(state, mesh.texCoordEnabled, mesh.brightnessEnabled,
                          textured, lightmapped);

        GX_ClearVtxDesc();
        GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        if (mesh.hasNormals || constantNormal)
        {
            GX_SetVtxDesc(GX_VA_NRM, GX_DIRECT);
            GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
        }
        if (mesh.colorEnabled)
        {
            GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
            GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        }
        if (mesh.texCoordEnabled)
        {
            GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
            GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        }
        if (mesh.brightnessEnabled)
        {
            GX_SetVtxDesc(GX_VA_TEX1, GX_DIRECT);
            GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_F32, 0);
        }
        return true;
    }

}


void wii_native_invalidate_texture_binding_cache()
{
    invalidateLoadedTextureCache();
}

int wii_native_create_chunk_handle()
{
    // Handles are process-local opaque IDs. Zero remains the invalid sentinel.
    // Renderer count is bounded, so monotonic allocation cannot realistically
    // wrap during one game session and avoids free-list aliasing bugs.
    return s_nextChunkHandle++;
}

void wii_native_destroy_chunk_handle(int handle)
{
    wii_native_clear_chunk_list(handle);
}

bool wii_native_draw_mesh(const WiiNativeMeshView& mesh)
{
    if (!mesh.vertices || mesh.count <= 0 || mesh.first < 0 || mesh.vertexStride != 32)
    {
        noteDroppedSubmission("mesh-shape");
        return false;
    }

    u8 gxMode = 0xff;
    int primitiveVertices = 0;
    bool continuousPrimitive = false;
    bool closePrimitive = false;
    if (!getGxPrimitive(mesh.primitive, gxMode, primitiveVertices,
                        continuousPrimitive, closePrimitive))
    {
        noteDroppedSubmission("mesh-primitive");
        return false;
    }
    if (continuousPrimitive && mesh.count + (closePrimitive ? 1 : 0) > 65535)
    {
        noteDroppedSubmission("mesh-vertex-count");
        return false;
    }
    if (!prepareNativeMeshState(mesh, true))
        return false;

#if MC_LOG_LEVEL >= 2
    {
        static unsigned int loggedDraws = 0;
        if (loggedDraws < 32)
        {
            const unsigned char* firstVertex = static_cast<const unsigned char*>(mesh.vertices) +
                (size_t)mesh.first * (size_t)mesh.vertexStride;
            const float* p = reinterpret_cast<const float*>(firstVertex);
            const float* uv = reinterpret_cast<const float*>(firstVertex + 12);
            GXTexObj debugTexObj;
            const int resident = ((s_nativeState.texture_enabled & 1u) != 0) &&
                wii_native_texture_get_texobj_c(s_nativeState.texture0_id, &debugTexObj);
            const float ex = s_nativeState.modelview[0][0] * p[0] + s_nativeState.modelview[0][1] * p[1] +
                             s_nativeState.modelview[0][2] * p[2] + s_nativeState.modelview[0][3];
            const float ey = s_nativeState.modelview[1][0] * p[0] + s_nativeState.modelview[1][1] * p[1] +
                             s_nativeState.modelview[1][2] * p[2] + s_nativeState.modelview[1][3];
            const float ez = s_nativeState.modelview[2][0] * p[0] + s_nativeState.modelview[2][1] * p[1] +
                             s_nativeState.modelview[2][2] * p[2] + s_nativeState.modelview[2][3];
            const float cx = s_nativeState.projection[0][0] * ex + s_nativeState.projection[0][1] * ey +
                             s_nativeState.projection[0][2] * ez + s_nativeState.projection[0][3];
            const float cy = s_nativeState.projection[1][0] * ex + s_nativeState.projection[1][1] * ey +
                             s_nativeState.projection[1][2] * ez + s_nativeState.projection[1][3];
            const float cz = s_nativeState.projection[2][0] * ex + s_nativeState.projection[2][1] * ey +
                             s_nativeState.projection[2][2] * ez + s_nativeState.projection[2][3];
            const float cw = s_nativeState.projection[3][0] * ex + s_nativeState.projection[3][1] * ey +
                             s_nativeState.projection[3][2] * ez + s_nativeState.projection[3][3];
            const float invW = std::fabs(cw) > 1.0e-12f ? 1.0f / cw : 0.0f;
            MC_LOG_DEBUG("wii.render", "[WII][GX][DRAW] n=%u prim=%u verts=%d texcoord=%d color=%d normal=%d "
                   "texEn=%u texId=%d resident=%d alpha=%u/%u/%u blend=%u depth=%u/%u "
                   "write=%u/%u vp=%d,%d,%d,%d v0=(%d,%d,%d) uv=(%d,%d) rgba=%u,%u,%u,%u "
                   "cur=%u,%u,%u,%u ndc=(%d,%d,%d) w=%d\n",
                   loggedDraws, (unsigned int)mesh.primitive, mesh.count,
                   mesh.texCoordEnabled ? 1 : 0, mesh.colorEnabled ? 1 : 0, mesh.hasNormals ? 1 : 0,
                   (unsigned int)s_nativeState.texture_enabled, s_nativeState.texture0_id, resident,
                   (unsigned int)s_nativeState.alpha_test_enabled, (unsigned int)s_nativeState.alpha_func,
                   (unsigned int)s_nativeState.alpha_ref, (unsigned int)s_nativeState.blend_enabled,
                   (unsigned int)s_nativeState.ztest, (unsigned int)s_nativeState.zwrite,
                   (unsigned int)s_nativeState.color_update, (unsigned int)s_nativeState.alpha_update,
                   s_nativeState.viewport[0], s_nativeState.viewport[1], s_nativeState.viewport[2], s_nativeState.viewport[3],
                   diagMilli(p[0]), diagMilli(p[1]), diagMilli(p[2]),
                   diagMilli(uv[0]), diagMilli(uv[1]),
                   (unsigned int)firstVertex[20], (unsigned int)firstVertex[21],
                   (unsigned int)firstVertex[22], (unsigned int)firstVertex[23],
                   (unsigned int)colorByte(s_nativeState.current_color[0]),
                   (unsigned int)colorByte(s_nativeState.current_color[1]),
                   (unsigned int)colorByte(s_nativeState.current_color[2]),
                   (unsigned int)colorByte(s_nativeState.current_color[3]),
                   diagMilli(cx * invW), diagMilli(cy * invW), diagMilli(cz * invW), diagMilli(cw));
            ++loggedDraws;
        }
    }
#endif

    const unsigned char* base = static_cast<const unsigned char*>(mesh.vertices);
    int done = 0;
    while (done < mesh.count)
    {
        int batch = mesh.count - done;
        if (batch > 65535)
            batch = 65535;
        if (primitiveVertices > 0 && batch < mesh.count - done)
            batch -= batch % primitiveVertices;
        if (batch <= 0)
            break;

        const bool closeThisBatch = closePrimitive && done == 0 && batch == mesh.count;
        GX_Begin(gxMode, GX_VTXFMT0, (u16)(batch + (closeThisBatch ? 1 : 0)));
        for (int i = 0; i < batch + (closeThisBatch ? 1 : 0); ++i)
        {
            const int sourceIndex = (i == batch) ? 0 : i;
            const unsigned char* v = base +
                (size_t)(mesh.first + done + sourceIndex) * (size_t)mesh.vertexStride;
            const unsigned char* colorSource = nullptr;
            if (s_nativeState.shade_model == wiiRenderValue(WiiRenderShadeModel::Flat) &&
                primitiveVertices > 0 && sourceIndex < batch)
            {
                const int primitiveStart = (sourceIndex / primitiveVertices) * primitiveVertices;
                const int colorIndex = primitiveStart + primitiveVertices - 1;
                colorSource = base +
                    (size_t)(mesh.first + done + colorIndex) * (size_t)mesh.vertexStride;
            }
            emitImmediateMeshVertex(mesh, v, s_nativeState, colorSource);
        }
        GX_End();
        done += batch;
    }

    if (done != mesh.count)
    {
        noteDroppedSubmission("mesh-batch");
        return false;
    }

    wii_gx_invalidate_after_native_draw();
    s_nativeStateValid = false;
    return true;
}

int wii_native_create_static_list_handle()
{
    return s_nextStaticHandle++;
}

void wii_native_destroy_static_list_handle(int handle)
{
    auto it = s_staticLists.find(handle);
    if (it == s_staticLists.end())
        return;
    freeStaticList(it->second);
    s_staticLists.erase(it);
}

bool wii_native_compile_static_list(int handle, const void* vertices, int vertexStride,
                                    bool texCoordEnabled, bool colorEnabled, bool hasNormals,
                                    bool brightnessEnabled, int brightnessOffset,
                                    WiiRenderPrimitive primitive, int first, int count)
{
    if (handle <= 0 || !vertices || vertexStride != 32 || first < 0 || count <= 0)
        return false;

    WiiNativeMeshView mesh;
    mesh.vertices = vertices;
    mesh.vertexStride = vertexStride;
    mesh.texCoordEnabled = texCoordEnabled;
    mesh.colorEnabled = colorEnabled;
    mesh.hasNormals = hasNormals;
    mesh.brightnessEnabled = brightnessEnabled;
    mesh.brightnessOffset = brightnessOffset;
    mesh.primitive = primitive;
    mesh.first = first;
    mesh.count = count;

    u8 gxMode;
    int primitiveVertices;
    bool continuousPrimitive;
    bool closePrimitive;
    if (!getGxPrimitive(primitive, gxMode, primitiveVertices, continuousPrimitive, closePrimitive))
        return false;

    const unsigned int payload = (unsigned int)count *
        bytesPerVertex(texCoordEnabled, colorEnabled, hasNormals, brightnessEnabled);
    const unsigned int capacity = align32(payload + 512u +
        ((unsigned int)count / 65535u + 1u) * 32u);
    void* data = memalign(32, capacity);
    if (!data)
        return false;

    unsigned int listSize = 0;
    if (!emitDisplayList(mesh, data, capacity, listSize))
    {
        free(data);
        return false;
    }
    // The display-list bytes were produced by GX/write-gather, not by the CPU.
    // The destination cache range was invalidated before recording in
    // emitDisplayList(), so no post-record CPU flush is required (or safe).

    WiiNativeStaticList replacement;
    replacement.data = data;
    replacement.size = listSize;
    replacement.capacity = capacity;
    replacement.primitive = primitive;
    replacement.vertexCount = count;
    replacement.texCoordEnabled = texCoordEnabled;
    replacement.colorEnabled = colorEnabled;
    replacement.hasNormals = hasNormals;
    replacement.brightnessEnabled = brightnessEnabled;
    replacement.brightnessOffset = brightnessOffset;

    WiiNativeStaticList& destination = s_staticLists[handle];
    freeStaticList(destination);
    destination = replacement;
#if MC_LOG_LEVEL >= 2
    {
        static unsigned int loggedStaticCompiles = 0;
        if (loggedStaticCompiles < 8)
        {
            MC_LOG_DEBUG("wii.render", "[WII][GX][DL][STATIC-COMPILE] handle=%d verts=%d size=%u cap=%u tex=%d color=%d normal=%d\n",
                   handle, count, listSize, capacity, texCoordEnabled ? 1 : 0,
                   colorEnabled ? 1 : 0, hasNormals ? 1 : 0);
            ++loggedStaticCompiles;
        }
    }
#endif
    return true;
}

bool wii_native_call_static_list(int handle)
{
    auto it = s_staticLists.find(handle);
    if (it == s_staticLists.end())
    {
        noteDroppedSubmission("static-no-list");
        return false;
    }
    const WiiNativeStaticList& list = it->second;
    if (!list.data || list.size == 0 || list.vertexCount <= 0)
    {
        noteDroppedSubmission("static-empty");
        return false;
    }

    WiiNativeMeshView mesh;
    mesh.vertexStride = 32;
    mesh.texCoordEnabled = list.texCoordEnabled;
    mesh.colorEnabled = list.colorEnabled;
    mesh.hasNormals = list.hasNormals;
    mesh.brightnessEnabled = list.brightnessEnabled;
    mesh.brightnessOffset = list.brightnessOffset;
    mesh.primitive = list.primitive;
    mesh.first = 0;
    mesh.count = list.vertexCount;
    if (!prepareNativeMeshState(mesh, false))
        return false;

    GX_CallDispList(list.data, list.size);
    wii_gx_invalidate_after_native_draw();
    s_nativeStateValid = false;
    return true;
}

bool wii_native_compile_chunk_list(int handle, const void* vertices, int vertexStride,
                                   bool texCoordEnabled, bool colorEnabled,
                                   bool brightnessEnabled, int brightnessOffset,
                                   WiiRenderPrimitive primitive, int first, int count,
                                   float translateX, float translateY, float translateZ,
                                   float sectionSize,
                                   const WiiNativeChunkFaceGroups* faceGroups)
{
    if (handle <= 0 || !vertices || vertexStride != 32 || first < 0 || count <= 0)
        return false;

    WiiNativeMeshView mesh;
    mesh.vertices = vertices;
    mesh.vertexStride = vertexStride;
    mesh.texCoordEnabled = texCoordEnabled;
    mesh.colorEnabled = colorEnabled;
    mesh.hasNormals = false;
    mesh.brightnessEnabled = brightnessEnabled;
    mesh.brightnessOffset = brightnessOffset;
    mesh.primitive = primitive;
    mesh.first = first;
    mesh.count = count;

    const bool compactTerrainFormat = true;

    /* A GX display list stores the primitive command plus the direct vertex
     * payload. Add generous command/alignment slop so GX_EndDispList never has
     * to report an overflow for a normal chunk section. */
    const unsigned int payload = (unsigned int)count *
        bytesPerTerrainVertex(texCoordEnabled, colorEnabled, brightnessEnabled, compactTerrainFormat);
    // A split section pays one more GX_Begin and up to 32 bytes of realignment
    // per direction; reserve that on top of the existing slop rather than
    // letting a dense section fail to record.
    const unsigned int splitSlop = faceGroups != nullptr
        ? (unsigned int)WiiNativeChunkFaceGroups::kGroupCount * 64u : 0u;
    const unsigned int requiredCapacity = align32(payload + 1024u + splitSlop +
        ((unsigned int)count / 65535u + 1u) * 32u);
    unsigned int capacity = 0;
    void* data = acquireChunkBuffer(requiredCapacity, capacity);
    if (!data)
        return false;

    unsigned int listSize = 0;
    unsigned int groupOffset[WiiNativeChunkFaceGroups::kGroupCount] = {};
    unsigned int groupSize[WiiNativeChunkFaceGroups::kGroupCount] = {};
    int groupCount = 0;
    if (faceGroups != nullptr &&
        emitChunkDisplayLists(mesh, data, capacity, compactTerrainFormat, *faceGroups,
                              groupOffset, groupSize, listSize))
    {
        groupCount = WiiNativeChunkFaceGroups::kGroupCount;
    }
    else if (!emitDisplayList(mesh, data, capacity, listSize, compactTerrainFormat))
    {
        releaseChunkBuffer(data, capacity);
        return false;
    }

    // The display-list bytes were produced by GX/write-gather, not by the CPU.
    // The destination cache range was invalidated before recording in
    // emitDisplayList(), so no post-record CPU flush is required (or safe).

    WiiNativeChunkList replacement;
    replacement.data = data;
    replacement.size = listSize;
    replacement.capacity = capacity;
    replacement.primitive = primitive;
    replacement.vertexCount = count;
    replacement.texCoordEnabled = texCoordEnabled;
    replacement.colorEnabled = colorEnabled;
    replacement.brightnessEnabled = brightnessEnabled;
    replacement.brightnessOffset = brightnessOffset;
    replacement.compactTerrainFormat = compactTerrainFormat;
    replacement.translateX = translateX;
    replacement.translateY = translateY;
    replacement.translateZ = translateZ;
    replacement.sectionSize = sectionSize;
    replacement.groupCount = groupCount;
    if (groupCount > 0)
    {
        for (int g = 0; g < WiiNativeChunkFaceGroups::kGroupCount; ++g)
        {
            replacement.groupOffset[g] = groupOffset[g];
            replacement.groupSize[g] = groupSize[g];
            replacement.groupPlaneMin[g] = faceGroups->planeMin[g];
            replacement.groupPlaneMax[g] = faceGroups->planeMax[g];
        }
        replacement.worldOrigin[0] = faceGroups->worldOriginX;
        replacement.worldOrigin[1] = faceGroups->worldOriginY;
        replacement.worldOrigin[2] = faceGroups->worldOriginZ;
    }
#if MC_LOG_LEVEL >= 2
    {
        const unsigned char* base = static_cast<const unsigned char*>(vertices) + (size_t)first * (size_t)vertexStride;
        const float* firstPos = reinterpret_cast<const float*>(base);
        replacement.firstPosition[0] = firstPos[0];
        replacement.firstPosition[1] = firstPos[1];
        replacement.firstPosition[2] = firstPos[2];
        replacement.localMin[0] = replacement.localMax[0] = firstPos[0];
        replacement.localMin[1] = replacement.localMax[1] = firstPos[1];
        replacement.localMin[2] = replacement.localMax[2] = firstPos[2];
        if (texCoordEnabled)
        {
            const float* uv = reinterpret_cast<const float*>(base + 12);
            replacement.firstUv[0] = uv[0];
            replacement.firstUv[1] = uv[1];
        }
        if (colorEnabled)
            std::memcpy(replacement.firstColor, base + 20, sizeof(replacement.firstColor));
        for (int i = 1; i < count; ++i)
        {
            const float* pos = reinterpret_cast<const float*>(base + (size_t)i * (size_t)vertexStride);
            for (int axis = 0; axis < 3; ++axis)
            {
                if (pos[axis] < replacement.localMin[axis]) replacement.localMin[axis] = pos[axis];
                if (pos[axis] > replacement.localMax[axis]) replacement.localMax[axis] = pos[axis];
            }
        }
    }
#endif

    WiiNativeChunkList& destination = s_chunkLists[handle];
    freeChunkList(destination);
    destination = replacement;
#if MC_LOG_LEVEL >= 2
    {
        static unsigned int loggedChunkCompiles = 0;
        if (loggedChunkCompiles < 16)
        {
            MC_LOG_DEBUG("wii.render", "[WII][GX][DL][CHUNK-COMPILE] handle=%d verts=%d size=%u cap=%u tex=%d color=%d compact=%d pos=(%d,%d,%d)\n",
                   handle, count, listSize, capacity, texCoordEnabled ? 1 : 0,
                   colorEnabled ? 1 : 0, compactTerrainFormat ? 1 : 0, diagMilli(translateX), diagMilli(translateY),
                   diagMilli(translateZ));
            ++loggedChunkCompiles;
        }
    }
#endif
    return true;
}

bool wii_native_call_chunk_list(int handle)
{
    auto it = s_chunkLists.find(handle);
    if (it == s_chunkLists.end())
        return false;

    WiiNativeChunkList& list = it->second;
    if (!list.data || list.size == 0 || list.vertexCount <= 0)
        return false;

    if (!prepareNativeTerrainBatch(list.texCoordEnabled, list.colorEnabled,
                                      list.brightnessEnabled, list.compactTerrainFormat))
        return false;
    loadChunkTransform(list.translateX, list.translateY, list.translateZ, list.sectionSize);
    callChunkGeometry(list);
    wii_gx_invalidate_after_native_draw();
    s_nativeStateValid = false;
    return true;
}


void wii_native_capture_terrain_camera()
{
    // Camera/projection state is captured lazily from the live GL state at
    // native batch begin, after RenderList has applied its translation.
}

void wii_native_set_terrain_render_list_offset(float x, float y, float z)
{
    (void)x; (void)y; (void)z;
}

void wii_native_set_terrain_eye(float x, float y, float z)
{
    s_terrainEyeWorld[0] = x;
    s_terrainEyeWorld[1] = y;
    s_terrainEyeWorld[2] = z;
    s_terrainEyeValid = true;
}

void wii_native_face_cull_stats(unsigned int *drawn, unsigned int *culled)
{
    if (drawn) *drawn = s_faceGroupsDrawn;
    if (culled) *culled = s_faceGroupsCulled;
}

void wii_native_set_terrain_fog(int mode, float density, float start, float end,
                                float r, float g, float b, float a)
{
    s_nativeFog.valid = true;
    s_nativeFog.mode = mode;
    s_nativeFog.density = density;
    s_nativeFog.start = start;
    s_nativeFog.end = end;
    s_nativeFog.color[0] = r;
    s_nativeFog.color[1] = g;
    s_nativeFog.color[2] = b;
    s_nativeFog.color[3] = a;
}

bool wii_native_begin_terrain_pass(WiiNativeTerrainPass pass)
{
    if (s_terrainPassActive || s_chunkBatchActive)
        return false;
    if (pass != WII_NATIVE_TERRAIN_OPAQUE &&
        pass != WII_NATIVE_TERRAIN_TRANSLUCENT_DEPTH &&
        pass != WII_NATIVE_TERRAIN_TRANSLUCENT_COLOR)
        return false;

    WiiNativeRenderStateSnapshot state;
    if (!snapshotSupportedTerrainState(state))
        return false;

    s_terrainPassActive = true;
    s_terrainPassMode = pass;
    s_terrainEarlyDepth = false;
    s_terrainPassEarlyDepth = false;
    s_terrainPassPrepared = false;
    s_terrainPassTexCoord = false;
    s_terrainPassColor = false;
    s_terrainPassBrightness = false;
    s_terrainPassCompactFormat = false;
    s_terrainPassPrimitive = WiiRenderPrimitive::Quads;
    return true;
}

void wii_native_end_terrain_pass()
{
    if (!s_terrainPassActive)
        return;

    if (s_chunkBatchActive)
        wii_native_end_chunk_batch();

    if (s_terrainPassPrepared)
    {
        wii_gx_invalidate_after_native_draw();
        // applyTerrainPassState() latched this pass' own blend, cull and
        // colour-write state into GX. The depth-only translucent pass turns
        // colour writes off, and the colour pass that would turn them back on
        // is skipped when nothing translucent is visible, so hand GX back the
        // tracked GL state here instead of relying on the next draw to do it.
        wii_gx_native_restore_pixel_engine();
    }
    s_nativeStateValid = false;

    s_terrainPassActive = false;
    s_terrainPassMode = WII_NATIVE_TERRAIN_OPAQUE;
    s_terrainEarlyDepth = false;
    s_terrainPassEarlyDepth = false;
    s_terrainPassPrepared = false;
    s_terrainPassTexCoord = false;
    s_terrainPassColor = false;
    s_terrainPassBrightness = false;
    s_terrainPassCompactFormat = false;
    s_terrainPassPrimitive = WiiRenderPrimitive::Quads;
}

void wii_native_set_terrain_early_depth(bool enabled)
{
    // Nothing is applied here. The flag is part of the prepared-state signature,
    // so the next wii_native_begin_chunk_batch() sees the mismatch and
    // reprograms GX; changing it mid-batch would leave the lists already
    // submitted to that batch drawn with the wrong compare location.
    s_terrainEarlyDepth = enabled;
}

void wii_native_invalidate_terrain_pass()
{
    if (!s_terrainPassActive || !s_terrainPassPrepared)
        return;

    if (s_chunkBatchActive)
        wii_native_end_chunk_batch();

    wii_gx_invalidate_after_native_draw();
    wii_gx_native_restore_pixel_engine();
    s_nativeStateValid = false;
    s_terrainPassEarlyDepth = false;
    s_terrainPassPrepared = false;
    s_terrainPassTexCoord = false;
    s_terrainPassColor = false;
    s_terrainPassBrightness = false;
    s_terrainPassCompactFormat = false;
    s_terrainPassPrimitive = WiiRenderPrimitive::Quads;
}

bool wii_native_begin_chunk_batch(int firstHandle)
{
    if (s_chunkBatchActive)
        return false;

    auto it = s_chunkLists.find(firstHandle);
    if (it == s_chunkLists.end())
    {
        noteDroppedSubmission("chunk-no-list");
        return false;
    }

    WiiNativeChunkList& list = it->second;
    if (!list.data || list.size == 0 || list.vertexCount <= 0)
    {
        noteDroppedSubmission("chunk-empty");
        return false;
    }

    if (!s_terrainPassActive || !terrainPassSignatureMatches(list))
    {
        if (!prepareNativeTerrainBatch(list.texCoordEnabled, list.colorEnabled,
                                      list.brightnessEnabled, list.compactTerrainFormat))
        {
            noteDroppedSubmission("chunk-prepare");
            return false;
        }

        if (s_terrainPassActive)
            recordTerrainPassSignature(list);
    }

    s_chunkBatchActive = true;
    recordChunkBatchSignature(list);
#if MC_LOG_LEVEL >= 2
    {
        static unsigned int loggedBatchBegins = 0;
        if (loggedBatchBegins < 16)
        {
            MC_LOG_DEBUG("wii.render", "[WII][GX][DL][BATCH-BEGIN] handle=%d verts=%d size=%u tex=%d color=%d terrainPass=%d\n",
                   firstHandle, list.vertexCount, list.size, list.texCoordEnabled ? 1 : 0,
                   list.colorEnabled ? 1 : 0, s_terrainPassActive ? 1 : 0);
            ++loggedBatchBegins;
        }
    }
#endif
    return true;
}

bool wii_native_append_chunk_list(int handle)
{
    auto it = s_chunkLists.find(handle);
    if (it == s_chunkLists.end())
    {
        noteDroppedSubmission("append-no-list");
        return false;
    }

    WiiNativeChunkList& list = it->second;
    if (!list.data || list.size == 0 || list.vertexCount <= 0)
    {
        noteDroppedSubmission("append-empty");
        return false;
    }

    if (!chunkBatchSignatureMatches(list))
    {
        if (!prepareNativeTerrainBatch(list.texCoordEnabled, list.colorEnabled,
                                      list.brightnessEnabled, list.compactTerrainFormat))
        {
            noteDroppedSubmission("append-prepare");
            return false;
        }

        if (s_terrainPassActive)
            recordTerrainPassSignature(list);
        recordChunkBatchSignature(list);
    }

    loadChunkTransform(list.translateX, list.translateY, list.translateZ, list.sectionSize);
#if MC_LOG_LEVEL >= 2
    logChunkTransform(list, handle);
#endif
    callChunkGeometry(list);
    return true;
}

void wii_native_end_chunk_batch()
{
    if (!s_chunkBatchActive)
        return;

    if (!s_terrainPassActive)
    {
        wii_gx_invalidate_after_native_draw();
        s_nativeStateValid = false;
    }
    s_chunkBatchActive = false;
    s_chunkBatchTexCoord = false;
    s_chunkBatchColor = false;
    s_chunkBatchBrightness = false;
    s_chunkBatchCompactFormat = false;
    s_chunkBatchPrimitive = WiiRenderPrimitive::Quads;
}

void wii_native_clear_chunk_list(int handle)
{
    auto it = s_chunkLists.find(handle);
    if (it == s_chunkLists.end())
        return;
    freeChunkList(it->second);
    s_chunkLists.erase(it);
}

void wii_native_swap_chunk_lists(int firstHandle, int secondHandle)
{
    WiiNativeChunkList first;
    WiiNativeChunkList second;

    auto firstIt = s_chunkLists.find(firstHandle);
    if (firstIt != s_chunkLists.end())
        first = firstIt->second;
    auto secondIt = s_chunkLists.find(secondHandle);
    if (secondIt != s_chunkLists.end())
        second = secondIt->second;

    if (second.data)
        s_chunkLists[firstHandle] = second;
    else
        s_chunkLists.erase(firstHandle);

    if (first.data)
        s_chunkLists[secondHandle] = first;
    else
        s_chunkLists.erase(secondHandle);
}

unsigned int wii_native_chunk_list_memory()
{
    // Total resident: live sections plus the idle capacity the recycler is
    // holding. This is the figure the heap report wants, because both halves
    // are memory the allocator cannot hand to anyone else. Cache policy wants
    // the live half on its own -- wii_native_chunk_list_live_bytes().
    return wii_native_chunk_list_live_bytes() + s_chunkBufferPoolBytes;
}

unsigned int wii_native_chunk_list_live_bytes()
{
    unsigned int bytes = 0;
    for (const auto& entry : s_chunkLists)
        bytes += entry.second.capacity;
    return bytes;
}

unsigned int wii_native_chunk_list_count()
{
    unsigned int count = 0;
    for (const auto& entry : s_chunkLists)
        if (entry.second.data != nullptr && entry.second.size != 0)
            ++count;
    return count;
}

void wii_native_chunk_buffer_pool_stats(unsigned int *hits, unsigned int *misses,
                                        unsigned int *overflows, unsigned int *pooledBytes)
{
    if (hits)
        *hits = s_chunkBufferPoolHits;
    if (misses)
        *misses = s_chunkBufferPoolMisses;
    if (overflows)
        *overflows = s_chunkBufferPoolOverflows;
    // Idle capacity held for reuse. wii_native_chunk_list_memory() counts this
    // alongside the live sections, so subtracting it is what separates "the
    // renderer is holding too much" from "the recycler is holding too much".
    if (pooledBytes)
        *pooledBytes = s_chunkBufferPoolBytes;
}
