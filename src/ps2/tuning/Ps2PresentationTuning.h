#pragma once

#define PS2_SIN_TABLE_BITS 6

// Animated texture (TextureFX) update rate, in world ticks. Water, lava, fire,
// the portal, the compass and the clock each rewrite a 16x16 tile of their
// atlas every tick, and any tile touched marks the whole atlas dirty -- the PS2
// GL wrapper then re-sends the entire 256x256 terrain (and items) texture to
// VRAM on the next bind. That is ~64KB of DMA plus a palette re-map of every
// touched texel, 20 times a second, for animation nobody can see at 640x448.
//
// This does NOT affect the heap any more (the upload now runs through the
// parked block in gles_ps2.cpp), it is purely an EE/GS time knob:
//   1 = vanilla rate (default, no visual change)
//   N = update every N ticks (animation runs N times slower)
//   0 = never update after the first frame (fully static water/lava/fire)
#define PS2_DYNAMIC_TEXTURE_INTERVAL_TICKS 4

// GUI scale. Beta's ScaledResolution auto-scale only leaves scale 1 when
// height/(scale+1) >= 240. The PS2 NTSC framebuffer is 640x448, so 448/2 = 224
// < 240 and the HUD/menus are permanently locked at the tiny scale 1 -- the
// in-game "GUI Scale" option does nothing on console. Force a fixed factor here:
//   1 = vanilla tiny, 2 = Normal (320x224 virtual canvas), 3 = Large (213x149).
//   0 = fall back to the vanilla auto-scale loop (GameSettings.guiScale).
#define PS2_FORCE_GUI_SCALE 2

// Depth mapping. See the long note on ps2_gs_depth in Ps2ClipGuard.h: the GS
// depth buffer is PSMZ_16S (15 usable bits) and the textbook z_ndc mapping is
// hyperbolic, so precision collapses to a handful of integers past ~20 blocks
// and surfaces a fraction of a block apart -- a water top and the grass rim
// beside it -- quantise to the same Z and fight.
//   1 = Z proportional to eye distance (W buffer), ~512 units/block everywhere
//   0 = vanilla GL z_ndc mapping
// NOTE: 0 here is not a fallback, it is required by PS2_ENABLE_GREEDY_MESH.
// Linear Z is interpolated linearly in SCREEN space by the GS, so a triangle
// bows by span^2/(4t) blocks. Per-block quads span <= 1.4, which is nothing;
// greedy merges up to 16x16 cells, and a merged floor seen at a grazing angle
// from 5 blocks bows by 256/20 = ~13 blocks — surfaces punch through each
// other. Turning greedy off makes 1 safe again (and much more precise); with
// greedy on, the hyperbolic mapping plus a raised near plane is the pair that
// works.
#define PS2_LINEAR_DEPTH 0

// Camera near plane, in blocks. Only relevant with PS2_LINEAR_DEPTH 0, where
// depth resolution is proportional to it: 0.2 buys 4x over vanilla's 0.05 and
// is still inside the 0.3 half-width of the player's collision box, so it
// cannot open a hole through a wall the player is standing against. Raising it
// further starts clipping geometry the player can legitimately get close to.
#define PS2_NEAR_PLANE 0.2f

// Depth-mapping near plane, as a multiple of the projection's own near plane.
//
// With PS2_LINEAR_DEPTH 0 the GS Z resolution is
//     dZ/dt = ZMAX * near * far / ((far - near) * t^2)
// so it is proportional to the NEAR plane and essentially independent of the
// far one. PS2_NEAR_PLANE cannot be raised much further without clipping walls
// the player can legitimately stand against -- but the near plane the DEPTH
// MAPPING uses does not have to be the one the projection matrix uses. Nothing
// clips differently if only the mapping moves.
//
// The mapping is rewritten as an affine function of q = 1/w, which every path
// already has from the perspective divide:
//     z_gs = ZMAX * n' * far / (far - n') * q  -  ZMAX * n' / (far - n')
// At n' = near this reproduces the textbook mapping exactly; above it, every
// distance gains precision by the same factor.
//
// The cost is that geometry closer than n' clamps to ZMAX and loses ordering
// against other geometry that close. At the world's 0.2 near plane a boost of
// 5 puts n' at 1 block: inside that radius you can only be seeing the face of
// a block you are pressed against, and the first-person hand renders in its
// own pass behind its own depth clear. Raise this if surfaces a fraction of a
// block apart (water against a shore, snow on grass, a pressure plate) still
// fight at distance; lower it if geometry within a block of the eye starts
// sorting by submission order. 1.0 restores the original mapping.
#define PS2_DEPTH_NEAR_BOOST 1.0f

// Margin, in blocks, added around a chunk section's frustum-test box. Vanilla
// uses 6 to size the occlusion-query box; console never issues those queries,
// so here it only makes the frustum test accept sections it should reject --
// and each accepted section costs ~1000 EE vertex transforms. 1 keeps the
// section's own 1-block mesh margin covered. Raise it if geometry pops at the
// edge of view.
#define PS2_RENDERER_AABB_MARGIN 1.0f

// F3 lagometer: the frame-time histogram Minecraft::displayDebugInfo draws
// across the bottom of the screen. It replays 512 samples as individual
// drawRect calls every frame, on top of its own glClear(GL_DEPTH_BUFFER_BIT)
// and ortho setup -- pure cost on console, and the [PS2][FRAME] log already
// reports the same timings broken down by phase and with more precision than a
// bar chart on a TV. The F3 *text* readout is drawn by GuiIngame and is not
// affected by this knob.
//   1 = vanilla histogram
//   0 = compiled out
#define PS2_SHOW_LAGOMETER 0

// Software mouse pointer. The PS2 has no OS cursor, so the pointer that drives
// the existing mouse-hover/click GUI code has to be drawn by the game itself.
//   1 = draw data/assets/cursor.png (editable art)
//   0 = the old hardcoded arrow, six drawRect() calls, no texture needed
#define PS2_CURSOR_TEXTURE 1

// Drawn size of that pointer, in GUI units -- i.e. before ScaledResolution
// multiplies by PS2_FORCE_GUI_SCALE. cursor.png is 32x32, so at GUI scale 2 the
// default 16 puts it on screen at exactly its native 32x32 with no resampling;
// any other combination resamples with GS_FILTER_NEAREST and the 1px black
// outline in the art starts to break up.
#define PS2_CURSOR_SIZE 16

// Camera/input safety. Do not route PS2 right-stick camera through the fake
// LWJGL mouse event queue. Apply it directly in EntityRenderer so stale mouse
// deltas, menu cursor state, or SDL-style grab behavior cannot spin the view.
