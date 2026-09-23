#pragma once


// Central PS2-only tuning knobs.  Do not include this from PC code unless it is
// behind #ifdef PS2_PLATFORM.

// Set to 1 only for builds intended to scan mounted HDD/PFS volumes.
#define PS2_SUPPORT_HDD 0

// Minecraft Beta renderDistance values:
// 0 = Far, 1 = Normal, 2 = Short, 3 = Tiny.
// Keep the vanilla option on Tiny, then override the actual PS2 renderer/cache
// with the chunk constants below.
#define PS2_DEFAULT_RENDER_DISTANCE 3

// PS2 playable profile: 2 chunk radius around the player.
// This means a 5x5 horizontal chunk grid for world rendering/loading.
// PC keeps the normal vanilla/OptiFine render distance table.
#define PS2_VISIBLE_CHUNK_RADIUS 2
#define PS2_VISIBLE_CHUNK_DIAMETER (PS2_VISIBLE_CHUNK_RADIUS * 2 + 1)

// The vanilla world is 128 blocks tall (8 sections), but a 5x8x5 grid would
// require 200 renderer slots. Keep three sections centred on the player instead:
// 5x3x5 = 75 slots, essentially the same fixed cost as the old 3x8x3 = 72 grid,
// while extending the horizontal window from 16 to 32 blocks. PC is unaffected.
#define PS2_VERTICAL_CHUNK_COUNT 3
#define PS2_CENTER_VERTICAL_RENDERERS 1

// Chunk cache policy. This radius is deliberately ONE LARGER than
// PS2_VISIBLE_CHUNK_RADIUS: it is the streaming window, not the render window.
//
// At radius 2 the two were equal, and canChunkExist() then refused every chunk
// outside the 5x5 the renderer was already drawing. A chunk therefore entered the
// generation queue at the exact moment it became visible, with no lead time, and
// PS2_GENERATE_CHUNKS_PER_TICK/PS2_GENERATION_BUDGET_US had to produce it inside
// one world tick or the player saw a hole. Crossing a border exposes 3 new chunks
// (5 counting the corners of a diagonal), which is more than the streamer can
// build at the frame rates this port runs at.
//
// Radius 3 gives it a one-chunk ring of lead time, which is also what makes the
// direction-scored prefetch in EntityRenderer::prefetchNearbyChunks useful --
// that code clamps its own radius to this value, so at radius 2 it could only
// ever re-request chunks that were already mandatory.
//
// Cost is ~24 extra resident columns, roughly 2 MB. Measured heap use at radius 2
// was 8 MB of the 32 MB budget, so this stays well inside it. Watch mallocUsed in
// the [PS2][FRAME] log; radius 2 is the rollback.
//
// Simulation radii are NOT tied to this any more -- see PS2_RANDOM_TICK_CHUNK_RADIUS
// and PS2_MOB_SPAWN_CHUNK_RADIUS, both pinned to the visible radius. Growing the
// streaming window must not grow the per-tick simulation cost.
//
// The unload radius stays one beyond the cache radius as a hysteresis margin, so
// walking one chunk does not cause immediate delete/reload thrashing.
//
// Back at radius 2 for memory: 25 mandatory / 49 maximum columns instead of
// 49 / 81, about 2 MB less resident world. The price is the lead-time ring
// described above -- a chunk enters the generation queue when it becomes
// visible, so crossing a border at speed can show a hole for a tick or two
// until the streamer catches up. If that is worse than the OOM it buys off,
// 3 / 4 is the rollback.
#define PS2_CHUNK_CACHE_RADIUS 2
#define PS2_CHUNK_UNLOAD_RADIUS 3
#define PS2_CHUNK_MAP_RESERVE 64

// Eviction rate. Chunks leave the cache only once they are BOTH outside
// PS2_CHUNK_UNLOAD_RADIUS and untouched for this many world ticks; the tick
// budget then bounds how many are actually freed per tick, because unloadChunk
// can write the chunk out and the Memory Card is slow.
//
// One per tick keeps the measured two-chunk unload burst out of a single frame.
// At 20 ticks/s it still drains normal movement faster than chunks become stale.
#define PS2_MAX_CHUNK_UNLOADS_PER_TICK 1
// Emergency eviction must stay incremental too. The old generic path multiplied
// the normal budget by 4, recreating the exact multi-chunk free/save spike this
// profile is trying to avoid. Two still drains pathological growth without
// allowing a large destructor/save burst in one tick.
#define PS2_EMERGENCY_CHUNK_UNLOADS_PER_TICK 2
#define PS2_MIN_UNUSED_TICKS_BEFORE_UNLOAD 60

