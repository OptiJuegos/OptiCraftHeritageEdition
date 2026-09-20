#pragma once

// Central platform feature switches.
//
// Keep exact hardware/backend checks narrow:
//   PLATFORM_PS2        -> PlayStation 2 only. Use for pad, Memory Card,
//                          gsKit/GLES wrapper, PS2SDK, VU/GS, etc.
//   PLATFORM_WII        -> Nintendo Wii only. Use for WPAD/PAD, libfat paths,
//                          the GX wrapper, libogc, ASND, MEM1/MEM2, etc.
//
// Use feature/profile checks for game-side compromises. There are TWO, and the
// split matters -- see the PLATFORM_BOUNDED_WORLD block further down:
//   PLATFORM_BOUNDED_WORLD -> the world must fit in a fixed memory budget. Use
//                           for chunk cache radius, unload policy, preload
//                           radius, generation/decoration throttles.
//   PLATFORM_CONSOLE_LOW  -> weak CPU profile. Use for arithmetic shortcuts
//                           (float noise, heightmap terrain), lighting and
//                           entity/random tick cuts, fixed renderer grids, and
//                           backend workarounds a stronger console does not need.
//
// New low-end console ports can define their own PLATFORM_<NAME> and opt into
// either profile without pretending to be PS2.

#ifndef PLATFORM_PS2
#  if defined(PS2_PLATFORM)
#    define PLATFORM_PS2 1
#  else
#    define PLATFORM_PS2 0
#  endif
#endif

#ifndef PLATFORM_WII
#  if defined(WII_PLATFORM)
#    define PLATFORM_WII 1
#  else
#    define PLATFORM_WII 0
#  endif
#endif

// User-facing hardware calibration features.
#ifndef PLATFORM_HAS_CONTROLLER_CALIBRATION
#  define PLATFORM_HAS_CONTROLLER_CALIBRATION (PLATFORM_PS2 || PLATFORM_WII)
#endif

#ifndef PLATFORM_HAS_ASPECT_RATIO_OPTION
#  define PLATFORM_HAS_ASPECT_RATIO_OPTION (PLATFORM_PS2 || PLATFORM_WII)
#endif

// Game-side optimization policies. These describe the reason a code path exists
// instead of naming the console that first needed it.
#ifndef PLATFORM_CACHE_NEAREST_PLAYER
#  define PLATFORM_CACHE_NEAREST_PLAYER (PLATFORM_PS2 || PLATFORM_WII || PLATFORM_PC_LEGACY)
#endif

// The Wii takes the throttle too: it is a tick-rate policy over distance, not
// an arithmetic shortcut, so it does not belong to PLATFORM_CONSOLE_LOW. The
// radii and divisors it reads come from WiiWorldTuning.h.
#ifndef PLATFORM_THROTTLE_ENTITY_AI
#  define PLATFORM_THROTTLE_ENTITY_AI (PLATFORM_PS2 || PLATFORM_WII || PLATFORM_PC_LEGACY)
#endif

// Entities with a chunk retention radius (the Ender Dragon) keep their
// footprint resident and generated while they cross the sliding world window.
// A bounded-world concern, not a CPU one: without it the Wii unloads the
// dragon with its chunk the moment it flies past the cache radius.
#ifndef PLATFORM_ENTITY_CHUNK_RETENTION
#  define PLATFORM_ENTITY_CHUNK_RETENTION (PLATFORM_PS2 || PLATFORM_WII)
#endif

// java.util.Random's 48-bit LCG step as 32-bit multiplies (see Random::next).
// Bit-identical to the 64-bit product, so seeds stay compatible; it only
// matters on cores where a 64-bit multiply is a library call.
#ifndef PLATFORM_RANDOM_SPLIT_MULTIPLY
#  define PLATFORM_RANDOM_SPLIT_MULTIPLY (PLATFORM_PS2 || PLATFORM_WII)
#endif

#ifndef PLATFORM_DIRECT_ANALOG_MOVEMENT
#  define PLATFORM_DIRECT_ANALOG_MOVEMENT PLATFORM_PS2
#endif

#ifndef PLATFORM_ASYNC_CHUNK_GENERATION
#  define PLATFORM_ASYNC_CHUNK_GENERATION (PLATFORM_WII || PLATFORM_PC_LEGACY)
#endif

// OptiFine custom animations (/anim/*.properties, custom_terrain_N.png,
// custom_water_*.png...). Off on the consoles: nothing ships them, and the
// probe alone is ~520 optional files x several spellings of failed opens on
// every RenderEngine (re)load -- a FAT directory walk each over USB/SD.
#ifndef PLATFORM_OPTIFINE_CUSTOM_ANIMATIONS
#  define PLATFORM_OPTIFINE_CUSTOM_ANIMATIONS (!(PLATFORM_PS2 || PLATFORM_WII))
#endif

