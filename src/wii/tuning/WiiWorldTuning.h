#pragma once


// Central Wii-only tuning knobs.
//
// Included from platform/PlatformTuning.h AFTER the desktop baseline table, and
// only when PLATFORM_WII is set. It is an override file, not a third copy of the
// table: the Wii agrees with the desktop baseline on most of those ~60 knobs, so
// only the ones it genuinely disagrees with appear here, each with the
// measurement or the arithmetic that chose it.
//
// What this file is NOT
// --------------------
// It is not the PS2 profile with different numbers. The Wii deliberately keeps
// PLATFORM_CONSOLE_LOW at 0 (see platform/PlatformConfig.h), so vanilla world
// generation, real lighting, options.txt and a user-selectable render distance
// all stay. Everything overridden below is under PLATFORM_BOUNDED_WORLD, i.e.
// "the chunks have to fit", plus two backend facts about the console.
//
// The budget these numbers are sized against
// ------------------------------------------
//   MEM1  24 MB, of which the ~14.5 MB DOL, two 640x480 framebuffers (~1.2 MB)
//         and the 256 KB GX FIFO leave roughly 8 MB.
//   MEM2  64 MB, ~52 MB of which is Arena2.
// With MALLOC_MEM2=1 (src/wii/system/WiiEarlyMemory.cpp), libogc lets malloc grow through
// the remaining MEM1 arena and then continue into MEM2. Against that:
//   - a loaded chunk column is ~80 KB (32 KB blocks + 3 nibble arrays + heightmap)
//   - a compiled chunk section is a GX display list at ~29 bytes per vertex,
//     which the port's own logs put at 20-200 KB per section
// The second of those is the larger consumer and is bounded by the render
// distance; these knobs bound the first, and the streaming PLATFORM_PRELOAD_RADIUS_BLOCKS          work that feeds it.

// -----------------------------------------------------------------------------
// Backend facts (not budgets)
// -----------------------------------------------------------------------------

#if PLATFORM_WII
#define PLATFORM_BOUNDED_WORLD 1
#endif

// GX accepts quads natively, so expanding each face from 4 to 6 vertices wastes
// a third of mesh memory and CPU. Start with 256 KiB and let Tessellator grow
// geometrically only when a batch actually needs more scratch space.
#undef  PLATFORM_TESSELLATOR_CONVERT_QUADS
#define PLATFORM_TESSELLATOR_CONVERT_QUADS       0
#undef  PLATFORM_TESSELLATOR_BUFFER_INTS
#define PLATFORM_TESSELLATOR_BUFFER_INTS         0x10000

// There is no OS cursor to fall back on, and data/assets/cursor.png ships with
// the game, so use the drawn pointer rather than the vector arrow.
#undef  PLATFORM_CURSOR_TEXTURE
#define PLATFORM_CURSOR_TEXTURE                  1

// Vanilla uses 6 to size the occlusion-query box around a section's frustum
// test. GX issues no occlusion queries, so here the margin only makes the
// test accept sections it should reject, and each accepted section costs a full
// GX display-list replay. 1 keeps the section's own 1-block mesh margin covered.
// Raise it if geometry pops at the edge of view.
#undef  PLATFORM_RENDERER_AABB_MARGIN
#define PLATFORM_RENDERER_AABB_MARGIN            1.0f

// -----------------------------------------------------------------------------
// Memory budget (PLATFORM_BOUNDED_WORLD)
// -----------------------------------------------------------------------------

// FAR is a desktop assumption, not a budget anyone checked against this heap.
// RenderGlobal::loadRenderers derives its grid from this value:
//
//   FAR    (0)  26 x 8 x 26 = 5408 sections,  676 chunk columns
//   NORMAL (1)  17 x 8 x 17 = 2312 sections,  289 chunk columns
//   SHORT  (2)   9 x 8 x  9 =  648 sections,   81 chunk columns
//
// FAR asks for ~54 MB of chunk columns before a single triangle exists, and the
// display lists on top of that cannot fit. The failure mode is the nasty one --
// not a crash but silently missing terrain, because a section whose list
// allocation fails is skipped and never retried, its WorldRenderer having
// already been marked clean.
//
// SHORT was the first estimate and hardware said no. Measured 2026-08-05 on the
// [WII][RAM] log, ~14 s after entering a world at SHORT:
//
//   post-spawnChunks  free=14076KB in 1418 blocks  sbrk left: MEM2=28303KB
//   in-world  (+4 s)  free= 2104KB in  378 blocks  sbrk left: MEM2=17743KB
//   in-world (+10 s)  free= 4288KB in  662 blocks  sbrk left: MEM2= 3147KB
//
// Read those as deltas rather than levels. Between the samples sbrk took 10.5 MB
// and then 14.5 MB more from Arena2 while the free total barely moved, i.e. ~35
// MB of NET LIVE allocation in 14 seconds, ending with ~53 MB live, 4 MB free and
// 3 MB of arena left. That is the world streaming in, and it is consumption, not
// fragmentation: fragmentation shows a large free total the allocator cannot
// place, and here the free total is simply gone.
//
// 648 sections against ~35 MB of growth puts a section at roughly 54 KB, square
// in the 20-200 KB range this port's own logs reported. TINY is 5 x 8 x 5 = 200
// sections, 31% of that, so the same content costs ~11 MB.
//
// This is a floor chosen to boot reliably, NOT a verdict on what the console can
// do. The render distance stays user-selectable -- PLATFORM_CONSOLE_LOW is 0, and
// platformGameSettingsCycleRenderDistance() on the Wii cycles NORMAL/SHORT/TINY,
// dropping only FAR -- and the gxlists= figure in the [WII][RAM] line remains the
// runtime measurement for display-list pressure.
//
// Wii starts at SHORT. At the ~54 KB per section the numbers above put a section
// at, NORMAL's 2312 sections are ~125 MB of display lists against an 11 MB live
// budget (PLATFORM_WII_GXLIST_EVICT_HIGH_WATER_BYTES in WiiFrameTuning.h), so what
// the extra distance actually buys is not terrain but eviction churn: the gate
// stays open permanently and sections are re-meshed as fast as they are dropped.
// SHORT's 648 are ~35 MB, still over that budget but far closer to a working set
// the gate can hold. NORMAL stays one step up the cycle.
#undef  PLATFORM_DEFAULT_RENDER_DISTANCE
#define PLATFORM_DEFAULT_RENDER_DISTANCE         2

