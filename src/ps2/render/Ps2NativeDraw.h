#pragma once

#ifdef PS2_PLATFORM

// Values intentionally match the legacy OpenGL primitive constants so current
// Tessellator callers can migrate without translating their cached draw mode.
enum Ps2NativePrimitive {
    PS2_NATIVE_PRIM_POINTS = 0,
    PS2_NATIVE_PRIM_LINES = 1,
    PS2_NATIVE_PRIM_LINE_LOOP = 2,
    PS2_NATIVE_PRIM_LINE_STRIP = 3,
    PS2_NATIVE_PRIM_TRIANGLES = 4,
    PS2_NATIVE_PRIM_TRIANGLE_STRIP = 5,
    PS2_NATIVE_PRIM_TRIANGLE_FAN = 6,
    PS2_NATIVE_PRIM_QUADS = 7
};

// Optional texture-atlas metadata for packed terrain. Vertex indices use the
// same coordinate system as Ps2NativeMeshView::first, so a bounded sub-draw can
// share the parent mesh's run table without rebasing it.
struct Ps2NativeClampRun {
    int firstVertex;
    int vertexCount;
    unsigned char tileX;
    unsigned char tileY;
};

class Ps2NativeClampRunCursor {
public:
    Ps2NativeClampRunCursor(const Ps2NativeClampRun* runs, int runCount)
        : m_runs(runs), m_runCount(runCount > 0 ? runCount : 0), m_index(0)
    {
    }

    const Ps2NativeClampRun* find(int vertex)
    {
        if (m_runs == nullptr)
            return nullptr;

        while (m_index < m_runCount &&
               vertex >= m_runs[m_index].firstVertex + m_runs[m_index].vertexCount)
            ++m_index;

        if (m_index >= m_runCount || vertex < m_runs[m_index].firstVertex)
            return nullptr;
        return &m_runs[m_index];
    }

private:
    const Ps2NativeClampRun* m_runs;
    int m_runCount;
    int m_index;
};

// One contiguous sub-range of a packed mesh's own vertex/texCoord/color
// arrays. Lets a caller submit several non-contiguous fragments (e.g. face-
// bucket culling punching holes in an otherwise-contiguous tile run) without
// gathering them into a scratch copy first: the draw loop iterates the slice
// list directly against the mesh's own un-gathered arrays and keeps one
// shared batch/strip state across all of them, the same way the direct VU1
// terrain path's DMA REF chaining already avoids an EE-side copy for the
// identical reason (see ps2_vu1_terrain_append_sliced_batch).
struct Ps2NativeSlice {
    int firstVertex;
    int vertexCount;
};

// Transitional native PS2 geometry ingress.
//
// This started as a vertex-stream-only ingress and is being expanded in small
// pieces. Terrain now has explicit native pass setup, while fallback, GUI and
// lit-model state can still come from the compatibility layer. Keeping one GS
// renderer underneath both paths avoids duplicating raster logic during the
// migration.
struct Ps2NativeMeshView {
	// Packed terrain stores S16 XYZ1 at 1/1024, S16 UV at 1/4096 and
	// GS-native RGBA8. Its quad vertices are already in strip order. Other
	// native meshes use the ordinary interleaved float layout below.
	bool packedTerrain;
	const Ps2NativeClampRun* clampRuns;
	int clampRunCount;

    const void* vertices;
    int vertexStride;
    int vertexSize;

    const void* texCoords;
    int texCoordStride;
    bool texCoordEnabled;

    const void* colors;
    int colorStride;
    int colorSize;
    bool colorEnabled;
    bool colorFloat;

    const void* normals;
    int normalStride;
    bool normalFloat;
    bool hasNormals;

    int drawMode;
    int first;
    int count;

    // Optional multi-range source list (packedTerrain only). When set, the
    // draw iterates these slices from the same vertices/texCoords/colors
    // arrays instead of the single first/count range above; first is unused
    // in that case, but count must still equal the sum of every slice's
    // vertexCount (buffer-guard sizing and the entry sanity check both key
    // off it). nullptr/0 for every other caller -- this is a pure addition,
    // not a replacement for first/count.
    const Ps2NativeSlice* slices;
    int sliceCount;
};


// Base camera matrix for a native PS2 terrain pass. RenderGlobal snapshots it
// once, before iterating visible sections, so chunk rendering no longer needs
// an emulated GL matrix push/translate/pop or a projection*modelview multiply
// for every WorldRenderer.
struct Ps2NativeFrameContext {
    float mvp[16];
    float viewW;
    float viewH;
    bool valid;
};

// Prepared matrix state for one native PS2 geometry group. A translated copy
// is derived from Ps2NativeFrameContext once per section, then every bounded
// sub-draw reuses it.
struct Ps2NativeDrawContext {
    float mvp[16];
    bool fullyInside;
    bool valid;
};

enum Ps2NativeTerrainPass {
    PS2_NATIVE_TERRAIN_OPAQUE = 0,
    PS2_NATIVE_TERRAIN_TRANSLUCENT = 1
};

// Configure the terrain raster state directly in the PS2 backend. This keeps
// the world hot path from round-tripping texture/blend/depth/cull setup through
// the public OpenGL compatibility API while preserving the same internal state
// for fallback draws that still need it.
void ps2_native_begin_terrain_pass(unsigned int textureId, Ps2NativeTerrainPass pass);
void ps2_native_end_terrain_pass(Ps2NativeTerrainPass pass);

// Snapshot projection*modelview for the current camera without mutating GL
// state. Returns false when the active state is not suitable for a native 3D
// draw.
bool ps2_native_prepare_frame_context(Ps2NativeFrameContext& out);

// Append one model-space translation to a previously prepared frame context.
// For M*T only the fourth column changes, so this avoids another 4x4 multiply.
bool ps2_native_prepare_translated_context(Ps2NativeDrawContext& out,
                                           const Ps2NativeFrameContext& frame,
                                           float tx, float ty, float tz,
                                           bool fullyInside);

// Draw using an already prepared MVP. This is the chunk hot path.
// Returns false only when the mesh/state cannot be represented by the native PS2 backend.
bool ps2_native_draw_mesh_prepared(const Ps2NativeMeshView& mesh,
                                   const Ps2NativeDrawContext& context);

// Returns true when the draw was consumed by the native PS2 geometry ingress.
// Perspective draws use VU/gsKit; ortho HUD/GUI batches feed the existing native
// gsKit emitter without programming any OpenGL-compatible client-array state.
bool ps2_native_draw_mesh(const Ps2NativeMeshView& mesh);

#endif