#ifndef PLATFORM_OPTIFINE_RANDOM_MOBS
#  define PLATFORM_OPTIFINE_RANDOM_MOBS (!(PLATFORM_PS2 || PLATFORM_WII))
#endif

#ifndef PLATFORM_OPTIFINE_CUSTOM_FONTS
#  define PLATFORM_OPTIFINE_CUSTOM_FONTS (!(PLATFORM_PS2 || PLATFORM_WII))
#endif

#ifndef PLATFORM_LOCAL_STATS
#  define PLATFORM_LOCAL_STATS (PLATFORM_PS2 || PLATFORM_WII)
#endif

#ifndef PLATFORM_ENUMERATE_SAVE_DIRECTORIES
#  define PLATFORM_ENUMERATE_SAVE_DIRECTORIES PLATFORM_PS2
#endif

#ifndef PLATFORM_LOCAL_RESOURCES_ONLY
#  if defined(NO_NETWORK) || PLATFORM_PS2 || PLATFORM_WII
#    define PLATFORM_LOCAL_RESOURCES_ONLY 1
#  else
#    define PLATFORM_LOCAL_RESOURCES_ONLY 0
#  endif
#endif

// Storage/region capabilities used by Minecraft-side save code.
#ifndef PLATFORM_REGION_WHOLE_FILE_BUFFER
#  define PLATFORM_REGION_WHOLE_FILE_BUFFER PLATFORM_PS2
#endif

#ifndef PLATFORM_REGION_RANDOM_ACCESS
#  define PLATFORM_REGION_RANDOM_ACCESS PLATFORM_PS2
#endif

#ifndef PLATFORM_SMALL_REGION_SCRATCH
#  define PLATFORM_SMALL_REGION_SCRATCH PLATFORM_PS2
#endif

#ifndef PLATFORM_FAST_REGION_COMPRESSION
#  define PLATFORM_FAST_REGION_COMPRESSION (PLATFORM_PS2 || PLATFORM_WII)
#endif

#ifndef PLATFORM_PROFILE_STREAMING
#  define PLATFORM_PROFILE_STREAMING (PLATFORM_PS2 || PLATFORM_WII)
#endif

// PS2 region files keep a whole-region write buffer, so a modified chunk can be
// serialized when it leaves the resident cache without forcing an immediate
// Memory Card flush. Track gameplay edits separately from generation/lighting
// dirtiness so walking does not turn every generated chunk into an I/O write.
#ifndef PLATFORM_SAVE_RUNTIME_CHUNK_EDITS_ON_UNLOAD
#  define PLATFORM_SAVE_RUNTIME_CHUNK_EDITS_ON_UNLOAD PLATFORM_PS2
#endif

#ifndef PLATFORM_PROFILE_RENDER_PHASES
#  define PLATFORM_PROFILE_RENDER_PHASES (PLATFORM_PS2 || PLATFORM_WII)
#endif

#ifndef PLATFORM_NATIVE_TERRAIN_PIPELINE
#  define PLATFORM_NATIVE_TERRAIN_PIPELINE PLATFORM_WII
#endif

#ifndef PLATFORM_SINGLE_LOCAL_PLAYER
#  define PLATFORM_SINGLE_LOCAL_PLAYER PLATFORM_PS2
#endif

#ifndef PLATFORM_BOUNDED_PATHFIND
#  define PLATFORM_BOUNDED_PATHFIND (PLATFORM_CONSOLE_LOW || PLATFORM_WII || PLATFORM_PC_LEGACY)
#endif

#ifndef PLATFORM_HAS_VIRTUAL_KEYBOARD
#  define PLATFORM_HAS_VIRTUAL_KEYBOARD (PLATFORM_PS2 || PLATFORM_WII)
#endif

#ifndef PLATFORM_SIMPLE_TRANSPARENT_TERRAIN
#  define PLATFORM_SIMPLE_TRANSPARENT_TERRAIN PLATFORM_PS2
#endif

#ifndef PLATFORM_GUI_FORCE_DEPTH_DISABLED
// Native console GUI passes are pure 2D when no world is loaded.  Do not let
// TEST/ZBUF state inherited from a previous GS/GX pass decide whether the menu
// background is visible.  This is a 2D/3D state question only: gsKit does not
// alternate GS drawing contexts, gsKit_sync_flip() swaps ActiveBuffer and
// re-points both contexts at the new draw buffer while PrimContext stays put,
// so the every-other-frame old/black screen once blamed on per-context depth
// state was really FRAME.FBP (see ps2_apply_color_mask).
#  define PLATFORM_GUI_FORCE_DEPTH_DISABLED (PLATFORM_PS2 || PLATFORM_WII)
#endif

