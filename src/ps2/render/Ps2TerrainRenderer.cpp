#include "ps2/render/Ps2TerrainRenderer.h"
#include "ps2/render/Ps2RenderBackend.h"
#include "ps2/render/Ps2Vu1Terrain.h"

#ifdef PS2_PLATFORM

#include <algorithm>
#include <cstdint>
#include <cstring>

#include "platform/Log.h"
#include "ps2/render/Ps2CaptureLayout.h"
#include "platform/PlatformTuning.h"
#include "ps2/render/Ps2ClipGuard.h"

namespace
{
    static const int_t kMaxTerrainBatchVertices = 3072;
    static const float kClusterPlaneEpsilon = 1e-4f;
    static Ps2TerrainClusterStats s_clusterStats = {};
    static Ps2TerrainPass s_currentPass = PS2_TERRAIN_PASS_OPAQUE;

	enum Ps2OpaqueSubmitPhase
	{
		PS2_OPAQUE_SUBMIT_IMMEDIATE,
		PS2_OPAQUE_SUBMIT_QUEUE,
		PS2_OPAQUE_SUBMIT_VU1,
		PS2_OPAQUE_SUBMIT_VU0
	};

	// Per-cluster visibility and guard-band risk for one queued section.
	//
	// ps2_terrain_end() runs every queued section through ps2_terrain_draw_section
	// twice -- once to submit direct VU1 ranges while Path1 is held, once to run
	// the clipping-capable VU0 work on Path3. Both passes see the same MVP and
	// the same cluster bounds, so the classification below is bit-for-bit
	// identical between them: 64 clusters against 6 frustum planes plus 64
	// against 5 guard-band planes, recomputed for nothing. At the full-section
	// replay cap that was ~45000 redundant plane/AABB tests per frame on a 294MHz EE.
	//
	// Bytes, not ints: the values are 0..2 and 0..3, and this array is resident
	// for the whole session.
	struct Ps2ClusterClassification
	{
		unsigned char visibility[PS2_MESH_CLUSTER_COUNT];
		unsigned char guardRisk[PS2_MESH_CLUSTER_COUNT];
		bool valid;
	};

	struct Ps2QueuedTerrainSection
	{
		Ps2RendererFrame frame;
		Ps2TerrainSectionView section;
		Ps2TerrainFallbackDraw fallback;
		bool forceVu0All;
		Ps2ClusterClassification classification;
	};

	static const int kMaxQueuedTerrainSections =
		(int)PLATFORM_MAX_RENDERED_SECTIONS_PER_PASS * 2;
	static Ps2QueuedTerrainSection s_queuedTerrain[kMaxQueuedTerrainSections];
	static int s_queuedTerrainCount = 0;
	static Ps2OpaqueSubmitPhase s_opaqueSubmitPhase = PS2_OPAQUE_SUBMIT_IMMEDIATE;
	static bool s_currentVu1Retry = false;
	static bool s_forceVu0All = false;
	// Classification slot of the section currently being drawn, or null for an
	// immediate (translucent / unqueued) draw. Same per-call channel the two
	// flags above use.
	static Ps2ClusterClassification* s_classification = nullptr;

	// Index of the section the opaque replay is currently submitting, -1 while
	// no replay is running. Written unconditionally because it is two stores;
	// read only by ps2_dbg_terrain_section() when a VIF1 transfer times out, so
	// the stall dump can name the section whose packet wedged VU1.
	static int s_traceSection = -1;

#ifdef PS2_RENDER_STATS
#define PS2_TERRAIN_CLUSTER_STAT(expr) do { expr; } while (0)
#else
#define PS2_TERRAIN_CLUSTER_STAT(expr) do { } while (0)
#endif

    struct Ps2ClusterFrustum
    {
        float plane[6][4];
    };

    struct Ps2ClusterGuardBand
    {
        float plane[5][4];
    };

    enum Ps2ClusterGuardRisk
    {
        PS2_CLUSTER_GUARD_SAFE = 0,
        PS2_CLUSTER_GUARD_NEAR = 1 << 0,
        PS2_CLUSTER_GUARD_SIDE = 1 << 1
    };

    static void setPlane(float* out, const float* mvp, int row, float sign)
    {
        out[0] = mvp[3]  + sign * mvp[row];
        out[1] = mvp[7]  + sign * mvp[4 + row];
        out[2] = mvp[11] + sign * mvp[8 + row];
        out[3] = mvp[15] + sign * mvp[12 + row];
    }

    static void buildClusterFrustum(Ps2ClusterFrustum& out, const float* mvp)
    {
        // OpenGL clip space: -w <= x/y/z <= w. The MVP is column-major, so
        // every plane is row(w) +/- row(component).
        setPlane(out.plane[0], mvp, 0,  1.0f);
        setPlane(out.plane[1], mvp, 0, -1.0f);
        setPlane(out.plane[2], mvp, 1,  1.0f);
        setPlane(out.plane[3], mvp, 1, -1.0f);
        setPlane(out.plane[4], mvp, 2,  1.0f);
        setPlane(out.plane[5], mvp, 2, -1.0f);
    }

    static void setGuardPlane(float* out, const float* mvp,
                              int row, float sign, float wScale)
    {
        out[0] = wScale * mvp[3]  + sign * mvp[row];
        out[1] = wScale * mvp[7]  + sign * mvp[4 + row];
        out[2] = wScale * mvp[11] + sign * mvp[8 + row];
        out[3] = wScale * mvp[15] + sign * mvp[12 + row];
    }

    static void buildClusterGuardBand(Ps2ClusterGuardBand& out,
                                      const float* mvp,
                                      float viewW, float viewH)
    {
        const float gx = ps2_guard_clip_scale(viewW);
        const float gy = ps2_guard_clip_scale(viewH);

        // Direct VU1 terrain does not clip. Match the five planes used by the
        // native VU0 clipper: w >= epsilon and the GS-safe XY guard band.
        out.plane[0][0] = mvp[3];
        out.plane[0][1] = mvp[7];
        out.plane[0][2] = mvp[11];
        out.plane[0][3] = mvp[15] - PS2_NEAR_CLIP_W;
        setGuardPlane(out.plane[1], mvp, 0,  1.0f, gx);
        setGuardPlane(out.plane[2], mvp, 0, -1.0f, gx);
        setGuardPlane(out.plane[3], mvp, 1,  1.0f, gy);
        setGuardPlane(out.plane[4], mvp, 1, -1.0f, gy);
    }

