#pragma once

#include <cstdint>

#include "wii/render/WiiRenderTypes.h"

struct WiiNativeMeshView
{
    const void* vertices = nullptr;
    int vertexStride = 0;
    bool texCoordEnabled = false;
    bool colorEnabled = false;
    bool hasNormals = false;
    bool brightnessEnabled = false;
    int brightnessOffset = 28;
    WiiRenderPrimitive primitive = WiiRenderPrimitive::Quads;
    int first = 0;
    int count = 0;
};

// Native GX path for the fixed 32-byte Minecraft Tessellator layout. Covers
// the common unlit path plus Minecraft's standard directional entity/item
// lighting. The Wii game path is GX-only. A false return means the supplied mesh/state
// is invalid or outside Beta++'s supported fixed-function contract; there is no
// compatibility submission fallback in the Wii build.
bool wii_native_draw_mesh(const WiiNativeMeshView& mesh);

// Call when GX texture state is changed outside WiiNativeDraw so the native
// texture-load cache cannot mistake a temporary binding for the current one.
void wii_native_invalidate_texture_binding_cache();

// Persistent static geometry recorded as a real GX display list. Unlike chunk
// lists, these handles carry no terrain transform and are replayed against the
// live modelview/projection and fixed-function state.
int wii_native_create_static_list_handle();
void wii_native_destroy_static_list_handle(int handle);
bool wii_native_compile_static_list(int handle, const void* vertices, int vertexStride,
                                    bool texCoordEnabled, bool colorEnabled, bool hasNormals,
                                    bool brightnessEnabled, int brightnessOffset,
                                    WiiRenderPrimitive primitive, int first, int count);
bool wii_native_call_static_list(int handle);

// Per-face-direction split of a chunk's opaque geometry, as produced by
// wii_sort_opaque_faces(). Supplying it makes the compile record one display
// sub-list per direction instead of one list for the section, so the draw can
// skip the directions that face away from the eye.
//
// Counts are quads in the reordered vertex buffer, in group order; the sum must
// equal the mesh's quad count or the split is refused and the whole section is
// recorded as a single list. The plane extents are section-local and are tested
// against the eye position relative to worldOrigin, which is the section's true
// world corner -- NOT the clipped translate the modelview uses, which has been
// through the 1024-block wrap.
struct WiiNativeChunkFaceGroups
{
    static const int kGroupCount = 7;
    int quadCount[kGroupCount];
    float planeMin[kGroupCount];
    float planeMax[kGroupCount];
    float worldOriginX;
    float worldOriginY;
    float worldOriginZ;
};

// Compile/replay terrain geometry as a real GX display list. Native chunk
// handles live in their own namespace and are never compatibility display-list names.
// Terrain geometry itself has no compatibility display-list fallback on Wii.
int wii_native_create_chunk_handle();
void wii_native_destroy_chunk_handle(int handle);
bool wii_native_compile_chunk_list(int handle, const void* vertices, int vertexStride,
                                   bool texCoordEnabled, bool colorEnabled,
                                   bool brightnessEnabled, int brightnessOffset,
                                   WiiRenderPrimitive primitive, int first, int count,
                                   float translateX, float translateY, float translateZ,
                                   float sectionSize,
                                   const WiiNativeChunkFaceGroups* faceGroups = nullptr);
bool wii_native_call_chunk_list(int handle);
enum WiiNativeTerrainPass
{
    WII_NATIVE_TERRAIN_OPAQUE = 0,
    WII_NATIVE_TERRAIN_TRANSLUCENT_DEPTH = 1,
    WII_NATIVE_TERRAIN_TRANSLUCENT_COLOR = 2
};

void wii_native_capture_terrain_camera();
void wii_native_set_terrain_render_list_offset(float x, float y, float z);

// Interpolated eye position for the frame being drawn, in world coordinates.
// Published once per terrain pass; the face-direction cull is the only consumer
// and simply keeps every group when nothing has published one.
void wii_native_set_terrain_eye(float x, float y, float z);
void wii_native_set_terrain_fog(int mode, float density, float start, float end,
                                float r, float g, float b, float a);

// Opt the following chunk batches of the OPAQUE terrain pass out of the alpha
// test, which is what lets GX_SetZCompLoc(GX_ENABLE) reject a fragment against
// the Z buffer before the TEV stages texture it.
//
// Only legal for lists whose geometry has no texels the alpha test would have
// rejected; the caller owns that guarantee (WorldRenderer::terrainPassNeedsAlphaTest).
// Takes effect on the next batch, resets to false at both ends of a terrain
// pass, and is ignored outside the opaque pass.
void wii_native_set_terrain_early_depth(bool enabled);
bool wii_native_begin_terrain_pass(WiiNativeTerrainPass pass);
void wii_native_end_terrain_pass();
void wii_native_invalidate_terrain_pass();
bool wii_native_begin_chunk_batch(int firstHandle);
bool wii_native_append_chunk_list(int handle);
void wii_native_end_chunk_batch();
void wii_native_clear_chunk_list(int handle);
void wii_native_swap_chunk_lists(int firstHandle, int secondHandle);
unsigned int wii_native_chunk_list_memory();
unsigned int wii_native_chunk_list_live_bytes();
unsigned int wii_native_chunk_list_count();
void wii_native_chunk_buffer_pool_stats(unsigned int *hits, unsigned int *misses,
                                        unsigned int *overflows, unsigned int *pooledBytes);

// Face-direction cull effectiveness, counted in display sub-lists since the
// last boot. A section split into groups that are never culled is pure overhead,
// so these are what say whether the split is paying for itself.
void wii_native_face_cull_stats(unsigned int *drawn, unsigned int *culled);