#ifndef PLATFORM_CHUNK_EDGE_FOG
#  define PLATFORM_CHUNK_EDGE_FOG PLATFORM_WII
#endif

#ifndef PLATFORM_PC
#  if PLATFORM_PS2 || PLATFORM_WII
#    define PLATFORM_PC 0
#  else
#    define PLATFORM_PC 1
#  endif
#endif

// Low-end desktop build selected by gcc32-legacy-release. This is deliberately
// independent from PLATFORM_CONSOLE_LOW: feature flags opt into selected CPU and
// world-generation shortcuts without inheriting the console memory model.
#ifndef PLATFORM_PC_LEGACY
#  if PLATFORM_PC && defined(PC_LEGACY_BUILD)
#    define PLATFORM_PC_LEGACY 1
#  else
#    define PLATFORM_PC_LEGACY 0
#  endif
#endif

#if PLATFORM_PC_LEGACY && !PLATFORM_PC
#  error "PLATFORM_PC_LEGACY is only valid for the desktop PC backend"
#endif

// The Wii is deliberately absent here. It has a hardware FPU, 24-bit Z and real
// GX display lists, so the arithmetic and lighting shortcuts this profile turns
// on are the wrong default for it -- they would change the generated world
// (float biome noise, heightmap terrain, trimmed Perlin octaves, a different
// bedrock RNG stream) and disable the light flood-fill, all to work around an
// EE the Wii does not have. Options persistence is handled independently by
// PlatformStorage on both consoles. cmake/wii.cmake exposes
// -DWII_CONSOLE_LOW=ON, which predefines PLATFORM_CONSOLE_LOW=1 on the command
// line and wins over this block, so the profile can be switched on from the
// build once there are frame-time measurements to justify it.
//
// What the Wii DID need from the old combined profile is the memory half, which
// is now PLATFORM_BOUNDED_WORLD below.
#ifndef PLATFORM_CONSOLE_LOW
#  if PLATFORM_PS2
#    define PLATFORM_CONSOLE_LOW 1
#  else
#    define PLATFORM_CONSOLE_LOW 0
#  endif
#endif

// World-generation features (WorldGenLakes, WorldGenBigTree, ...) that carry
// their own float/double type alias and switch on this rather than reading
// PLATFORM_CONSOLE_LOW directly, so a feature can be brought onto the float
// path individually as each one is verified against Random's call sequence.
// WorldGenLakes and WorldGenBigTree both already route every random draw
// through nextFloat()/nextDoubleFloat() -- the same next(26)/next(27) calls
// nextDouble() makes -- so switching the arithmetic here does not change how
// many random numbers a feature consumes, and therefore cannot diverge a seed.
#ifndef PLATFORM_FLOAT_FEATURE_GENERATION
#  define PLATFORM_FLOAT_FEATURE_GENERATION (PLATFORM_CONSOLE_LOW || PLATFORM_PC_LEGACY)
#endif

// Bound the resident world to a fixed memory budget.
//
// This used to be part of PLATFORM_CONSOLE_LOW, and bundling the two cost the
// Wii port a working configuration: it has ~60 MB of heap against the PS2's 32,
// so it needs every one of the memory guards -- a chunk cache with a real unload
// radius, a preload radius that is not the desktop's 17x17 columns (~23 MB
// before the first frame), throttled synchronous generation, and deferred
// decoration -- while needing NONE of the CPU compromises above. With a single
// switch it could only have both or neither, and "neither" is what put it on the
// out-of-memory screen.
//
// Gate on this for anything whose reason is "the chunks do not fit". Gate on
// PLATFORM_CONSOLE_LOW for anything whose reason is "the CPU cannot afford it"
// or "this backend cannot do it".
//
// PLATFORM_CONSOLE_LOW implies this: a platform that cannot afford the CPU
// certainly cannot afford unbounded memory, and the implication keeps every
// existing PS2 configuration -- including -DWII_CONSOLE_LOW=ON -- valid.
#ifndef PLATFORM_BOUNDED_WORLD
#  if PLATFORM_PS2 || PLATFORM_WII || PLATFORM_CONSOLE_LOW
#    define PLATFORM_BOUNDED_WORLD 1
#  else
#    define PLATFORM_BOUNDED_WORLD 0
#  endif
#endif

#if PLATFORM_CONSOLE_LOW && !PLATFORM_BOUNDED_WORLD
#  error "PLATFORM_CONSOLE_LOW requires PLATFORM_BOUNDED_WORLD: the CPU profile \
reads chunk-cache and generation-throttle state that only the memory profile \
declares."
#endif