// ofMipmapLevel the performance profile starts from. GX has native mipmaps
// (wii_native_texture_upload_level_rgba) and the 1 MB texture cache is what a
// 256x256 RGB5A3 atlas sampled at 2-4 texels per pixel misses; levels 1-2 are
// a third more texture RAM (every texture takes them, ~+45 KB per 256 atlas)
// for far terrain that both fetches less and stops shimmering. Sampling stays
// NEAR_MIP_NEAR unless ofMipmapLinear is switched on in the options.
#undef  PLATFORM_DEFAULT_MIPMAP_LEVEL
#define PLATFORM_DEFAULT_MIPMAP_LEVEL            0

// Moving vertical renderer window, the PS2's 5x3x5 idea at Wii proportions
// (Ps2CoreTuning.h). The renderer grid is the product of three numbers and the
// vertical one was the only one still paying for the whole world column: SHORT
// with all 8 sections is 9 x 8 x 9 = 648 slots, of which the bands more than a
// few chunks above or below the player are terrain the horizontal window cannot
// reach anyway.
//
// 5 sections is 80 blocks tall against SHORT's 72-block horizontal reach, so the
// window covers what the far plane can show and the clipped bands sit outside
// the frustum in ordinary play. 9 x 5 x 9 = 405 slots, 37% fewer sections to
// mesh, keep resident and evict against the 11 MB list budget. Standing on a
// peak looking down into a deep valley is where this shows: sections outside
// the window have no renderer, so that floor is not drawn until the window
// follows the player down. Drop to 4 if the [WII][RAM] gxlists= figure still
// sits at the eviction high water.
#undef  PLATFORM_VERTICAL_CHUNK_COUNT
#define PLATFORM_VERTICAL_CHUNK_COUNT            5
#undef  PLATFORM_CENTER_VERTICAL_RENDERERS
#define PLATFORM_CENTER_VERTICAL_RENDERERS       1

// Preload radius, in blocks, for Minecraft::preloadWorld.
//
// This was the single largest allocation in the port and it was never a
// decision: preloadWorld read 128 from a hardcoded #else branch that only the
// PS2 escaped, and its loop steps 16 blocks at a time over [-c, c] on both axes
// -- 17 x 17 = 289 chunk columns, each generated SYNCHRONOUSLY on the loading
// screen. That is ~23 MB before the first frame, plus whatever the light
// flood-fill pulls in from neighbours.
//
// Worse, it does not come back: the unloadAllChunks() immediately after cannot
// free any of it, because eviction requires a chunk to be both outside the
// unload radius and untouched for PLATFORM_MIN_UNUSED_TICKS_BEFORE_UNLOAD ticks,
// and these were all touched a moment ago. The first pass frees nothing, returns
// false, and the drain loop exits.
//
// Keep startup preload at 32 blocks (5 x 5 columns) even though SHORT is the
// default render distance. The remaining area streams incrementally after spawn
// instead of synchronously generating all 9 x 9 visible columns on the loading screen.
#undef  PLATFORM_PRELOAD_RADIUS_BLOCKS
#define PLATFORM_PRELOAD_RADIUS_BLOCKS 32

#undef PLATFORM_LIGHTING_UPDATES_PER_FRAME
#define PLATFORM_LIGHTING_UPDATES_PER_FRAME 256
// Interactive lighting burst; see PS2_LIGHTING_INTERACTIVE_QUEUE_MAX.
#undef  PLATFORM_LIGHTING_INTERACTIVE_QUEUE_MAX
#define PLATFORM_LIGHTING_INTERACTIVE_QUEUE_MAX 256
#undef  PLATFORM_LIGHTING_INTERACTIVE_BURST
#define PLATFORM_LIGHTING_INTERACTIVE_BURST     2048