    static void clusterPlaneDistance(const Ps2MeshCluster& cluster,
                                     const float* plane,
                                     float& distance, float& radius)
    {
        const float centerX = (cluster.minX + cluster.maxX) * 0.5f;
        const float centerY = (cluster.minY + cluster.maxY) * 0.5f;
        const float centerZ = (cluster.minZ + cluster.maxZ) * 0.5f;
        const float extentX = (cluster.maxX - cluster.minX) * 0.5f;
        const float extentY = (cluster.maxY - cluster.minY) * 0.5f;
        const float extentZ = (cluster.maxZ - cluster.minZ) * 0.5f;

        distance = plane[0] * centerX + plane[1] * centerY +
            plane[2] * centerZ + plane[3];
        radius =
            (plane[0] < 0.0f ? -plane[0] : plane[0]) * extentX +
            (plane[1] < 0.0f ? -plane[1] : plane[1]) * extentY +
            (plane[2] < 0.0f ? -plane[2] : plane[2]) * extentZ;
    }

    static int classifyCluster(const Ps2MeshCluster& cluster,
                               const Ps2ClusterFrustum& frustum)
    {
        if (!cluster.valid())
            return 0;

        bool fullyInside = true;

        for (int i = 0; i < 6; ++i)
        {
            const float* plane = frustum.plane[i];
            float distance;
            float radius;
            clusterPlaneDistance(cluster, plane, distance, radius);

            // Bias both decisions toward the clipped VU0 path. Near-plane
            // precision must never turn an intersecting cluster into VU1 work.
            if (distance + radius < -kClusterPlaneEpsilon)
                return 0;
            if (distance - radius <= kClusterPlaneEpsilon)
                fullyInside = false;
        }

        return fullyInside ? 2 : 1;
    }

    static int classifyClusterGuardRisk(const Ps2MeshCluster& cluster,
                                        const Ps2ClusterGuardBand& guardBand)
    {
        if (!cluster.valid())
            return PS2_CLUSTER_GUARD_NEAR | PS2_CLUSTER_GUARD_SIDE;

        int risk = PS2_CLUSTER_GUARD_SAFE;
        for (int i = 0; i < 5; ++i)
        {
            float distance;
            float radius;
            clusterPlaneDistance(cluster, guardBand.plane[i], distance, radius);
            if (distance - radius <= kClusterPlaneEpsilon)
            {
                risk |= i == 0
                    ? PS2_CLUSTER_GUARD_NEAR
                    : PS2_CLUSTER_GUARD_SIDE;
            }
        }
        return risk;
    }

    static bool clusterCanSkipClip(int visibilityClass, int guardRisk)
    {
        if (visibilityClass == 0 || guardRisk != PS2_CLUSTER_GUARD_SAFE)
            return false;
#if PS2_VU1_GUARD_BAND_PARTIALS
        return true;
#else
        return visibilityClass == 2;
#endif
    }

    static Ps2NativeTerrainPass nativePass(Ps2TerrainPass pass)
    {
        return pass == PS2_TERRAIN_PASS_TRANSLUCENT
            ? PS2_NATIVE_TERRAIN_TRANSLUCENT
            : PS2_NATIVE_TERRAIN_OPAQUE;
    }

    static int_t boundedBatchSize(int_t remaining, int_t drawMode)
    {
        int_t count = std::min(remaining, kMaxTerrainBatchVertices);
        if ((drawMode == PS2_NATIVE_PRIM_QUADS && (PLATFORM_TESSELLATOR_CONVERT_QUADS != 0)) ||
            drawMode == PS2_NATIVE_PRIM_TRIANGLES)
        {
            count -= count % 3;
        }
        else if (drawMode == PS2_NATIVE_PRIM_QUADS)
        {
            count -= count % 4;
        }
        return count;
    }

    static Ps2NativeMeshView makeMesh(const Ps2TerrainSectionView& section,
                                      const int_t* raw,
                                      int_t vertexCount)
    {
        Ps2NativeMeshView mesh;
		mesh.packedTerrain = false;
        mesh.clampRuns = nullptr;
        mesh.clampRunCount = 0;
        mesh.vertices = raw;
        mesh.vertexStride = Ps2CaptureLayout::Stride;
        mesh.vertexSize = 3;
        mesh.texCoords = section.hasTexture
            ? reinterpret_cast<const float*>(raw) + 3
            : nullptr;
        mesh.texCoordStride = Ps2CaptureLayout::Stride;
        mesh.texCoordEnabled = section.hasTexture;
        mesh.colors = section.hasColor
            ? reinterpret_cast<const uint8_t*>(raw) + Ps2CaptureLayout::ColorOffset
            : nullptr;
        mesh.colorStride = Ps2CaptureLayout::Stride;
        mesh.colorSize = 4;
        mesh.colorEnabled = section.hasColor;
        mesh.colorFloat = false;
        mesh.normals = nullptr;
        mesh.normalStride = 0;
        mesh.normalFloat = false;
        mesh.hasNormals = section.hasNormals;
        mesh.drawMode = (section.drawMode == PS2_NATIVE_PRIM_QUADS && (PLATFORM_TESSELLATOR_CONVERT_QUADS != 0))
            ? PS2_NATIVE_PRIM_TRIANGLES
            : section.drawMode;
        mesh.first = 0;
        mesh.count = vertexCount;
        mesh.slices = nullptr;
        mesh.sliceCount = 0;
        return mesh;
    }