// Multiplayer servers normally stream a much larger chunk window than this
// 32 MB client can keep resident. Keep Packet51 columns compressed until they
// enter the PS2 working set. Two promotions per world tick fills a fresh 5x5
// client window in roughly half the time of the old one-per-tick path while the
// renderer's independent wall-clock mesh budget still bounds geometry work.
// NetworkManager queue accounting stays honest because queued Packet51 objects
// remain near their compressed wire size instead of each expanding to ~100 KB.
// The byte and entry limits remain hard ceilings.
#define PS2_MP_DEFERRED_CHUNKS 1
#define PS2_MP_COMPRESSED_CHUNK_CACHE_BYTES (6u * 1024u * 1024u)
#define PS2_MP_CHUNK_PROMOTIONS_PER_TICK 2
#define PS2_MP_MAX_DEFERRED_CHUNKS 512u
#define PS2_MP_MAX_CHANGES_PER_CHUNK 128u
#define PS2_MP_MAX_DEFERRED_CHANGES 4096u

// Read-only worlds are already fully generated on disk and never need to keep
// dirty chunks resident for a later save. Use the visible radius as the load
// window, keep one chunk of unload hysteresis, and drain stale columns quickly
// enough that dense prebuilt maps cannot build up a large resident tail.
#define PS2_READ_ONLY_CHUNK_CACHE_RADIUS 2
#define PS2_READ_ONLY_CHUNK_UNLOAD_RADIUS 3
#define PS2_READ_ONLY_MAX_CHUNK_UNLOADS_PER_TICK 4
#define PS2_READ_ONLY_MIN_UNUSED_TICKS_BEFORE_UNLOAD 10

// New-world/preload radius in blocks. 32 blocks = 2 chunks each way.
#define PS2_PRELOAD_RADIUS_BLOCKS 32

// Vanilla refuses to tick entities unless a 32-block area around them is loaded.
// That is fine on PC, but PS2 intentionally keeps only a 5x5 chunk cache.
// With the vanilla 32-block guard the local player can stop ticking after spawn,
// which looks like floating in the air and makes movement/gravity do nothing.
// Keep this at 0 so the player only needs the current chunk loaded to simulate.
#define PS2_PLAYER_UPDATE_CHUNK_RANGE_BLOCKS 0

// Memory Card is extremely slow. During gameplay, PS2 should not autosave every
// few seconds like PC. Worlds can be saved explicitly from the pause menu/helper.
#define PS2_DISABLE_RUNTIME_AUTOSAVE 1
#define PS2_SKIP_NEW_WORLD_FULL_SAVE 1

// Presentation rate cap, in frames per second. 0 = uncapped (present on every
// vblank, so the rate floats between 60 and whatever the EE manages).
//
// 30 is the right target for this port: a steady 30 reads far smoother than a
// rate swinging between 40 and 60, and holding the frame frees EE time for the
// world tick instead of spending it on frames nobody perceives. The pacing loop
// in Display_ps2.cpp waits in whole fields, so a frame the EE cannot finish in
// budget still presents at its own rate rather than dropping to 15.
//
// Enabled 2026-08-30 as the first measured change from the render-pipeline
// audit (see PS2 terrain renderer audit, bottleneck #1): this is a pure
// config flip onto a pacing path that already existed and was previously
// unused. Rollback is this line alone -- set back to 0 to restore the exact
// uncapped behavior; nothing else in Display_ps2.cpp changes.
#define PS2_TARGET_FPS 30

// Tessellator backend facts. The GS path consumes native quads and the console
// heap cannot afford the desktop 8 MB scratch buffer. Keep these values in the
// platform profile rather than branching inside Tessellator.cpp.
#define PS2_TESSELLATOR_CONVERT_QUADS 0
#define PS2_TESSELLATOR_BUFFER_INTS   0x10000

// Vertex and UV math precision for the geometry emitters (Tessellator, the six
// RenderBlocks face emitters, the greedy mesher).
//
// The R5900 has no double-precision FPU, so every `double` add, multiply and
// divide is a libgcc software call. The face emitters are built entirely out of
// them: renderTopFace computes four UV coordinates as
// `((double)texel + bound * 16.0) / 256.0` and five vertex coordinates as
// `world + bound`, then Tessellator::addVertex adds the three translation
// offsets -- and narrows the result to float anyway, because the vertex buffer
// has always held 32-bit floats. That runs six times per block, for several
// hundred blocks, on every section rebuild.
//
// 1 makes those scalars `float` (tess_coord_t, declared in Tessellator.h) so
// the same arithmetic runs on the hardware FPU. The stored vertex is unchanged
// in format and, within the range below, in value.
//
// The one thing it costs: the translation offset is applied in float rather
// than double, i.e. `(float)world + (float)(-origin)` instead of
// `(float)(world - origin)`. Beta's block bounds are multiples of 1/16, so a
// position stays exact while the coordinate plus those four fractional bits fit
// the 24-bit float mantissa -- roughly 2^20 (about 1,000,000) blocks from the
// origin, past which vertices begin to snap. UVs are derived from the texture
// index and never from a world coordinate, so they are unaffected.
//
// It also selects the float half-angle in EntityRenderer's perspectiveGL. That
// one is not a frame-time decision -- it runs once per frame -- but it is the
// port's only live double tan(), so the float path can keep libm's
// double-precision kernel and its reduction tables out of a link that has 16 KB
// of instruction cache to fit into.
//
// Not covered here, because they were never double to begin with: the PS2
// projection/ortho backend entry points take float (see Ps2RenderApi.h), and
// the shared platform/RenderAPI.h keeps its double signatures for the GL
// backend, narrowing once in RenderAPI_GS_PS2.cpp.
//
// Set to 0 to restore the exact double path; that also isolates any geometry
// regression from the rest of the renderer. PC always uses the double path.
#define PS2_FLOAT_VERTEX_MATH 1