// The job count above is the deterministic backstop; this is what actually
// bounds the frame. Same pairing, and the same reasoning, as the mesh budget's
// PLATFORM_MAX_RENDERER_UPDATES_PER_FRAME / PLATFORM_CHUNK_BUILD_BUDGET_MS: a
// count cannot bound a unit of work whose cost varies this much. Each of these
// 256 jobs is a light flood fill over a box, and the queue is fed hardest at
// exactly the moment the frame can least afford it -- when a streamed chunk is
// published.
//
// 2 ms of a 16.7 ms frame, sized against the mesher's 3 ms so the two together
// stay inside the frame alongside the tick and the draw. Unlike the mesh
// budget this one has no work-counter fallback of its own, because it does not
// need one: if the Wii timebase ever misbehaved, elapsed reads 0 and the 256
// count is still there underneath.
// 3 ms at WII_TARGET_FPS 30 (was 2 ms of a 16.7 ms frame): the per-frame
// budgets scale with the frame or the queue drains half as fast.
#undef  PLATFORM_LIGHTING_BUDGET_US
#define PLATFORM_LIGHTING_BUDGET_US 3000

// The desktop table leaves the light queue effectively unbounded (1,000,000
// jobs, 5-entry merge scan). Async generation publishes several columns per
// tick here, each enqueueing hundreds of nearly identical boxes, and the 2 ms
// budget above drains them far slower than they arrive, so the backlog is heap
// that grows for as long as the player keeps walking. Scan a wider tail before
// allocating a new box (the PS2 figure) and cap the queue: a job dropped at the
// cap is re-issued by the next nearby change once the backlog drains.
#undef  PLATFORM_LIGHTING_MERGE_SCAN
#define PLATFORM_LIGHTING_MERGE_SCAN            96
#undef  PLATFORM_LIGHTING_QUEUE_HARD_CAP
#define PLATFORM_LIGHTING_QUEUE_HARD_CAP        8192

// Entity simulation normally requires every chunk in a 32-block radius (5x5
// columns in the worst alignment). The Wii resident cache is deliberately
// smaller, so entities at its streaming edge could stop ticking altogether.
// One chunk keeps a useful safety margin without requiring the vanilla 5x5 set.
#undef  PLATFORM_PLAYER_UPDATE_CHUNK_RANGE_BLOCKS
#define PLATFORM_PLAYER_UPDATE_CHUNK_RANGE_BLOCKS 16


// Resident chunk cache, in chunks around the player.
//
// setChunkLoadRadiusFromRenderDistance() ignores the selected distance on a
// bounded world and pins this constant, so it has to cover the WIDEST distance
// the player can select, not the default one. WorldRenderer meshes through a
// ChunkCache with a one-block margin, so the rule is (visible radius + 1): a
// renderer outside the cache compiles an EmptyChunk and shows up as a black
// terrain hole.
//
// SHORT is 9 columns wide, radius 4, so 5. That is only sound because SHORT is
// also the ceiling now -- platformGameSettingsCycleRenderDistance() and
// platformGameSettingsClampRenderDistance() in GameSettingsBackend_WII.cpp keep
// the selection inside SHORT/TINY, the second of those so an options.txt written
// by an older build cannot load NORMAL into a cache that cannot serve it. Raising
// either of those means raising this back to 9 in the same commit.
//
// The unload radius is one chunk beyond, so walking a chunk boundary does not
// immediately delete and regenerate the ring behind you. The 13 x 13 source
// ceiling is about 13.5 MB at 80 KB per column, down from 29 MB at radius 9, and
// chunks are still created on demand; prefetch below follows the selected render
// distance instead of always filling this maximum radius.
#undef  PLATFORM_CHUNK_CACHE_RADIUS
#define PLATFORM_CHUNK_CACHE_RADIUS              5
#undef  PLATFORM_CHUNK_UNLOAD_RADIUS
#define PLATFORM_CHUNK_UNLOAD_RADIUS             6

// 3 (from 4): the per-chunk fixed cost of the random-tick sweep (cave-sound
// light probe, snow/ice findTopSolidBlock + biome lookup) runs on 49 columns
// instead of 81. Block ticks per world tick are set by
// PLATFORM_RANDOM_BLOCK_TICKS_PER_CHUNK below and are unchanged; what stops
// is simulation of the outer ring the player cannot see at SHORT.
#undef  PLATFORM_RANDOM_TICK_CHUNK_RADIUS
#define PLATFORM_RANDOM_TICK_CHUNK_RADIUS 3

// Spread the fixed per-chunk weather/cave work across ticks and reduce the
// random block-update rate from the desktop's 80 per resident chunk. With the
// observed 81-chunk working set, the desktop values issue 6480 random probes
// every world tick; 20 keeps crops/fluids responsive while cutting that to a
// quarter, and the round-robin preserves that chosen total rate.
#undef  PLATFORM_RANDOM_BLOCK_TICKS_PER_CHUNK
#define PLATFORM_RANDOM_BLOCK_TICKS_PER_CHUNK    10
#undef  PLATFORM_RANDOM_TICK_CHUNKS_PER_TICK
#define PLATFORM_RANDOM_TICK_CHUNKS_PER_TICK     9