// Bound the decoded ARGB helper cache on PS2 to one most-recent resource.
// This preserves useful back-to-back reuse (compass/watch both read items.png)
// without retaining every 256x256 colormap/atlas for the whole session.
#ifndef PLATFORM_BOUNDED_DECODED_TEXTURE_CACHE
#  define PLATFORM_BOUNDED_DECODED_TEXTURE_CACHE PLATFORM_PS2
#endif

#ifndef PLATFORM_HAS_SLOW_STORAGE
#  if PLATFORM_CONSOLE_LOW
#    define PLATFORM_HAS_SLOW_STORAGE 1
#  else
#    define PLATFORM_HAS_SLOW_STORAGE 0
#  endif
#endif

// Follows the memory profile, not the CPU one: the Wii is memory-constrained
// without being CPU-constrained, which is the whole point of the split above.
#ifndef PLATFORM_HAS_LIMITED_MEMORY
#  if PLATFORM_BOUNDED_WORLD
#    define PLATFORM_HAS_LIMITED_MEMORY 1
#  else
#    define PLATFORM_HAS_LIMITED_MEMORY 0
#  endif
#endif

// Draw the glyph quads immediately through the Tessellator instead of compiling
// them into 256 GL display lists and replaying them with glCallLists().
//
//   PS2  its GL wrapper has no display lists at all.
//   Wii  wiigx does implement them, but this is the one path in the port with
//        no display-list payoff -- 288 lists, each recorded through a 1 MB
//        scratch buffer during startup, to draw four vertices -- and the
//        immediate path is the same Tessellator quad path the rest of the GUI
//        already uses and that is known to work here. It is also the port's only
//        runtime evidence about display lists: the widgets and the logo render
//        while the text, which differs from them only by going through a list,
//        does not.
//
// This is deliberately NOT tied to PLATFORM_CONSOLE_LOW: it is a backend
// capability question, not a performance budget.
#ifndef PLATFORM_FONT_IMMEDIATE
#  if PLATFORM_PS2 || PLATFORM_WII
#    define PLATFORM_FONT_IMMEDIATE 1
#  else
#    define PLATFORM_FONT_IMMEDIATE 0
#  endif
#endif

// Backends without a persistent geometry object submit ModelRenderer boxes from
// their current transform. Wii and PS2 are both excluded: Wii compiles each box
// once into a native GX display list, PS2 into a captured RAM mesh, and both
// replay it against the live animated modelview.
#ifndef PLATFORM_MODEL_IMMEDIATE
#  define PLATFORM_MODEL_IMMEDIATE 0
#endif

// Persistent native meshes are a backend capability. Wii records immutable GX
// geometry; PC keeps the original GL retained path and PS2 uses captured RAM
// meshes/immediate submission instead.
#ifndef PLATFORM_PERSISTENT_RENDER_MESH
#  define PLATFORM_PERSISTENT_RENDER_MESH PLATFORM_WII
#endif

// Model geometry is persistent on more backends than terrain is. PS2 terrain
// keeps its own packed path and must not be routed through the persistent mesh
// API, but model boxes are invariant geometry worth compiling once: they are
// held in Ps2ModelGeometryCache and replayed with the live matrix stack, tint
// and lighting. Kept separate from PLATFORM_PERSISTENT_RENDER_MESH for exactly
// that reason.
#ifndef PLATFORM_MODEL_PERSISTENT_MESH
#  if PLATFORM_PS2
#    define PLATFORM_MODEL_PERSISTENT_MESH 1
#  else
#    define PLATFORM_MODEL_PERSISTENT_MESH PLATFORM_PERSISTENT_RENDER_MESH
#  endif
#endif

// Draw a pointer inside GuiScreen. Consoles have no OS cursor, so without this
// the existing mouse-hover/click GUI code is unusable: the player has no idea
// where they are aiming. Both console backends feed lwjgl::Mouse from a stick
// (PS2) or the Wiimote IR pointer (Wii), so the coordinates are already there.
#ifndef PLATFORM_SOFTWARE_CURSOR
#  if PLATFORM_PS2 || PLATFORM_WII
#    define PLATFORM_SOFTWARE_CURSOR 1
#  else
#    define PLATFORM_SOFTWARE_CURSOR 0
#  endif
#endif

// Framebuffer RGB readback is used only by screenshot backends. PS2 does not
// expose it, so do not keep a false RenderAPI stub in that target.
#ifndef PLATFORM_TEXTURE_QUALITY_CONTROLS
#  define PLATFORM_TEXTURE_QUALITY_CONTROLS (PLATFORM_PC || PLATFORM_WII)
#endif

#ifndef PLATFORM_FRAMEBUFFER_READBACK
#  define PLATFORM_FRAMEBUFFER_READBACK (PLATFORM_PC || PLATFORM_WII)
#endif