// Read chunk blocks, metadata and light through raw array bases resolved once
// per ChunkCache, instead of one virtual call per access.
//
// ChunkCache::getBlockId is the hottest accessor in the port: a section build
// issues roughly 25k of them from RenderBlocks (six neighbour probes per block)
// and one per cell per face direction from the greedy mesher. Every one ends in
// chunk->getBlockID(), which is virtual because EmptyChunk overrides it, so the
// EE drains its pipeline on an indirect jump it cannot predict -- on top of the
// IBlockAccess dispatch that RenderBlocks already suppresses (see the note on
// accessGetBlockId in RenderBlocks.h, which names this as the remaining half).
//
// With this on, the ChunkCache constructor resolves the block, metadata,
// skylight and blocklight arrays of each source chunk once and the accessors
// index them directly. Semantics are unchanged: a chunk that is missing, empty
// (EmptyChunk) or not fully sized resolves to a null base and every accessor
// then returns the same 0 those chunks already returned, including the
// Chunk::isLit side effect, which only fires on a real skylight value. The
// Chunk* array itself is untouched, so isBlockOpaqueCube's missing-chunk rule
// still sees exactly what it saw before.
#define PS2_FAST_CHUNK_BLOCK_READS 1

// Entity drop shadows. /misc/shadow.png is a soft radial alpha gradient, but the
// GS framebuffer is PSMCT16 with a single alpha bit, so the gradient collapses
// to fully-opaque or fully-gone: every shadow renders as hard black squares
// (one per ground block the shadow spans) instead of a soft blob. It is also
// genuinely expensive — renderShadow runs a 3D block scan plus a light lookup
// per entity per frame, and it runs for the inventory preview player too, which
// is where the black rectangle behind Steve came from.
#define PS2_SKIP_ENTITY_SHADOWS 1

// Compute MathHelper::floor_double from the IEEE-754 fields with integer
// arithmetic instead of casting through the double FPU the R5900 does not have.
//
// This is the most frequently called function in the port: 472 call sites, and
// the densest of them are the hot ones -- World.cpp alone has 91, and
// getCollidingBoundingBoxes, handleMaterialAcceleration and isMaterialInBB each
// spend six of them just to establish their block range. Every entity
// coordinate that becomes a block index goes through here.
//
// The current form costs six libgcc calls -- __unorddf2 for the NaN test, two
// clamp comparisons, __fixdfsi for the cast, then __floatsidf and __ltdf2 to
// re-compare the result against the input -- roughly 259 instructions. The EE is
// a 64-bit MIPS core, so loading the double as an integer and shifting out its
// exponent and mantissa is all hardware: 49 instructions and no libgcc at all.
//
// It returns the same int for every input, NaN, infinity, denormals and both
// zeros included; MathHelperFloorTests.cpp checks that against the original
// expression over eight million values. The subtle case is not the arithmetic,
// it is Java's behaviour at the edges: (int)d saturates to INT_MIN, and floor
// then subtracts one from it, which wraps to INT_MAX. An implementation that
// saturates to INT_MIN instead -- the obvious way to write it -- disagrees on
// every value below -2^31.
//
// PC and Wii keep the cast: both have double-precision hardware, where the
// original is a single instruction and this would be slower.
#define PS2_INTEGER_FLOOR_DOUBLE 1

// Let the 2D sprite batch outlive the draw call that filled it.
//
// Gui::drawTexturedModalRect starts a tessellator batch, emits one quad and
// draws it, so every heart, hunger shank, hotbar slot and glyph is a separate
// draw with a full packet and state round trip behind it. Measured in the
// tutorial world the HUD was 213 sprites for 11.2ms of a 37ms render -- about
// 52us per axis-aligned quad, which is overhead rather than rasterisation.
//
// The GS sprite batch already exists; this only stops it from being emptied at
// the end of every call, so consecutive quads sharing a texture and GS state
// leave in one packet. Correctness rests entirely on the barriers: every GS
// register write, every other draw path and the end of the frame call
// ps2_draw_2d_flush_pending() before touching the GS. The GUI draws back to
// front with no depth test, so a sprite that leaves late is a visible bug.
//
// Set to 0 to restore the flush-per-call behaviour.
#define PS2_BATCH_2D_SPRITES 1

// Cache the invariant Legacy UI control-hint glyph row as captured geometry.
// Labels/layout still invalidate normally; only the glyph tessellation is retained.
#define PS2_CACHE_LEGACY_HINT_TEXT 1

// Lightweight debug output. A line is printed every N frames from the main loop.
#define PS2_PERF_LOG_INTERVAL_FRAMES 120