// Entity CPU guardrails. Wii keeps a higher budget than PS2, but caps
// concurrent mobs and A* work tightly enough to avoid entity-driven tick
// spikes. Spawning remains paced every 4 ticks to avoid rescanning the full
// eligible area continuously.
// 10 mobs, 2 paths per tick of 160 nodes, a spawn pass every 8 ticks: each
// mob is a pathfinder plus block/entity collision scans per tick, and with
// the AI throttle below only the ones near the player run every tick anyway.
#undef  PLATFORM_MAX_LIVE_MOBS
#define PLATFORM_MAX_LIVE_MOBS                   10
#undef  PLATFORM_PATHFIND_BUDGET_PER_TICK
#define PLATFORM_PATHFIND_BUDGET_PER_TICK        2
#undef  PLATFORM_PATHFIND_MAX_NODES
#define PLATFORM_PATHFIND_MAX_NODES              160
#undef  PLATFORM_MOB_SPAWN_INTERVAL_TICKS
#define PLATFORM_MOB_SPAWN_INTERVAL_TICKS        8
// Spawn Y drawn within this band of the player instead of the whole column
// (see PS2_MOB_SPAWN_Y_BAND): with the cap at 10, a mob spawned deep in a
// cave the player never enters holds a slot for its whole life.
#undef  PLATFORM_MOB_SPAWN_Y_BAND
#define PLATFORM_MOB_SPAWN_Y_BAND                24

// End profile (see PS2_END_MAX_ENDERMEN for the reasoning). The End biome
// spawns endermen in groups of four, so without a cap of its own one pass
// fills the 10 live slots. Teleport trails are drawn here, so keep a short one
// instead of the 128-sample vanilla trail.
#undef  PLATFORM_END_MAX_ENDERMEN
#define PLATFORM_END_MAX_ENDERMEN                4
#undef  PLATFORM_ENDERMAN_TELEPORT_PARTICLES
#define PLATFORM_ENDERMAN_TELEPORT_PARTICLES     32
#undef  PLATFORM_DRAGON_COLLISION_TICK_DIVISOR
#define PLATFORM_DRAGON_COLLISION_TICK_DIVISOR   2

// Creature decision AI (target search, repath) rate-limited by distance to the
// nearest player, the PS2 scheme (Ps2WorldTuning.h) at Wii proportions:
// physics, timers and rendering still run every tick, only
// EntityCreature::updatePlayerActionState is staggered. NEAR is half the
// entity render radius below; a mob further than that is small on screen and
// its path work is not. Divisors are powers of two (EntityLiving masks them).
#undef  PLATFORM_ENTITY_AI_NEAR_RADIUS_BLOCKS
#define PLATFORM_ENTITY_AI_NEAR_RADIUS_BLOCKS    24.0f
#undef  PLATFORM_ENTITY_AI_FAR_RADIUS_BLOCKS
#define PLATFORM_ENTITY_AI_FAR_RADIUS_BLOCKS     40.0f
#undef  PLATFORM_ENTITY_AI_MID_TICK_DIVISOR
#define PLATFORM_ENTITY_AI_MID_TICK_DIVISOR      2
#undef  PLATFORM_ENTITY_AI_FAR_TICK_DIVISOR
#define PLATFORM_ENTITY_AI_FAR_TICK_DIVISOR      4

// Living entities beyond the terrain distance cull
// (PLATFORM_WII_DISTANCE_CULL_RADIUS) stand on terrain that is not drawn, so
// skip their animated-model submission. Frustum-exempt entities are unaffected.
#undef  PLATFORM_LIMIT_ENTITY_RENDER_DISTANCE
#define PLATFORM_LIMIT_ENTITY_RENDER_DISTANCE    1
#undef  PLATFORM_ENTITY_RENDER_RADIUS_BLOCKS
#define PLATFORM_ENTITY_RENDER_RADIUS_BLOCKS     48.0f

// Entity-entity push resolution is only observable near the player; beyond
// the render radius skip the chunk/AABB scan and keep everything else.
#undef  PLATFORM_LIMIT_ENTITY_PUSH_COLLISIONS
#define PLATFORM_LIMIT_ENTITY_PUSH_COLLISIONS    1
#undef  PLATFORM_ENTITY_PUSH_COLLISION_RADIUS_BLOCKS
#define PLATFORM_ENTITY_PUSH_COLLISION_RADIUS_BLOCKS 48.0f

// The Legacy PC / PS2 collision and entity-query fast paths. Same block
// collision semantics; per-block virtual World/Chunk lookups become direct
// resident-section reads, and unit cubes short-circuit before the pooled
// AxisAlignedBB path. Not arithmetic shortcuts, so they do not belong to
// PLATFORM_CONSOLE_LOW.
#undef  PLATFORM_FAST_BLOCK_COLLISIONS
#define PLATFORM_FAST_BLOCK_COLLISIONS           1
#undef  PLATFORM_EARLY_UNIT_CUBE_COLLISION_TEST
#define PLATFORM_EARLY_UNIT_CUBE_COLLISION_TEST  1
#undef  PLATFORM_EARLY_COLLISION_EXIT
#define PLATFORM_EARLY_COLLISION_EXIT            1
#undef  PLATFORM_ENTITY_QUERY_CACHE
#define PLATFORM_ENTITY_QUERY_CACHE              1
// Sized for the unload-radius ceiling above (13 x 13), so the chunk maps never
// rehash while the player crosses the edge of the SHORT working set.
#undef  PLATFORM_CHUNK_MAP_RESERVE
#define PLATFORM_CHUNK_MAP_RESERVE               169