	static Ps2NativeMeshView makePackedMesh(const short* positions,
												  const short* texCoords,
												  const unsigned char* colors,
												  int_t firstVertex,
												  int_t vertexCount,
                                              const Ps2NativeClampRun* clampRuns,
                                              int clampRunCount,
                                              const Ps2NativeSlice* slices = nullptr,
                                              int sliceCount = 0)
	{
		Ps2NativeMeshView mesh;
		mesh.packedTerrain = true;
		mesh.clampRuns = clampRuns;
		mesh.clampRunCount = clampRunCount;
		mesh.vertices = positions;
		mesh.vertexStride = 4 * (int)sizeof(short);
		mesh.vertexSize = 3;
		mesh.texCoords = texCoords;
		mesh.texCoordStride = 2 * (int)sizeof(short);
		mesh.texCoordEnabled = true;
		mesh.colors = colors;
		mesh.colorStride = 4;
		mesh.colorSize = 4;
		mesh.colorEnabled = true;
		mesh.colorFloat = false;
		mesh.normals = nullptr;
		mesh.normalStride = 0;
		mesh.normalFloat = false;
		mesh.hasNormals = false;
		mesh.drawMode = PS2_NATIVE_PRIM_QUADS;
		mesh.first = firstVertex;
		mesh.count = vertexCount;
		mesh.slices = slices;
		mesh.sliceCount = sliceCount;
		return mesh;
	}
}

void ps2_terrain_begin(unsigned int textureId, Ps2TerrainPass pass)
{
    // A previous pass should already have released Path1.  Keeping this guard
    // makes terrain setup robust against an early-returning caller and ensures
    // the texture/state work below always starts on Path3.
    ps2_render_release_path1();
    s_currentPass = pass;
    ps2_native_begin_terrain_pass(textureId, nativePass(pass));
    ps2_vu1_terrain_begin_pass();
	s_queuedTerrainCount = 0;
	s_forceVu0All = false;
	s_currentVu1Retry = false;
	// Only ps2_terrain_end()'s two-phase replay has a slot to fill; an immediate
	// draw must not write into whichever section was replayed last.
	s_classification = nullptr;
	s_opaqueSubmitPhase = pass == PS2_TERRAIN_PASS_OPAQUE
		? PS2_OPAQUE_SUBMIT_QUEUE
		: PS2_OPAQUE_SUBMIT_IMMEDIATE;
}

void ps2_terrain_end(Ps2TerrainPass pass)
{
	if (pass == PS2_TERRAIN_PASS_OPAQUE && s_queuedTerrainCount > 0)
	{
		// Opaque depth-tested terrain has no ordering dependency. Submit every
		// direct VU1 range while Path1 is owned, release it once, then run all
		// clipping-capable VU0 work through Path3. This removes the per-section
		// Path1/Path3 ping-pong without deferring mutable build data past the pass.
		MC_LOG_TRACE("terrain", "[PS2] opaque replay: %d sections, VU1 phase\n",
			s_queuedTerrainCount);
		s_opaqueSubmitPhase = PS2_OPAQUE_SUBMIT_VU1;
		for (int i = 0; i < s_queuedTerrainCount; ++i)
		{
			// Two stores, no formatting and no flush: a log line per section
			// would be ~128 synced writes a frame, which on USB is slow enough
			// to change the timing being investigated. The VIF1 stall dump
			// reads this instead, so the cost is only paid when it matters.
			s_traceSection = i;
			s_currentVu1Retry = false;
			s_forceVu0All = false;
			// First pass classifies and fills the slot; second pass reads it.
			s_queuedTerrain[i].classification.valid = false;
			s_classification = &s_queuedTerrain[i].classification;
			(void)ps2_terrain_draw_section(s_queuedTerrain[i].frame,
				s_queuedTerrain[i].section, s_queuedTerrain[i].fallback);
			s_queuedTerrain[i].forceVu0All = s_currentVu1Retry;
		}

		ps2_render_release_path1();
		MC_LOG_TRACE("terrain", "[PS2] opaque replay: VU0 phase\n");
		s_opaqueSubmitPhase = PS2_OPAQUE_SUBMIT_VU0;
		for (int i = 0; i < s_queuedTerrainCount; ++i)
		{
			s_traceSection = i;
			s_forceVu0All = s_queuedTerrain[i].forceVu0All;
			s_classification = &s_queuedTerrain[i].classification;
			(void)ps2_terrain_draw_section(s_queuedTerrain[i].frame,
				s_queuedTerrain[i].section, s_queuedTerrain[i].fallback);
		}
		s_classification = nullptr;
		s_traceSection = -1;

		// The pass can report itself ready and still route everything to VU0:
		// without PS2_VU1_CLIPPED_PARTIALS only clusters proved fully inside the
		// frustum take the direct path, so a small render distance can leave the
		// count at zero. Sections vs vertices separates "VU1 is off" from "VU1 is
		// on and nothing qualified", which look identical while playing.
		static bool s_replayReported = false;
		if (!s_replayReported)
		{
			s_replayReported = true;
			MC_LOG_INFO("terrain",
				"[PS2] first opaque replay: %d sections, VU1 submitted %ld vertices\n",
				s_queuedTerrainCount, ps2_vu1_terrain_submitted_vertices());
		}
	}

	s_queuedTerrainCount = 0;
	s_opaqueSubmitPhase = PS2_OPAQUE_SUBMIT_IMMEDIATE;
	s_forceVu0All = false;
	s_currentVu1Retry = false;

#if PS2_VU1_TERRAIN_CANARY
    if (pass == PS2_TERRAIN_PASS_TRANSLUCENT)
        (void)ps2_vu1_terrain_draw_canary();
#endif

    // XGKICK (Path1) must be fully drained before entities/HUD resume gsKit
    // submissions on Path3.
    ps2_render_release_path1();
    ps2_native_end_terrain_pass(nativePass(pass));
}

