#pragma once

#ifdef PS2_PLATFORM

// Greedy mesher for the PS2 terrain build path.
//
// The default PS2 chunk build emits one quad per visible cube face. Greedy
// meshing merges adjacent, identically-shaded faces of full opaque cubes into a
// single larger quad, which cuts the vertex count the EE has to transform every
// frame (the PS2 is EE/transform bound, not fill bound). The merged quad tiles
// its single 16x16 atlas cell via the GS REGION_REPEAT path already implemented
// in ps2_set_texture_clamp_for_uv().
//
// Only "simple full opaque cubes" are greedy-meshed (see ps2_is_greedy_cube).
// Everything else (stairs, slabs, crops, glass, water, torches, ...) must keep
// going through RenderBlocks::renderBlockByRenderType in the normal per-block
// loop, so the caller must skip greedy cubes there to avoid double rendering.

class Block;
class ChunkCache;

// True when this block is handled by the greedy pass and must therefore be
// skipped by the per-block RenderBlocks loop (opaque pass only).
bool ps2_is_greedy_cube(Block* block);

// Greedy-mesh ONE face direction (0..5, matching RenderBlocks: 0=Y- 1=Y+ 2=Z-
// 3=Z+ 4=X- 5=X+) of the opaque full cubes inside [x0,x1) x [y0,y1) x [z0,z1)
// into Tessellator::instance (the caller must have started a quad batch).
// Coordinates are world block coordinates; vertices are emitted in world space
// (the section translation is applied by the caller). Returns true if anything
// was emitted.
//
// This used to sweep all six faces in one call, and the caller ran it entirely
// on the first step of the opaque pass -- outside the per-step block budget
// completely. That is a 16x16x16 section x 6 directions = 24576 makeFaceKey
// probes (each a getBlockId + shouldSideBeRendered + getBlockTexture +
// colorMultiplier + getBlockBrightness) in a single frame, which is where the
// "chunk build max=191.3ms" spikes in the FRAME log came from: the budget the
// rest of the build respects simply did not apply to it.
//
// The caller may restrict the bounds to a group of independent planes, spreading
// one face direction over several steps. The completed opaque mesh is
// subsequently grouped by atlas clamp in Ps2MeshSort, while the rectangle merge
// here removes most of the quads before that sort runs.
bool ps2_greedy_mesh_face(ChunkCache& cc, int face,
                          int x0, int y0, int z0,
                          int x1, int y1, int z1);

// Number of face directions ps2_greedy_mesh_face accepts.
enum { PS2_GREEDY_FACE_COUNT = 6 };

#endif // PS2_PLATFORM