// Compressed multiplayer columns sent outside the resident radius. Beta's
// protocol cannot request a chunk again, so dropping a full Packet51 payload
// creates permanent holes while walking on a server whose view-distance
// exceeds the Wii cache. Keep the original zlib payload until Packet50 unloads
// it. This value is a soft target used to report cache pressure; it must never
// trigger destructive eviction of a server-owned full chunk payload.
#undef PLATFORM_MP_DEFERRED_CHUNKS
#define PLATFORM_MP_DEFERRED_CHUNKS               1
// The bounded client chunk map is platform policy, independent of the
// deferred pipeline above: even a rolled-back profile still evicts live
// columns beyond PLATFORM_CHUNK_UNLOAD_RADIUS (WorldClient::
// trimClientChunkCache) or a server outgrows the Wii's RAM the same way it
// outgrew the 3DS heap.
#undef PLATFORM_MP_BOUNDED_CHUNK_CACHE
#define PLATFORM_MP_BOUNDED_CHUNK_CACHE           1
#undef PLATFORM_MP_COMPRESSED_CHUNK_CACHE_BYTES
#define PLATFORM_MP_COMPRESSED_CHUNK_CACHE_BYTES  (12u * 1024u * 1024u)
#undef PLATFORM_MP_CHUNK_PROMOTIONS_PER_TICK
#define PLATFORM_MP_CHUNK_PROMOTIONS_PER_TICK     2

// Eviction rate. Higher than the PS2's 2 because unloadChunk writes through
// libfat rather than a Memory Card, so draining a ring costs far less here, and
// because the cache has to keep up with a player who can cross chunk borders
// faster than the PS2 can generate them. Shorter grace period than the desktop's
// 200 ticks for the same reason: 3 seconds of standing still is long enough to be
// sure the player is not about to walk straight back.
#undef  PLATFORM_MAX_CHUNK_UNLOADS_PER_TICK
#define PLATFORM_MAX_CHUNK_UNLOADS_PER_TICK      2
#undef  PLATFORM_MIN_UNUSED_TICKS_BEFORE_UNLOAD
#define PLATFORM_MIN_UNUSED_TICKS_BEFORE_UNLOAD  60

// Synchronous terrain generation throttle.
//
// Without this, crossing into ungenerated territory makes one world tick
// generate the whole leading edge of the cache at once -- the multi-second
// stall, and a burst of allocator traffic large enough to matter on a heap this
// size.
//
// SYNC_RADIUS 0 guarantees only the column occupied by the player. The renderer
// generation gate requests adjacent columns ahead of time, under the normal
// per-tick budget. Radius 1 exempted all nine nearby columns from that budget,
// so several renderer candidates could each generate one in the same frame and
// recreate the very hitch this throttle is meant to prevent.
//
// CHUNKS_PER_TICK 1 streams the rest. Both are starting points chosen from the
// arithmetic, not from a measurement on hardware -- watch the world-tick time
// once there are numbers and move them together.
#undef  PLATFORM_GENERATE_SYNC_RADIUS
#define PLATFORM_GENERATE_SYNC_RADIUS            0
#undef  PLATFORM_GENERATE_CHUNKS_PER_TICK
#define PLATFORM_GENERATE_CHUNKS_PER_TICK        1

// Deferred decoration, 1 chunk per world tick.
//
// The count is the smaller half of this knob. The important half is that any
// non-zero value moves populate() out of prepareChunk() and onto a queue gated
// by canPopulateChunk(), which breaks the populate -> setBlockWithNotify ->
// provideChunk -> populate re-entrancy. On the inline path that chain can
// force-generate neighbours from inside a generation call, which is unbounded
// growth on a bounded cache.
//
// Decoration is also tunable by feature below; the queue controls when the
// monolithic call runs, while those counts control its worst-case duration.
#undef  PLATFORM_DEFERRED_POPULATE
#define PLATFORM_DEFERRED_POPULATE                1
// Vegetation and ores written at generation; see PS2_CHUNK_LOCAL_DECORATION.
// Runs on the main thread in finishAsyncChunkData, after the async terrain.
#undef  PLATFORM_CHUNK_LOCAL_DECORATION
#define PLATFORM_CHUNK_LOCAL_DECORATION           (WII_FAST_WORLDGEN ? 1 : 0)
#undef  PLATFORM_INCREMENTAL_POPULATE
#define PLATFORM_INCREMENTAL_POPULATE             1

#undef  PLATFORM_POPULATE_CHUNKS_PER_TICK
#define PLATFORM_POPULATE_CHUNKS_PER_TICK        1
// 48 steps / 2.5 ms (from 32 / 1.8 ms): a published column is not meshed
// until its decoration settles (PLATFORM_DEFER_MESH_DURING_POPULATE), so the
// populate drain is the first leg of the latency between a chunk arriving
// from the worker and the player seeing it. At WII_TARGET_FPS 30 the tick has
// the room; the budget still bounds the duration.
#undef  PLATFORM_POPULATE_STEPS_PER_TICK
#define PLATFORM_POPULATE_STEPS_PER_TICK         48

// Decoration budget for a tick that already published a column, instead of the
// stand-down that used to apply.
//
// The stand-down assumed publishing is rare. Once the async generator is fed at
// its queue limit rather than 2 requests per tick (see
// PLATFORM_WII_ASYNC_GENERATION_REQUESTS_PER_TICK in WiiStreamingTuning.h) a
// result is waiting on nearly every tick while the player explores, so the gate
// was closed permanently and decoration did not slow down, it stopped. Trees are
// what you notice, because BiomeDecorator's stage machine reaches them after the
// ores, clay and sand that are underground or unremarkable.
//
// A quarter of the full step budget. The 1800 us deadline in
// drainPendingPopulate() still applies on top, so this bounds the count while
// that bounds the duration -- the same pairing as the knob above. Set to 0 to
// restore the exact stand-down; nothing else changes.
#undef  PLATFORM_POPULATE_STEPS_AFTER_PUBLISH
#define PLATFORM_POPULATE_STEPS_AFTER_PUBLISH     8