Ps2TerrainDrawResult ps2_terrain_draw_section(const Ps2RendererFrame& frame,
                                              const Ps2TerrainSectionView& section,
                                              const Ps2TerrainFallbackDraw& fallback)
{
    Ps2TerrainDrawResult result = { 0, 0, true };
	if (s_currentPass == PS2_TERRAIN_PASS_OPAQUE &&
		s_opaqueSubmitPhase == PS2_OPAQUE_SUBMIT_QUEUE)
	{
		if (s_queuedTerrainCount >= kMaxQueuedTerrainSections)
		{
			result.complete = false;
			return result;
		}

		Ps2QueuedTerrainSection& queued = s_queuedTerrain[s_queuedTerrainCount++];
		queued.frame = frame;
		queued.section = section;
		queued.fallback = fallback;
		queued.forceVu0All = false;
		return result;
	}

	if (section.vertexCount <= 0)
        return result;

    const std::size_t slotsPerVertex = Ps2CaptureLayout::Slots;
	const bool packedOpaque = s_currentPass == PS2_TERRAIN_PASS_OPAQUE &&
		section.opaqueMesh != nullptr && section.opaqueMesh->valid() &&
		section.opaqueMesh->vertexCount() >= section.vertexCount;
	if (!packedOpaque && (section.raw == nullptr || section.rawIntCount == 0 ||
		(section.rawIntCount % slotsPerVertex) != 0u))
    {
        result.complete = false;
        return result;
    }

	const int_t totalVertices = packedOpaque
		? section.vertexCount
		: (int_t)std::min((std::size_t)section.vertexCount,
			section.rawIntCount / slotsPerVertex);
    if (totalVertices <= 0)
        return result;

    Ps2NativeDrawContext nativeContext;
    bool nativePathUsable = false;
    if (section.nativeEnabled)
    {
        nativePathUsable = ps2_renderer_prepare_translated_context(
            nativeContext, frame,
            section.translateX, section.translateY, section.translateZ,
            section.fullyInside);
    }

	const bool directVu1Usable = PS2_DIRECT_VU1_TERRAIN && !s_forceVu0All &&
        s_currentPass == PS2_TERRAIN_PASS_OPAQUE && nativePathUsable &&
		packedOpaque && ps2_vu1_terrain_pass_ready() &&
        section.drawMode == PS2_NATIVE_PRIM_QUADS && section.hasTexture &&
        !section.hasNormals;

    bool abortDraw = false;
    auto emitVu0Raw = [&](const int_t* raw, int_t vertexCount, bool fullyInside)
    {
		if (s_opaqueSubmitPhase == PS2_OPAQUE_SUBMIT_VU1)
			return;
        if (abortDraw || raw == nullptr || vertexCount <= 0)
        {
            if (raw == nullptr || vertexCount < 0)
                result.complete = false;
            return;
        }

#if defined(PS2_ENABLE_VU1_TERRAIN)
        // Any native VU0/gsKit draw must first submit and drain queued Path1
        // work from preceding direct terrain ranges.
        ps2_render_release_path1();
#endif

        int_t emitted = 0;
        while (emitted < vertexCount)
        {
            const int_t batchVertices = boundedBatchSize(vertexCount - emitted, section.drawMode);
            if (batchVertices <= 0)
            {
                result.complete = false;
                abortDraw = true;
                return;
            }

            const int_t* batchRaw = raw + (std::size_t)emitted * slotsPerVertex;

            if (nativePathUsable)
            {
                const Ps2NativeMeshView mesh = makeMesh(section, batchRaw, batchVertices);
                Ps2NativeDrawContext rangeContext = nativeContext;
                rangeContext.fullyInside = fullyInside;
                if (ps2_renderer_draw_prepared(mesh, rangeContext))
                {
                    result.nativeVertices += batchVertices;
                    emitted += batchVertices;
                    continue;
                }

                nativePathUsable = false;
            }

            if (fallback.draw == nullptr)
            {
                result.complete = false;
                abortDraw = true;
                return;
            }

            fallback.draw(fallback.user, batchRaw, batchVertices,
                          section.drawMode,
                          section.hasTexture, section.hasColor, section.hasNormals);
            result.fallbackVertices += batchVertices;
            emitted += batchVertices;
        }
    };

	auto emitVu0Packed = [&](const short* positions,
								 const short* texCoords,
								 const unsigned char* colors,
								 int_t firstVertex,
								 int_t vertexCount,
								 bool fullyInside,
                                 const Ps2NativeClampRun* clampRuns,
                                 int clampRunCount)
	{
		if (s_opaqueSubmitPhase == PS2_OPAQUE_SUBMIT_VU1)
			return;
		if (abortDraw || positions == nullptr || texCoords == nullptr || colors == nullptr ||
			firstVertex < 0 || vertexCount <= 0)
		{
			result.complete = false;
			abortDraw = true;
			return;
		}

#if defined(PS2_ENABLE_VU1_TERRAIN)
		ps2_render_release_path1();
#endif

		int_t emitted = 0;
		while (emitted < vertexCount)
		{
			const int_t batchVertices = boundedBatchSize(
				vertexCount - emitted, PS2_NATIVE_PRIM_QUADS);
			if (batchVertices <= 0 || !nativePathUsable)
			{
				result.complete = false;
				abortDraw = true;
				return;
			}

			const Ps2NativeMeshView mesh = makePackedMesh(
				positions, texCoords, colors, firstVertex + emitted, batchVertices,
                clampRuns, clampRunCount);
			Ps2NativeDrawContext rangeContext = nativeContext;
			rangeContext.fullyInside = fullyInside;
			if (!ps2_renderer_draw_prepared(mesh, rangeContext))
			{
				nativePathUsable = false;
				result.complete = false;
				abortDraw = true;
				return;
			}

			result.nativeVertices += batchVertices;
			emitted += batchVertices;
		}
	};

	// Zero-copy counterpart to emitVu0Packed: draws several non-contiguous
	// slices of the SAME mesh arrays in one native draw call instead of one
	// call per contiguous range. positions/texCoords/colors and clampRuns are
	// the mesh's own, un-gathered arrays -- no EE-side memcpy, matching the
	// direct VU1 terrain path's DMA REF scatter-gather (Ps2Vu1TerrainPackets.cpp).
	auto emitVu0PackedSliced = [&](const short* positions,
	                                const short* texCoords,
	                                const unsigned char* colors,
	                                const Ps2NativeSlice* slices,
	                                int sliceCount,
	                                int_t totalVertexCount,
	                                bool fullyInside,
	                                const Ps2NativeClampRun* clampRuns,
	                                int clampRunCount)
	{
		if (s_opaqueSubmitPhase == PS2_OPAQUE_SUBMIT_VU1)
			return;
		if (abortDraw || positions == nullptr || texCoords == nullptr || colors == nullptr ||
			slices == nullptr || sliceCount <= 0 || totalVertexCount <= 0)
		{
			result.complete = false;
			abortDraw = true;
			return;
		}

#if defined(PS2_ENABLE_VU1_TERRAIN)
		ps2_render_release_path1();
#endif

		if (!nativePathUsable)
		{
			result.complete = false;
			abortDraw = true;
			return;
		}

		const Ps2NativeMeshView mesh = makePackedMesh(
			positions, texCoords, colors, 0, totalVertexCount,
			clampRuns, clampRunCount, slices, sliceCount);
		Ps2NativeDrawContext rangeContext = nativeContext;
		rangeContext.fullyInside = fullyInside;
		if (!ps2_renderer_draw_prepared(mesh, rangeContext))
		{
			nativePathUsable = false;
			result.complete = false;
			abortDraw = true;
			return;
		}

		result.nativeVertices += totalVertexCount;
	};

    auto emitRange = [&](int_t firstVertex, int_t vertexCount, bool fullyInside)
    {
        if (abortDraw || firstVertex < 0 || vertexCount <= 0 ||
            firstVertex + vertexCount > totalVertices)
        {
            if (firstVertex < 0 || vertexCount < 0 || firstVertex + vertexCount > totalVertices)
                result.complete = false;
            return;
        }

#if defined(PS2_ENABLE_VU1_TERRAIN)
		if (s_opaqueSubmitPhase != PS2_OPAQUE_SUBMIT_VU0 &&
			nativePathUsable && directVu1Usable &&
            (fullyInside || PS2_VU1_CLIPPED_PARTIALS) &&
            (section.faceGroups == nullptr || section.faceGroups->valid) &&
            section.drawMode == PS2_NATIVE_PRIM_QUADS && section.hasTexture &&
            !section.hasNormals)
        {
            const Ps2Vu1TerrainDrawResult vu1 = ps2_vu1_terrain_draw_range(
				*section.opaqueMesh, firstVertex, vertexCount, frame.native,
                section.translateX, section.translateY, section.translateZ,
                fullyInside);
            if (vu1.status == PS2_VU1_TERRAIN_SUBMITTED)
            {
                PS2_TERRAIN_CLUSTER_STAT(++s_clusterStats.vu1Ranges);
                PS2_TERRAIN_CLUSTER_STAT(s_clusterStats.vu1Vertices += vu1.vertices);
                result.nativeVertices += vu1.vertices;
                return;
            }
            if (vu1.status == PS2_VU1_TERRAIN_FATAL)
            {
                result.nativeVertices += vu1.vertices;
                result.complete = false;
				s_currentVu1Retry = true;
                abortDraw = true;
                return;
            }

            // RETRY_NATIVE means no geometry from this range was accepted by
            // VU1. Queued submission records a whole-section VU0 replay;
            // immediate submission releases Path1 before writing Path3.
			s_currentVu1Retry = true;
			if (s_opaqueSubmitPhase == PS2_OPAQUE_SUBMIT_VU1)
			{
				// The VU0 phase will replay this section in full. Stop feeding
				// Path1 as soon as the section has requested that fallback.
				abortDraw = true;
				return;
			}
			ps2_render_release_path1();
        }
#endif
		if (s_opaqueSubmitPhase == PS2_OPAQUE_SUBMIT_VU1)
			return;

		if (packedOpaque)
		{
			const std::vector<Ps2TerrainTileRun>& clampRuns = section.opaqueMesh->runs();
			emitVu0Packed(section.opaqueMesh->positions(),
				section.opaqueMesh->texCoords(), section.opaqueMesh->colors(),
				firstVertex, vertexCount, fullyInside,
                clampRuns.empty() ? nullptr : &clampRuns[0], (int)clampRuns.size());
		}
		else
		{
			const int_t* rangeRaw = section.raw +
				(std::size_t)firstVertex * slotsPerVertex;
			emitVu0Raw(rangeRaw, vertexCount, fullyInside);
		}
    };

    auto emitClippedProbe = [&](int_t firstVertex, int_t vertexCount)
    {
#if defined(PS2_ENABLE_VU1_TERRAIN) && \
    PS2_VU1_CLIPPED_PROBE_BATCHES_PER_FRAME > 0 && !PS2_VU1_CLIPPED_PARTIALS
        if (abortDraw || s_opaqueSubmitPhase != PS2_OPAQUE_SUBMIT_VU1 ||
            !nativePathUsable || !directVu1Usable ||
            !ps2_vu1_terrain_clipped_probe_available())
            return;

        // This is deliberately a shadow submission. The authoritative VU0
        // gather below still includes the complete range, so an empty or
        // malformed clipped result cannot remove visible terrain.
        (void)ps2_vu1_terrain_probe_clipped_range(
            *section.opaqueMesh, firstVertex, vertexCount, frame.native,
            section.translateX, section.translateY, section.translateZ);
#else
        (void)firstVertex;
        (void)vertexCount;
#endif
    };

    if (section.faceGroups != nullptr && section.faceGroups->valid &&
        !section.faceGroups->ranges.empty())
    {
        const float eye[3] = {
            section.eyeLocalX,
            section.eyeLocalY,
            section.eyeLocalZ
        };

        // Second pass over a queued section: the MVP and the cluster bounds are
        // unchanged, so both classifications below would reproduce exactly what
        // the first pass already stored. See Ps2ClusterClassification.
        const bool reuseClassification =
            s_classification != nullptr && s_classification->valid;

        int clusterClass[PS2_MESH_CLUSTER_COUNT];
        if (reuseClassification)
        {
            for (int cluster = 0; cluster < PS2_MESH_CLUSTER_COUNT; ++cluster)
                clusterClass[cluster] = (int)s_classification->visibility[cluster];
        }
        else if (section.fullyInside)
        {
            for (int cluster = 0; cluster < PS2_MESH_CLUSTER_COUNT; ++cluster)
                clusterClass[cluster] = section.faceGroups->clusters[cluster].valid() ? 2 : 0;
        }
        else if (nativePathUsable)
        {
            Ps2ClusterFrustum frustum;
            buildClusterFrustum(frustum, nativeContext.mvp);
            for (int cluster = 0; cluster < PS2_MESH_CLUSTER_COUNT; ++cluster)
            {
                clusterClass[cluster] = classifyCluster(
                    section.faceGroups->clusters[cluster], frustum);
            }
        }
        else
        {
            // Without a valid MVP, retain the old safe behavior: draw every
            // populated cluster through the clipping-capable fallback.
            for (int cluster = 0; cluster < PS2_MESH_CLUSTER_COUNT; ++cluster)
                clusterClass[cluster] = section.faceGroups->clusters[cluster].valid() ? 1 : 0;
        }

        int clusterGuardRisk[PS2_MESH_CLUSTER_COUNT];
        if (reuseClassification)
        {
            for (int cluster = 0; cluster < PS2_MESH_CLUSTER_COUNT; ++cluster)
                clusterGuardRisk[cluster] = (int)s_classification->guardRisk[cluster];
        }
        else if (nativePathUsable && frame.native.viewW > 0.0f && frame.native.viewH > 0.0f)
        {
            Ps2ClusterGuardBand guardBand;
            buildClusterGuardBand(guardBand, nativeContext.mvp,
                                  frame.native.viewW, frame.native.viewH);
            for (int cluster = 0; cluster < PS2_MESH_CLUSTER_COUNT; ++cluster)
            {
                clusterGuardRisk[cluster] = classifyClusterGuardRisk(
                    section.faceGroups->clusters[cluster], guardBand);
            }
        }
        else
        {
            for (int cluster = 0; cluster < PS2_MESH_CLUSTER_COUNT; ++cluster)
            {
                clusterGuardRisk[cluster] =
                    PS2_CLUSTER_GUARD_NEAR | PS2_CLUSTER_GUARD_SIDE;
            }
        }

        if (s_classification != nullptr && !reuseClassification)
        {
            for (int cluster = 0; cluster < PS2_MESH_CLUSTER_COUNT; ++cluster)
            {
                s_classification->visibility[cluster] = (unsigned char)clusterClass[cluster];
                s_classification->guardRisk[cluster] = (unsigned char)clusterGuardRisk[cluster];
            }
            s_classification->valid = true;
        }

        int_t clusterVertices[PS2_MESH_CLUSTER_COUNT] = {};
        const std::vector<Ps2MeshRange>& classifiedRanges = section.faceGroups->ranges;
        for (std::size_t i = 0; i < classifiedRanges.size(); ++i)
            clusterVertices[classifiedRanges[i].cluster()] += classifiedRanges[i].vertexCount();

		if (s_opaqueSubmitPhase != PS2_OPAQUE_SUBMIT_VU0)
		{
			PS2_TERRAIN_CLUSTER_STAT(++s_clusterStats.sections);
			for (int cluster = 0; cluster < PS2_MESH_CLUSTER_COUNT; ++cluster)
			{
				if (clusterVertices[cluster] <= 0)
					continue;
				if (clusterClass[cluster] == 2)
					PS2_TERRAIN_CLUSTER_STAT(++s_clusterStats.insideClusters);
				else if (clusterClass[cluster] == 1)
					PS2_TERRAIN_CLUSTER_STAT(++s_clusterStats.partialClusters);
				else
				{
					PS2_TERRAIN_CLUSTER_STAT(++s_clusterStats.outsideClusters);
					PS2_TERRAIN_CLUSTER_STAT(
						s_clusterStats.outsideVertices += clusterVertices[cluster]);
					continue;
				}

				const int risk = clusterGuardRisk[cluster];
				if (risk == PS2_CLUSTER_GUARD_SAFE && clusterClass[cluster] == 1)
				{
					PS2_TERRAIN_CLUSTER_STAT(++s_clusterStats.guardSafePartialClusters);
					PS2_TERRAIN_CLUSTER_STAT(
						s_clusterStats.guardSafePartialVertices += clusterVertices[cluster]);
				}
				if ((risk & PS2_CLUSTER_GUARD_NEAR) != 0)
					PS2_TERRAIN_CLUSTER_STAT(++s_clusterStats.guardNearRiskClusters);
				if ((risk & PS2_CLUSTER_GUARD_SIDE) != 0)
					PS2_TERRAIN_CLUSTER_STAT(++s_clusterStats.guardSideRiskClusters);
			}
		}

        auto faceVisible = [&](int group)
        {
            bool keep = true;
#if PLATFORM_FACE_BUCKET_CULL
            if (group != PS2_FACE_OTHER)
            {
                const int axis = group >> 1;
                const bool positive = (group & 1) == 0;
                const float margin = (float)PLATFORM_FACE_CULL_EYE_MARGIN;
                keep = positive
                    ? eye[axis] + margin > section.faceGroups->planeMin[group]
                    : eye[axis] - margin < section.faceGroups->planeMax[group];
            }
#else
            (void)group;
#endif
            return keep;
        };

        // Opaque geometry has no ordering requirement. Submit all direct VU1
        // ranges first, then gather the intersecting ranges into one tile-ordered
        // VU0 stream. This avoids both Path1/Path3 alternation and one tiny VU0
        // draw per cluster/face range.
        auto emitVu1Ranges = [&]()
        {
#if defined(PS2_ENABLE_VU1_TERRAIN) && !PS2_VU1_CLIPPED_PARTIALS
            // The packed mesh is tile-major. Face/cluster culling can punch small
            // holes inside one tile run; feeding every surviving fragment through
            // draw_range() made each hole become another MSCAL/XGKICK. Scatter the
            // surviving source slices into one VU input buffer with VIF UNPACKs and
            // kick once for up to 80 vertices. No EE vertex copy is required.
            if (packedOpaque && directVu1Usable && section.opaqueMesh != nullptr)
            {
                const std::vector<Ps2MeshRange>& ranges = section.faceGroups->ranges;
                const std::vector<Ps2TerrainTileRun>& tileRuns = section.opaqueMesh->runs();
                std::size_t rangeIndex = 0;

                for (std::size_t tr = 0; tr < tileRuns.size() && !abortDraw; ++tr)
                {
                    const Ps2TerrainTileRun& tile = tileRuns[tr];
                    const int_t tileBegin = (int_t)tile.firstVertex;
                    const int_t tileEnd = tileBegin + (int_t)tile.vertexCount;
                    while (rangeIndex < ranges.size() &&
                           ranges[rangeIndex].firstVertex + ranges[rangeIndex].vertexCount() <= tileBegin)
                        ++rangeIndex;

                    Ps2Vu1TerrainSlice slices[8];
                    int sliceCount = 0;
                    int batchVertices = 0;

                    auto flushSlices = [&]() -> bool
                    {
                        if (batchVertices <= 0)
                            return true;
                        const Ps2Vu1TerrainDrawResult vu1 = ps2_vu1_terrain_draw_slices(
                            *section.opaqueMesh, slices, sliceCount, batchVertices,
                            tile.tileX, tile.tileY, frame.native,
                            section.translateX, section.translateY, section.translateZ);
                        if (vu1.status == PS2_VU1_TERRAIN_SUBMITTED)
                        {
                            PS2_TERRAIN_CLUSTER_STAT(++s_clusterStats.vu1Ranges);
                            PS2_TERRAIN_CLUSTER_STAT(s_clusterStats.vu1Vertices += vu1.vertices);
                            result.nativeVertices += vu1.vertices;
                            sliceCount = 0;
                            batchVertices = 0;
                            return true;
                        }
                        s_currentVu1Retry = true;
                        if (vu1.status == PS2_VU1_TERRAIN_FATAL)
                        {
                            result.nativeVertices += vu1.vertices;
                            result.complete = false;
                            abortDraw = true;
                            return false;
                        }
                        if (s_opaqueSubmitPhase == PS2_OPAQUE_SUBMIT_VU1)
                        {
                            // Queued mode will replay the section through VU0.
                            abortDraw = true;
                            return false;
                        }
                        // Immediate mode cannot replay the whole section later.
                        // Preserve correctness by emitting just this compacted set
                        // through the packed VU0 path, then continue with later tiles.
                        ps2_render_release_path1();
                        for (int si = 0; si < sliceCount && !abortDraw; ++si)
                        {
                            const std::vector<Ps2TerrainTileRun>& clampRuns =
                                section.opaqueMesh->runs();
                            emitVu0Packed(section.opaqueMesh->positions(),
                                section.opaqueMesh->texCoords(), section.opaqueMesh->colors(),
                                slices[si].firstVertex, slices[si].vertexCount, true,
                                clampRuns.empty() ? nullptr : &clampRuns[0],
                                (int)clampRuns.size());
                        }
                        sliceCount = 0;
                        batchVertices = 0;
                        return !abortDraw;
                    };

                    for (std::size_t ri = rangeIndex; ri < ranges.size() && !abortDraw; ++ri)
                    {
                        const Ps2MeshRange& range = ranges[ri];
                        const int_t rangeBegin = range.firstVertex;
                        const int_t rangeEnd = rangeBegin + range.vertexCount();
                        if (rangeBegin >= tileEnd)
                            break;
                        if (rangeEnd <= tileBegin)
                            continue;

                        const int visibilityClass = clusterClass[range.cluster()];
                        const bool clipSafe = clusterCanSkipClip(
                            visibilityClass, clusterGuardRisk[range.cluster()]);
                        if (visibilityClass == 0 || !clipSafe || !faceVisible(range.faceGroup()))
                            continue;

                        int_t cursor = rangeBegin > tileBegin ? rangeBegin : tileBegin;
                        const int_t endVertex = rangeEnd < tileEnd ? rangeEnd : tileEnd;
                        while (cursor < endVertex && !abortDraw)
                        {
                            if (sliceCount >= 8 || batchVertices >= PS2_VU1_TERRAIN_MAX_VERTICES)
                            {
                                if (!flushSlices())
                                    break;
                            }
                            int_t take = endVertex - cursor;
                            const int_t room = PS2_VU1_TERRAIN_MAX_VERTICES - batchVertices;
                            if (take > room) take = room;
                            take &= ~3;
                            if (take <= 0)
                            {
                                if (!flushSlices()) break;
                                continue;
                            }

                            if (sliceCount > 0 &&
                                slices[sliceCount - 1].firstVertex + slices[sliceCount - 1].vertexCount == cursor)
                            {
                                slices[sliceCount - 1].vertexCount += take;
                            }
                            else
                            {
                                if (sliceCount >= 8)
                                {
                                    if (!flushSlices()) break;
                                    continue;
                                }
                                slices[sliceCount].firstVertex = cursor;
                                slices[sliceCount].vertexCount = take;
                                ++sliceCount;
                            }
                            batchVertices += take;
                            cursor += take;
                        }
                    }
                    if (!abortDraw)
                        (void)flushSlices();
                }
                return;
            }
#endif
            // Fallback for raw meshes and experimental clipped-VU1 builds.
            int_t runStart = -1;
            int_t runCount = 0;
            bool runFullyInside = false;
            const std::vector<Ps2MeshRange>& ranges = section.faceGroups->ranges;
            for (std::size_t i = 0; i < ranges.size() && !abortDraw; ++i)
            {
                const Ps2MeshRange& range = ranges[i];
                const int visibilityClass = clusterClass[range.cluster()];
                const bool fullyInside = clusterCanSkipClip(
                    visibilityClass, clusterGuardRisk[range.cluster()]);
                const bool keep = visibilityClass != 0 &&
                    (fullyInside || PS2_VU1_CLIPPED_PARTIALS) &&
                    faceVisible(range.faceGroup());
                if (keep)
                {
                    if (runStart >= 0 && runFullyInside == fullyInside &&
                        runStart + runCount == range.firstVertex)
                    {
                        runCount += range.vertexCount();
                    }
                    else
                    {
                        if (runStart >= 0)
                            emitRange(runStart, runCount, runFullyInside);
                        runStart = range.firstVertex;
                        runCount = range.vertexCount();
                        runFullyInside = fullyInside;
                    }
                }
                else if (runStart >= 0)
                {
                    emitRange(runStart, runCount, runFullyInside);
                    runStart = -1;
                    runCount = 0;
                }
            }
            if (!abortDraw && runStart >= 0)
                emitRange(runStart, runCount, runFullyInside);
        };

        auto emitGatheredRanges = [&](bool wantClipSafe)
        {
			if (!packedOpaque)
			{
				const std::vector<Ps2MeshRange>& ranges = section.faceGroups->ranges;
				for (std::size_t i = 0; i < ranges.size() && !abortDraw; ++i)
				{
					const Ps2MeshRange& range = ranges[i];
					const bool clipSafe = clusterCanSkipClip(
						clusterClass[range.cluster()], clusterGuardRisk[range.cluster()]);
					if (clusterClass[range.cluster()] == 0 || clipSafe != wantClipSafe ||
						!faceVisible(range.faceGroup()))
						continue;
					PS2_TERRAIN_CLUSTER_STAT(++s_clusterStats.vu0GatherBatches);
					PS2_TERRAIN_CLUSTER_STAT(
						s_clusterStats.vu0GatherVertices += range.vertexCount());
					emitRange(range.firstVertex, range.vertexCount(), wantClipSafe);
				}
				return;
			}

            // Zero-copy gather: accumulate a bounded list of {firstVertex,
            // vertexCount} slices of the mesh's OWN arrays instead of
            // memcpy'ing them into a scratch buffer. Clamp runs are resolved
            // by Ps2Draw3D's clampRunCursor straight from the mesh's own tile
            // run table (clampRuns below), in the mesh's own vertex-index
            // space -- so unlike the old memcpy path, there is no rebasing to
            // a compacted buffer's local indices to track here at all.
            static const int kMaxGatherSlices = 32;
            Ps2NativeSlice gatherSlices[kMaxGatherSlices];
            int gatherSliceCount = 0;
            int_t gatherVertices = 0;
            const std::vector<Ps2MeshRange>& ranges = section.faceGroups->ranges;
            const std::vector<Ps2TerrainTileRun>& clampRuns = section.opaqueMesh->runs();

            auto flushGather = [&]()
            {
                if (gatherVertices <= 0 || abortDraw)
                    return;
                PS2_TERRAIN_CLUSTER_STAT(++s_clusterStats.vu0GatherBatches);
                PS2_TERRAIN_CLUSTER_STAT(s_clusterStats.vu0GatherVertices += gatherVertices);
                emitVu0PackedSliced(section.opaqueMesh->positions(),
                    section.opaqueMesh->texCoords(), section.opaqueMesh->colors(),
                    gatherSlices, gatherSliceCount, gatherVertices, wantClipSafe,
                    clampRuns.empty() ? nullptr : &clampRuns[0], (int)clampRuns.size());
                gatherSliceCount = 0;
                gatherVertices = 0;
            };

            // Quad-aligned by construction: every range is a whole number of
            // quads, kMaxTerrainBatchVertices is a multiple of 4, and every
            // `take` below is rounded down to one -- so a split slice can
            // only end on a quad boundary, never mid-quad.
            auto appendSlice = [&](int_t firstVertex, int_t vertexCount)
            {
                while (vertexCount > 0 && !abortDraw)
                {
                    const bool canMerge = gatherSliceCount > 0 &&
                        gatherSlices[gatherSliceCount - 1].firstVertex +
                            gatherSlices[gatherSliceCount - 1].vertexCount == firstVertex;
                    if (!canMerge && gatherSliceCount >= kMaxGatherSlices)
                    {
                        flushGather();
                        continue;
                    }

                    int_t take = kMaxTerrainBatchVertices - gatherVertices;
                    if (take > vertexCount) take = vertexCount;
                    take -= take & 3;
                    if (take <= 0)
                    {
                        flushGather();
                        continue;
                    }

                    if (canMerge)
                    {
                        gatherSlices[gatherSliceCount - 1].vertexCount += (int)take;
                    }
                    else
                    {
                        gatherSlices[gatherSliceCount].firstVertex = (int)firstVertex;
                        gatherSlices[gatherSliceCount].vertexCount = (int)take;
                        ++gatherSliceCount;
                    }
                    gatherVertices += take;
                    firstVertex += take;
                    vertexCount -= take;
                }
            };

            for (std::size_t i = 0; i < ranges.size() && !abortDraw; ++i)
            {
                const Ps2MeshRange& range = ranges[i];
                const bool clipSafe = clusterCanSkipClip(
                    clusterClass[range.cluster()],
                    clusterGuardRisk[range.cluster()]);
                const bool submittedByVu1 = directVu1Usable &&
                    (clipSafe || PS2_VU1_CLIPPED_PARTIALS);
                if (clusterClass[range.cluster()] == 0 ||
                    clipSafe != wantClipSafe ||
                    submittedByVu1 ||
                    !faceVisible(range.faceGroup()))
                    continue;

                appendSlice(range.firstVertex, range.vertexCount());
            }

            flushGather();
        };

		if (directVu1Usable && s_opaqueSubmitPhase != PS2_OPAQUE_SUBMIT_VU0)
            emitVu1Ranges();
		if (!abortDraw && directVu1Usable &&
			s_opaqueSubmitPhase == PS2_OPAQUE_SUBMIT_VU1 &&
			ps2_vu1_terrain_clipped_probe_available())
		{
			const std::vector<Ps2MeshRange>& ranges = section.faceGroups->ranges;
			for (std::size_t i = 0; i < ranges.size() &&
				ps2_vu1_terrain_clipped_probe_available(); ++i)
			{
				const Ps2MeshRange& range = ranges[i];
				const int visibilityClass = clusterClass[range.cluster()];
				const bool fullyInside = clusterCanSkipClip(
					visibilityClass, clusterGuardRisk[range.cluster()]);
				if (visibilityClass != 0 && !fullyInside &&
					faceVisible(range.faceGroup()))
				{
					emitClippedProbe(range.firstVertex, range.vertexCount());
				}
			}
		}
		if (!abortDraw && s_opaqueSubmitPhase != PS2_OPAQUE_SUBMIT_VU1)
            emitGatheredRanges(true);
		if (!abortDraw && s_opaqueSubmitPhase != PS2_OPAQUE_SUBMIT_VU1)
            emitGatheredRanges(false);
    }
    else
    {
		if (!section.fullyInside)
			emitClippedProbe(0, totalVertices);

		// With no classified ranges the whole mesh is one VU1 range. Its VU0
		// phase is needed only when that VU1 submission explicitly asked to retry.
		const bool submittedByVu1 = directVu1Usable &&
			(section.fullyInside || PS2_VU1_CLIPPED_PARTIALS);
		if (!(s_opaqueSubmitPhase == PS2_OPAQUE_SUBMIT_VU0 && submittedByVu1))
			emitRange(0, totalVertices, section.fullyInside);
    }

    return result;
}

// Read by the VIF1 stall dump in Ps2Vu1DmaQueue.cpp. Lives here because this is
// the only module that knows how far the opaque replay has got.
extern "C" int ps2_dbg_terrain_section()
{
	return s_traceSection;
}

void ps2_terrain_take_cluster_stats(Ps2TerrainClusterStats& out)
{
    out = s_clusterStats;
    std::memset(&s_clusterStats, 0, sizeof(s_clusterStats));
}

#endif