#undef  PLATFORM_POPULATE_BUDGET_US
#define PLATFORM_POPULATE_BUDGET_US              2500
#undef  PLATFORM_POPULATE_SNOW_COLUMNS_PER_STEP
#define PLATFORM_POPULATE_SNOW_COLUMNS_PER_STEP  32

// Prevent generation, decoration and lighting from spending their independent
// limits together in the same rendered frame.
#undef  PLATFORM_STREAMING_FRAME_BUDGET_US
#define PLATFORM_STREAMING_FRAME_BUDGET_US       6000

// World-generation fidelity profile. Keep Java 1.2.5 decoration counts by
// default. Set WII_FAST_WORLDGEN=1 to reduce caves, filler veins, liquids,
// dungeons and the snow pass when frame-time is more important than seed parity.
// Deferred chunk generation/decoration remains enabled in both profiles; those
// scheduling changes preserve the generated content.
#ifndef WII_FAST_WORLDGEN
#define WII_FAST_WORLDGEN 1
#endif

// The vanilla 5x5 biome-shape blend divides by the same per-biome height
// denominator for every neighbour sample. Cache its reciprocal once per biome
// so chunk generation keeps the same 10x10 GenLayer halo and replaces those
// repeated divisions with multiplications.
#ifndef WII_FAST_BIOME_BLEND
#define WII_FAST_BIOME_BLEND 1
#endif
#undef  PLATFORM_FAST_BIOME_BLEND
#define PLATFORM_FAST_BIOME_BLEND                WII_FAST_BIOME_BLEND

// Noise biome field instead of the GenLayer chain, the profile the PS2 already
// runs (PS2_FAST_BIOME_SOURCE in Ps2WorldTuning.h).
//
// Stock 1.2.5 builds roughly 26 layers per chunk and walks all of them uncached,
// because the heightmap generator asks for a 20x20 halo while BiomeCache only
// serves aligned 16x16 tiles. WorldChunkManagerFast.cpp replaces that with three
// 2D octave fields on a four-block lattice: the same halo costs 36 evaluations
// per field instead of 400. On the Wii that work sits on the async generation
// worker, so it is the bulk of what makes a streamed column expensive.
//
// Structures are unaffected and stay enabled. Both biome consumers that place
// them -- areBiomesViable() and findBiomePosition() -- read coarseBiomeIdArea(),
// which routes to the same field the terrain uses, so a village or stronghold
// cannot land on a biome the terrain disagrees with. The field emits desert,
// forest, extremeHills, swampland, taiga, icePlains and jungle out of the twelve
// MapGenStronghold accepts, and both village biomes; MapGenStronghold also keeps
// its computed coordinates when findBiomePosition finds nothing.
//
// FLOAT_TERRAIN_NOISE is not optional here -- WorldChunkManagerFast.cpp #errors
// without it, because getBiomeGenForCoordsFloat only exists under that profile.
// It moves terrain noise to single precision, which is a second departure from
// Java arithmetic on top of the biome field.
//
// This CHANGES generated terrain: no seed parity with Java, and rivers and
// beaches disappear because those came from dedicated GenLayer layers. Beta
// 1.7.3 had no river biome either, so that is a move toward this port's
// reference rather than away from it. Saved chunks are not touched; test on a
// new world. Set to 0 to restore the GenLayer field.
#ifndef WII_FAST_BIOME_SOURCE
#define WII_FAST_BIOME_SOURCE 1
#endif
#undef  PLATFORM_FAST_BIOME_SOURCE
#define PLATFORM_FAST_BIOME_SOURCE               WII_FAST_BIOME_SOURCE
#undef  PLATFORM_FLOAT_TERRAIN_NOISE
#define PLATFORM_FLOAT_TERRAIN_NOISE             WII_FAST_BIOME_SOURCE

// Highest-frequency octaves dropped from the 3D terrain noise, which is the
// largest fixed cost of generateBaseTerrain: three NoiseGeneratorOctaves over
// the 5x17x5 field, 16 + 16 + 8 octave passes per chunk. Octave i contributes
// with weight 2^i, so the finest ones are almost free to lose: the first four
// of the density generators' sixteen sum to 15 of a 65535 range (0.02%), and
// the first two of the select generator's eight to 3 of 255 (1.2%). That takes
// the 40 passes to 30. The permutation tables are still drawn from rand, so
// the seed stream reaching later generators is untouched -- only the sampled
// value moves, by less than the float rounding PLATFORM_FLOAT_TERRAIN_NOISE
// already introduces. PS2_TERRAIN_DENSITY_OCTAVES cuts from the heavy end
// instead and does reshape the terrain; this deliberately does not.
#ifndef WII_TERRAIN_DENSITY_SKIP_FINE_OCTAVES
#define WII_TERRAIN_DENSITY_SKIP_FINE_OCTAVES 4
#endif
#ifndef WII_TERRAIN_SELECT_SKIP_FINE_OCTAVES
#define WII_TERRAIN_SELECT_SKIP_FINE_OCTAVES 2
#endif
#undef  PLATFORM_TERRAIN_DENSITY_SKIP_FINE_OCTAVES
#define PLATFORM_TERRAIN_DENSITY_SKIP_FINE_OCTAVES  WII_TERRAIN_DENSITY_SKIP_FINE_OCTAVES
#undef  PLATFORM_TERRAIN_SELECT_SKIP_FINE_OCTAVES
#define PLATFORM_TERRAIN_SELECT_SKIP_FINE_OCTAVES   WII_TERRAIN_SELECT_SKIP_FINE_OCTAVES

// Retention box for cached StructureStarts, half-extent in blocks around the
// chunk just generated.
//
// Vanilla's MapGenStructure keeps every start it ever created for the whole
// session, and generateStructuresInChunk() walks all of them on every populate.
// On a heap this size that is the wrong shape twice: a mineshaft start alone
// carries hundreds of corridor components, and the walk grows with distance
// travelled rather than with anything in view.
//
// Starts are seed-derived, so dropping one is not loss: the next 17x17 source
// sweep that reaches its chunk rebuilds it. What the radius buys is not paying
// that rebuild while the player is still nearby. A generated chunk sits at
// most PLATFORM_CHUNK_CACHE_RADIUS (5) from the player, populate can reach
// PLATFORM_CHUNK_UNLOAD_RADIUS (6) beyond the player and the sweep another 8
// beyond the target: (5 + 6 + 8) x 16 = 304 blocks. Rounded up so the box
// stays a chunk clear of that.
#undef  PLATFORM_STRUCTURE_START_RETENTION_BLOCKS
#define PLATFORM_STRUCTURE_START_RETENTION_BLOCKS   320

// Fluid UV rotation is consumed as float by RenderBlocks. Keep the global
// Vec3D/API layout unchanged and use the existing single-precision flow-angle
// path only for the Wii renderer hot path.

#ifndef WII_PRECOMPUTE_INITIAL_HEIGHTMAP
#define WII_PRECOMPUTE_INITIAL_HEIGHTMAP 1
#endif
#undef  PLATFORM_PRECOMPUTE_INITIAL_HEIGHTMAP
#define PLATFORM_PRECOMPUTE_INITIAL_HEIGHTMAP    WII_PRECOMPUTE_INITIAL_HEIGHTMAP

// Reuse the generated-section accounting performed while importing the column
// buffer. This avoids rescanning every 4 KB ExtendedBlockStorage immediately
// after the same blocks were copied into it.
#undef  PLATFORM_BULK_GENERATED_CHUNK_IMPORT
#define PLATFORM_BULK_GENERATED_CHUNK_IMPORT     1

// Population already pins its 2x2 chunk footprint. Route block/light lookups
// through those resident chunks and keep decoration writes inside the same
// fast path rather than resolving the provider for every leaf, grass or vein.
#undef  PLATFORM_POPULATION_BLOCK_WRITER
#define PLATFORM_POPULATION_BLOCK_WRITER         1
#undef  PLATFORM_FAST_LIGHTING_CHUNK_ACCESS
#define PLATFORM_FAST_LIGHTING_CHUNK_ACCESS      1

// Do not throw away a partially-built GX staging mesh when population dirties
// the same section. Finish the current build, publish it atomically, then queue
// one final rebuild. Distant published sections also wait for population to
// finish before starting that rebuild so repeated decoration edits coalesce.
#undef  PLATFORM_COALESCE_MESH_REBUILDS
#define PLATFORM_COALESCE_MESH_REBUILDS          1
#undef  PLATFORM_DEFER_MESH_DURING_POPULATE
#define PLATFORM_DEFER_MESH_DURING_POPULATE      1
#undef  PLATFORM_POPULATE_MESH_DEFER_DISTANCE_SQ
// See PS2_POPULATE_MESH_DEFER_DISTANCE_SQ: no reason to hold a first mesh for
// the structure-only populate once vegetation is decorated at generation.
#define PLATFORM_POPULATE_MESH_DEFER_DISTANCE_SQ (PLATFORM_CHUNK_LOCAL_DECORATION ? 1.0e12f : 1024.0f)

// Common opaque terrain dominates Wii section scans. Reject fully enclosed
// unit cubes before RenderBlocks and feed the remaining cubes a precomputed
// face mask so the six neighbour-opacity tests are not repeated downstream.
#undef  PLATFORM_SKIP_ENCLOSED_OPAQUE_CUBES
#define PLATFORM_SKIP_ENCLOSED_OPAQUE_CUBES      1
#undef  PLATFORM_FAST_SIMPLE_CUBE_RENDER
#define PLATFORM_FAST_SIMPLE_CUBE_RENDER         1

#ifndef WII_FLOAT_ENTITY_AI_MATH
#define WII_FLOAT_ENTITY_AI_MATH 1
#endif
#undef  PLATFORM_FLOAT_ENTITY_AI_MATH
#define PLATFORM_FLOAT_ENTITY_AI_MATH            WII_FLOAT_ENTITY_AI_MATH

#ifndef WII_FLOAT_ENTITY_CORE_MATH
#define WII_FLOAT_ENTITY_CORE_MATH 0
#endif
#undef  PLATFORM_FLOAT_ENTITY_CORE_MATH
#define PLATFORM_FLOAT_ENTITY_CORE_MATH          WII_FLOAT_ENTITY_CORE_MATH

#ifndef WII_FLOAT_EXPLOSION_MATH
#define WII_FLOAT_EXPLOSION_MATH 0
#endif
#undef  PLATFORM_FLOAT_EXPLOSION_MATH
#define PLATFORM_FLOAT_EXPLOSION_MATH            WII_FLOAT_EXPLOSION_MATH

#ifndef WII_FLOAT_FLUID_FLOW
#define WII_FLOAT_FLUID_FLOW 0
#endif
#undef  PLATFORM_FLOAT_FLUID_FLOW
#define PLATFORM_FLOAT_FLUID_FLOW                WII_FLOAT_FLUID_FLOW
// Drop search depth for spreading liquids; see PS2_FLUID_FLOW_SEARCH_DEPTH.
#undef  PLATFORM_FLUID_FLOW_SEARCH_DEPTH
#define PLATFORM_FLUID_FLOW_SEARCH_DEPTH         (WII_FAST_WORLDGEN ? 2 : 4)

#undef  PLATFORM_POPULATE_WATER_SPRINGS
#define PLATFORM_POPULATE_WATER_SPRINGS          (WII_FAST_WORLDGEN ? 8 : 50)
#undef  PLATFORM_POPULATE_LAVA_SPRINGS
#define PLATFORM_POPULATE_LAVA_SPRINGS           (WII_FAST_WORLDGEN ? 4 : 20)
#undef  PLATFORM_POPULATE_DUNGEONS
#define PLATFORM_POPULATE_DUNGEONS               (WII_FAST_WORLDGEN ? 2 : 8)
#undef  PLATFORM_POPULATE_SNOW_PASS
#define PLATFORM_POPULATE_SNOW_PASS              (WII_FAST_WORLDGEN ? 0 : 1)

// Vanilla populate work Pocket Edition never did, same rationale as the PS2
// table (Ps2WorldTuning.h): lake blobs and the 64 / 128 flower and grass
// scatter attempts per call. Worldgen animal groups stay on (gameplay).
#undef  PLATFORM_POPULATE_LAKES
#define PLATFORM_POPULATE_LAKES                  (WII_FAST_WORLDGEN ? 0 : 1)
#undef  PLATFORM_FLOWER_PLACEMENT_ATTEMPTS
#define PLATFORM_FLOWER_PLACEMENT_ATTEMPTS       (WII_FAST_WORLDGEN ? 16 : 64)
#undef  PLATFORM_TALL_GRASS_PLACEMENT_ATTEMPTS
#define PLATFORM_TALL_GRASS_PLACEMENT_ATTEMPTS   (WII_FAST_WORLDGEN ? 32 : 128)

// Lower canopy density in forests and jungles to reduce decoration and mesh work.
#undef  PLATFORM_POPULATE_TREES_PER_CHUNK_MAX
#define PLATFORM_POPULATE_TREES_PER_CHUNK_MAX    4

// Fast-worldgen keeps the default profile seed-compatible, but lets performance
// builds reduce the two largest remaining decoration/generation sweeps explicitly.
#ifndef WII_CAVE_SOURCE_RADIUS
#define WII_CAVE_SOURCE_RADIUS                   (WII_FAST_WORLDGEN ? 4 : 8)
#endif
#undef  PLATFORM_CAVE_SOURCE_RADIUS
#define PLATFORM_CAVE_SOURCE_RADIUS              WII_CAVE_SOURCE_RADIUS
// Ravines share that sweep for a 1-in-50 roll; Pocket Edition never had them.
#undef  PLATFORM_SKIP_RAVINE_GENERATION
#define PLATFORM_SKIP_RAVINE_GENERATION          (WII_FAST_WORLDGEN ? 1 : 0)

// Mineshafts, villages and strongholds stay on: MapGenStructure pins its own
// sweep to the vanilla radius of 8 (289 source columns per generator per chunk),
// but switching them off would also remove the stronghold and with it the End.
// Kept at the desktop default; see PS2_GENERATE_MAP_FEATURES for the trade-off.

#undef  PLATFORM_POPULATE_CLAY_VEINS
#define PLATFORM_POPULATE_CLAY_VEINS             (WII_FAST_WORLDGEN ? 4 : 10)
#undef  PLATFORM_POPULATE_DIRT_VEINS
#define PLATFORM_POPULATE_DIRT_VEINS             (WII_FAST_WORLDGEN ? 8 : 20)
#undef  PLATFORM_POPULATE_GRAVEL_VEINS
#define PLATFORM_POPULATE_GRAVEL_VEINS           (WII_FAST_WORLDGEN ? 4 : 10)

// Chunk meshing probes blocks and light values tens of thousands of times per
// section. Cache the already-resident ExtendedBlockStorage pointers once in
// ChunkCache instead of paying the Chunk virtual accessor on every probe.
#undef  PLATFORM_FAST_CHUNK_BLOCK_READS
#define PLATFORM_FAST_CHUNK_BLOCK_READS          1

// Initial skylight generation used to emit a render invalidation for every lit
// block. Batch those notifications per vertical section; light values and the
// subsequent skylight-occlusion work remain unchanged.
#undef  PLATFORM_BATCH_INITIAL_SKYLIGHT_RENDER_UPDATES
#define PLATFORM_BATCH_INITIAL_SKYLIGHT_RENDER_UPDATES 1

#undef  PLATFORM_BATCH_POPULATION_LIGHTING
#define PLATFORM_BATCH_POPULATION_LIGHTING       1
