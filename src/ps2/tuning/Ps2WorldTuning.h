#pragma once

#define PS2_SKIP_MOB_SPAWNING 0
#define PS2_RANDOM_TICK_CHUNK_RADIUS PS2_CHUNK_CACHE_RADIUS
#define PS2_RANDOM_BLOCK_TICKS_PER_CHUNK 8

// Resident chunks visited per world tick by updateBlocksAndPlayCaveSounds.
//
// That phase is the single worst spike in the port: the [PS2][FRAME] log
// reported "slowTick=randomBlocks" at 162.9 / 176.2 / 182.8 ms against a tick
// AVERAGE of 4-10ms, which is what turns a 30 fps stretch into a visible 4 fps
// stall. Radius 2 makes it sweep all 25 resident chunks every tick, and each
// chunk can hit findTopSolidBlock + getBiomeGenAt (the snow/ice branch) and 8
// updateTick calls, any of which can cascade through setBlockWithNotify.
//
// Round-robin instead: visit this many chunks per tick and carry a cursor across
// ticks, so the PER-CHUNK overhead (the cave-sound light probe, the snow
// branch's findTopSolidBlock + getBiomeGenAt, the chunk lookup) is divided by
// 25/N while every chunk is still reached.
//
// This is NOT a world-speed cut. updateBlocksAndPlayCaveSounds scales
// PS2_RANDOM_BLOCK_TICKS_PER_CHUNK by exactly the same 25/N factor, so the
// number of random block ticks issued per world tick is unchanged (25 chunks x
// 8 == 5 chunks x 40) and grass, crops, saplings, leaf decay and fluids run at
// the rate PS2_RANDOM_BLOCK_TICKS_PER_CHUNK alone decides. Only the fixed
// per-chunk cost is spread.
//
// If the profile line below shows ticks= is what spikes rather than snow=/sound=,
// this knob will not help and PS2_RANDOM_BLOCK_TICKS_PER_CHUNK is the one to
// lower -- that one does change world speed.
//
// 0 = visit every chunk every tick (previous console behaviour).
#define PS2_RANDOM_TICK_CHUNKS_PER_TICK 5

// Per-branch breakdown of that phase, printed every N world ticks:
//   sound = the ambient.cave probe (getFullBlockLightValue + getSavedLightValue)
//   snow  = the snow/ice branch (findTopSolidBlock + getBiomeGenAt + setBlock)
//   ticks = the PS2_RANDOM_BLOCK_TICKS_PER_CHUNK updateTick calls
// slowTick only ever names the whole phase, so it cannot say which of the three
// produced a 180ms tick. Each figure is the worst single tick in the window.
// 0 = compiled out.
#ifdef PS2_RENDER_STATS
#define PS2_RANDOM_TICK_PROFILE_INTERVAL 200
#else
#define PS2_RANDOM_TICK_PROFILE_INTERVAL 0
#endif

// Mob CPU budget. Mobs are the heaviest per-tick EE cost: each one runs an A*
// pathfinder (unbounded node expansion in vanilla) plus block/entity collision
// scans every tick, so a handful of mobs can drop the tick from ~5ms to whole
// frames. These knobs bound that cost:
//   - MAX_LIVE_MOBS:      hard cap on simultaneously alive spawned mobs.
//   - PATHFIND_BUDGET:    max A* path computations allowed per world tick
//                         (round-robin; mobs that miss their turn skip repathing
//                         this tick and retry next tick).
//   - PATHFIND_MAX_NODES: cap on A* node expansions per single path search; when
//                         hit, the best-so-far partial path is returned.
#define PS2_MAX_LIVE_MOBS 8

// Entity simulation profile. Physics, timers and the entity itself still tick
// every world tick; only creature AI is rate-limited outside the near-player
// radius. This keeps motion continuous while avoiding repeated target/path work
// for mobs at the edge of the 5x5 playable cache. Divisors are powers of two so
// the stagger is a cheap mask in EntityCreature.
#define PS2_ENTITY_AI_NEAR_RADIUS_BLOCKS 24.0f
#define PS2_ENTITY_AI_FAR_RADIUS_BLOCKS  40.0f
#define PS2_ENTITY_AI_MID_TICK_DIVISOR   2
#define PS2_ENTITY_AI_FAR_TICK_DIVISOR   4

// Entity-entity push resolution is only observable around the rendered player.
// Outside the 32-block visible radius, skip the chunk/AABB scan but continue all
// other movement, block collision and entity timers normally.
#define PS2_ENTITY_PUSH_COLLISION_RADIUS_BLOCKS 32.0f

// The R5900 has no hardware double-precision arithmetic. Entity positions and
// AABBs remain double for Java/world-coordinate fidelity, but local AI deltas,
// distances and angle calculations can narrow to float because they operate over
// tens of blocks and are immediately consumed as float rotations/speeds.
#define PS2_FLOAT_ENTITY_AI_MATH 1
#define PS2_FLOAT_ENTITY_CORE_MATH 1

// Flowing-fluid top-face UV rotation is consumed as float by RenderBlocks. Keep
// the Vec3D storage/API unchanged, but avoid the software-emulated double atan2
// on the EE after the local flow vector has already been normalized.
#define PS2_FLOAT_FLUID_FLOW 1

// Explosion rays only travel a few blocks from their origin, so keep their
// marching coordinates rebased around the integer origin block and run the hot
// normalization/step arithmetic in float. Entity/world positions remain double.
// Block-density sampling follows the same local-coordinate rule.
#define PS2_FLOAT_EXPLOSION_MATH 1

// Vec3D's scalar helpers (lengthVector, distanceTo, normalize) and the block ray
// cast in World::rayTraceBlocks. Both keep their double storage and their double
// signatures; only the arithmetic inside narrows.
//
// Vec3D holds world coordinates, so this is not the blanket "Vec3D is float"
// change: each helper is a length or a ratio, and both are scale-relative, so
// narrowing the operands costs relative precision rather than absolute
// precision. lengthVector and distanceTo already truncate to float on return
// (MathHelper::sqrt_double returns float), so for those two the narrowing only
// moves where the existing truncation happens.
//
// rayTraceBlocks is the case that pays best. It is a bounded march -- the picker
// gives it a five block reach and it caps at 200 steps -- so the ray is
// rebased on its own start point and every step runs in float. That also deletes
// the six std::isnan(double) probes per iteration: each is an __unorddf2 call on
// the EE, and the R5900 FPU cannot produce a NaN in the first place.
//
// Set to 0 to restore the exact double paths.
#define PS2_FLOAT_VECTOR_MATH 1

// Run the AABB sweep in Entity::moveEntity against a float copy of the candidate
// list, rebased on the entity's own bounding box, instead of comparing double
// world coordinates six times over.
//
// The entity position, its AxisAlignedBB and the pooled block boxes all stay
// double: this rebases them once per move into a local frame and resolves the
// three axes there. Precision is therefore relative to the entity rather than to
// the world origin, so it does not decay as the player walks away from spawn --
// the same property that makes RenderManager::renderPosX safe in the renderer.
//
// What it gives up: the resolved motion delta makes a round trip through float,
// so a resting entity can settle roughly 1e-6 of a block inside a surface
// instead of exactly on it. moveEntity re-resolves from the double bounding box
// every tick, so the error cannot accumulate.
#define PS2_FLOAT_COLLISION_SWEEP 1

#define PS2_PATHFIND_BUDGET_PER_TICK 2
#define PS2_PATHFIND_MAX_NODES 200
// Run the full SpawnerAnimals pass every N world ticks instead of every tick.
// The pass costs ~8-10 ms of EE time (3 creature types x eligible chunks x
// biome lookups + cluster probes) plus a burst of allocator traffic. Spawn
// density is attempt-based, so mobs still appear at a similar rate.
#define PS2_MOB_SPAWN_INTERVAL_TICKS 4

// Temporary PS2 visual profile: the vanilla lighting arrays can be incomplete
// while chunks are generated lazily, producing fully black terrain faces.
// Force terrain brightness until a cheaper/fixed skylight pipeline is added.
#define PS2_FORCE_FULLBRIGHT_TERRAIN 0

// World-generation performance profile. PS2 uses the lightweight heightmap and
// float hot paths by default, while retaining the vanilla cave source radius and
// decoration counts. Set PS2_FAST_WORLDGEN=1 to additionally reduce the cave
// source sweep and decoration work. Every fast path trades seed parity for speed,
// and individual knobs can still be overridden independently.
#ifndef PS2_FAST_WORLDGEN
#define PS2_FAST_WORLDGEN 1
#endif

// Cave carving is one of the biggest chunk-generation costs. Java 1.2.5
// scans a source radius of 8 (17x17 chunks) around every target chunk. Keep
// that value as the default so cave origins and cross-chunk branches remain
// deterministic with the desktop port. The old radius-3 PS2 profile remains
// available as an explicit build-time optimization, for example
// -DPS2_CAVE_SOURCE_RADIUS=3, when performance is preferred over seed parity.
//
// The cave geometry can still use float on PS2 because the R5900 has no
// hardware double-precision arithmetic. This can move a rounded cave edge by
// a block, but it no longer silently drops distant vanilla cave origins.
#define PS2_SKIP_CAVE_GENERATION 0
#ifndef PS2_CAVE_SOURCE_RADIUS
#  if PS2_FAST_WORLDGEN
#    define PS2_CAVE_SOURCE_RADIUS 3
#  else
#    define PS2_CAVE_SOURCE_RADIUS 8
#  endif
#endif
#define PS2_CAVE_RARITY 15
#ifndef PS2_FLOAT_CAVE_GENERATION
#  define PS2_FLOAT_CAVE_GENERATION 1
#endif


// Lightweight 2D heightmap terrain generator (ChunkProviderGenerateLite.cpp).
// Replaces the vanilla 3D double-precision density field with a single all-float
// noise column per block. The EE has no hardware double FPU, so the vanilla
// 8/16-octave double Perlin sweeps are software-emulated and are the dominant
// cost of the multi-second stall when crossing a chunk border; the heightmap
// path is roughly an order of magnitude cheaper. Trade-off: no overhangs,
// floating islands or noise caves (caves are below the visible window anyway).
// Set to 0 to fall back to the vanilla 3D generator.
#ifndef PS2_USE_HEIGHTMAP_TERRAIN
#  define PS2_USE_HEIGHTMAP_TERRAIN 1
#endif
// Surface replacement for the PS2 heightmap path. The vanilla routine scans
// all 128 Y values and draws bedrock RNG at every level for every column. The
// heightmap already knows the solid top, so the fast path touches only the
// surface/filler band plus Y 0..4 bedrock. This changes the per-chunk surface
// RNG stream, which is acceptable under PS2_FAST_WORLDGEN.
#ifndef PS2_FAST_SURFACE_PASS
#  define PS2_FAST_SURFACE_PASS (PS2_FAST_WORLDGEN && PS2_USE_HEIGHTMAP_TERRAIN)
#endif
// The legacy block buffer is already scanned once while Chunk converts it into
// ExtendedBlockStorage. Cache the final opaque-column heights during that pass
// so the initial skylight build does not scan every column a second time.
#ifndef PS2_PRECOMPUTE_INITIAL_HEIGHTMAP
#  define PS2_PRECOMPUTE_INITIAL_HEIGHTMAP PS2_FAST_WORLDGEN
#endif
// Collapse the thousands of per-block renderer invalidations emitted while
// initial skylight is filled into at most one dirty range per section.
#ifndef PS2_BATCH_INITIAL_SKYLIGHT_RENDER_UPDATES
#  define PS2_BATCH_INITIAL_SKYLIGHT_RENDER_UPDATES 1
#endif
// Y at/below which empty columns fill with water in the optional fast
// heightmap generator. Release 1.2.5 uses sea level 63.
#define PS2_HEIGHTMAP_SEA_LEVEL 63
// Average land height the noise oscillates around.
#define PS2_HEIGHTMAP_BASE_HEIGHT 64
// Peak +/- swing of the broad continental noise, in blocks. 28 is a modest
// lift over the old 24-block profile: more hills without turning every biome
// into mountains or materially increasing the generator's noise cost.
#define PS2_HEIGHTMAP_AMPLITUDE 28
// Surface water freezes to ice when the column's biome temperature is below this.
// The fast heightmap path uses this approximate freeze threshold; the parity path
// uses the normal Java 1.2.5 biome/freezing rules. Float biome noise can nudge
// temperate columns across the threshold, so 0.35 keeps cold biomes frozen without
// the spurious patches. Set to a negative value to disable freezing entirely.
#define PS2_HEIGHTMAP_FREEZE_TEMP 0.35f

// Chunk decoration (populate) budget. populate() runs synchronously for up to
// 4 chunks in a single worldTick when crossing a chunk border, and the liquid
// spring passes are the worst offenders: vanilla scatters 50 water + 20 lava
// sources per chunk, each placed with setBlockWithNotify -> neighbour block
// updates and fluid propagation. 70 fluid foci x4 chunks = thousands of chained
// updates in one tick -- the dominant worldTick stall (and a deep update chain
// that can smash the small EE stack). These cap the per-chunk counts on console.
// 0 disables a pass entirely. PC keeps vanilla counts.
#ifndef PS2_POPULATE_WATER_SPRINGS
#  define PS2_POPULATE_WATER_SPRINGS (PS2_FAST_WORLDGEN ? 4 : 50)
#endif
#ifndef PS2_POPULATE_LAVA_SPRINGS
#  define PS2_POPULATE_LAVA_SPRINGS (PS2_FAST_WORLDGEN ? 2 : 20)
#endif
#ifndef PS2_POPULATE_DUNGEONS
#  define PS2_POPULATE_DUNGEONS (PS2_FAST_WORLDGEN ? 2 : 8)
#endif
#ifndef PS2_POPULATE_TREE_BONUS
#  define PS2_POPULATE_TREE_BONUS (PS2_FAST_WORLDGEN ? 2 : 0)
#endif
// These three passes replace stone with large 32-block filler pockets. They do
// not affect the actual ore economy, but vanilla runs 40 of them per populated
// chunk and every changed underground section is then fingerprinted, relit and
// remeshed. Keep a little variation while cutting that streaming-only work by
// 80% (8 attempts instead of 40). PC retains the vanilla counts below.
#ifndef PS2_POPULATE_CLAY_VEINS
#  define PS2_POPULATE_CLAY_VEINS (PS2_FAST_WORLDGEN ? 2 : 10)
#endif
#ifndef PS2_POPULATE_DIRT_VEINS
#  define PS2_POPULATE_DIRT_VEINS (PS2_FAST_WORLDGEN ? 4 : 20)
#endif
#ifndef PS2_POPULATE_GRAVEL_VEINS
#  define PS2_POPULATE_GRAVEL_VEINS (PS2_FAST_WORLDGEN ? 2 : 10)
#endif
// Surface snow pass scans 16x16 columns doing findTopSolidBlock + setBlockWithNotify.
// Cheap-ish but pure CPU with no gameplay value on console. 0 = skip it.
#ifndef PS2_POPULATE_SNOW_PASS
#  define PS2_POPULATE_SNOW_PASS (PS2_FAST_WORLDGEN ? 0 : 1)
#endif

// Light propagation is fed most heavily by the same newly populated chunks.
// A job count alone is not a frame-time bound because each MetadataChunkBlock
// covers a different-sized box. Check both: 128 jobs is the deterministic
// backstop, while 2.5 ms yields the queue between jobs on the normal render
// path. A single job remains atomic, so this cannot leave a half-updated box.
#define PS2_LIGHTING_UPDATES_PER_FRAME 128 // vanilla 500
#define PS2_LIGHTING_BUDGET_US         2500
// Vanilla only probes the five newest jobs for overlap. Chunk generation on PS2
// can enqueue tens of thousands of nearly-identical light boxes, so scan a
// wider tail before allocating another MetadataChunkBlock. The hard cap prevents pathological propagation storms from consuming the
// remaining heap; recent-neighbour merging catches the common duplicates first.
#define PS2_LIGHTING_MERGE_SCAN        96
#define PS2_LIGHTING_QUEUE_HARD_CAP    12288

// Deferred decoration. Vanilla populates up to 4 chunks *synchronously* inside the
// generation call (prepareChunk), on whatever frame first touches a new chunk --
// that is the worldTick spike when crossing a border. Worse, populate() places
// blocks with setBlockWithNotify, which re-enters provideChunk/prepareChunk and
// can recurse into more populate() calls (deep chain that can smash the small EE
// stack). On console we instead enqueue ready chunks and decorate this many per
// world tick, off the critical frame. This also breaks the re-entrancy entirely.
//
// Was DISABLED (0) at 32 MB: per-frame [PS2][FRAME] profiling proved the
// "chunkUnload" phase (= drainPendingPopulate -> populate) was the dominant tick
// cost (~400 ms EVERY tick) AND the RAM leak that drove a second OOM: vanilla
// populate() runs ~110 ore/dirt/gravel vein passes + lakes + dungeons + trees
// per chunk, and its +8-block decoration reached into neighbour chunks,
// force-generating them (chunk growth -> OOM).
//
// RE-ENABLED (1) trial: two things changed since. The Tessellator shrink freed
// ~7.5 MB of headroom, and the deferred queue now gates on canPopulateChunk()
// (+x/+z/diagonal neighbours must already exist), so decoration can no longer
// force-generate chunks — the mechanism behind the old leak. Watch slowTick/
// mallocUsed in the [PS2][FRAME] log; drop back to 0 (bare terrain) or switch
// to a trees-only populate if the tick regresses.
#define PS2_POPULATE_CHUNKS_PER_TICK 1
// Incremental PS2 populate advances individual generator operations rather than
// whole chunks. Allow several cheap operations in one tick, but stop after the
// time budget once an atomic generator returns. Heavy generators such as lakes
// remain atomic and therefore can exceed the budget by themselves.
#define PS2_POPULATE_STEPS_PER_TICK 8
#define PS2_POPULATE_BUDGET_US      4000

// Deferred terrain GENERATION. The decompiled provider generates every requested
// chunk *synchronously*, so when the player crosses into ungenerated territory
// the physics/entity/render code force-generates the whole 5x5 cache area inside
// a single worldTick -- the multi-second stall, and the EE stack/heap pressure
// behind the wild-jump / "Syscall undefined" crashes. These throttle it:
//   - GENERATE_SYNC_RADIUS:     chunks within this Chebyshev radius of the player's
//                               chunk always generate immediately, so the player
//                               never stands on (or falls through) un-generated air.
//                               1 = the 3x3 the player stands and collides in.
//                               0 = ONLY the chunk the player is in is forced
//                               synchronous; the surrounding ring streams at
//                               GENERATE_CHUNKS_PER_TICK. This is the key knob for
//                               the ~1s border-cross freeze: at radius 1 the whole
//                               new leading row (3 chunks) generates in one tick; at
//                               radius 0 the chunk being entered already exists from
//                               the previous frame's 3x3, so a normal walk generates
//                               0 sync chunks and the freeze disappears. Trade-off:
//                               sprinting into un-generated land briefly shows a bare
//                               edge until it streams in (no fall -- collision still
//                               force-generates the chunk under the player).
//   - GENERATE_CHUNKS_PER_TICK: max *non-critical* chunks generated per world tick.
//                               The outer cache ring streams in this many per tick;
//                               over-budget requests get the blank (air) chunk this
//                               tick and are retried next tick. 0 = no throttle
//                               (vanilla synchronous behaviour).
#define PS2_GENERATE_SYNC_RADIUS 0
#define PS2_GENERATE_CHUNKS_PER_TICK 1

// All-float biome noise. The biome temperature/humidity field
// a 2D simplex) is sampled ~3000 times per generated chunk, entirely in `double`
// -- software-emulated on the EE. Those values only feed threshold comparisons
// and a smoothstep, so float precision is ample. This runs that simplex in float.
// Trade-off: float rounding slightly shifts biome boundaries vs the double path,
// i.e. it CHANGES the generated world. Set to 0 to fall back to the exact double
// biome noise (useful to isolate any world-gen regression from the generation
// throttle above). PC always uses the double path.
#ifndef PS2_FLOAT_BIOME_NOISE
#  define PS2_FLOAT_BIOME_NOISE 1
#endif

// All-float ore/dirt/gravel/clay vein placement (WorldGenMinable). Measured:
// the "chunkUnload" tick phase — which is really drainPendingPopulate ->
// populate(), not chunk unloading — showed 428-514 ms spikes in the
// [PS2][FRAME] log on the frames where a chunk decorated. populate() runs ~92
// veins per chunk (10 clay + 20 dirt + 10 gravel + 20 coal + 20 iron + 2 gold +
// 8 redstone + 1 diamond + 1 lapis), and WorldGenMinable::generate was written
// entirely in `double`: 33 interpolation steps, each sweeping a ~5x5x5 box with
// several doubles per cell. The EE has no hardware double FPU, so that is a few
// million software-emulated operations per chunk.
//
// The values only feed an ellipsoid test against 1.0 and a floor(), so float is
// ample. RNG calls are untouched (same calls, same order), so vein *placement*
// still follows the world seed; only a vein's rounded edge can shift by a block.
// Ore counts and depths are unchanged — nothing becomes unmineable.
//
// Trade-off: like PS2_FLOAT_BIOME_NOISE, this CHANGES generated vein shapes very
// slightly versus the double path. Set to 0 for bit-exact vanilla arithmetic
// (useful to isolate a world-gen regression); the 0 path is bit-identical to the
// original, the hoisting done alongside this is exact.
#ifndef PS2_FLOAT_ORE_VEINS
#  define PS2_FLOAT_ORE_VEINS 1
#endif

// Octave counts for ChunkProviderGenerate's main terrain-shape noise. Vanilla
// is 16 octaves for the high/low density fields (field_912_k, field_911_l) and
// 8 for the blend selector (field_910_m); measured, that is ~17,000 Perlin
// samples per chunk (a 5x17x5 grid x 40 combined octaves), already in float
// (PLATFORM_CONSOLE_LOW) and already reusing the corner interpolation across
// unchanged Y cells -- the "generate" frame-log figure this trims is close to
// the algorithmic floor of that approach, not a leftover double or a missed
// cache. Each dropped octave is a halving of that frequency's contribution, so
// the terrain gets very slightly smoother/less detailed at the highest
// frequencies; the low-frequency shape (the part a player actually reads as
// "terrain") is unchanged. This does NOT preserve vanilla terrain for a given
// seed -- unlike PS2_FLOAT_BIOME_NOISE/PS2_FLOAT_ORE_VEINS above, which only
// round differently, fewer octaves is fewer octaves. Set both back to 16/8 for
// bit-for-bit vanilla shape at the original CPU cost.
#ifndef PS2_TERRAIN_DENSITY_OCTAVES
#  define PS2_TERRAIN_DENSITY_OCTAVES 12
#endif
#ifndef PS2_TERRAIN_SELECT_OCTAVES
#  define PS2_TERRAIN_SELECT_OCTAVES 6
#endif

// Size of MathHelper's sine lookup table, as a power of two. Vanilla is 16, i.e.
// 65536 entries, and on this console that is the wrong shape twice over.
//
// It is 256 KB of a 32 MB budget, sitting permanently in .bss. And the R5900 has
// an 8 KB data cache, so a table 32x larger than the whole cache turns every
// MathHelper::sin/cos into a guaranteed miss at main-memory latency -- the index
// is an angle, so consecutive calls land nowhere near each other and there is no
// locality to recover. Entity movement, mob AI, particles and every rotated
// model call these constantly.
//
// 12 is 4096 entries = 16 KB: 16x smaller than the D-cache pressure it used to
// apply, and 240 KB returned to the heap.
//
// Trade-off, and it is a real one: the angular step goes from 2*pi/65536 to
// 2*pi/4096, so sin/cos gain a worst-case error of ~0.0015 (0.09 degrees). That
// is invisible in a rendered rotation but it does mean entity motion no longer
// matches the double-precision Java path exactly. Set to 16 to restore the
// vanilla table; the desktop build always uses 16.
//
// The alternative not taken: 256 entries plus linear interpolation would be 1 KB
// (permanently cache-resident) AND more accurate than vanilla, at the cost of a
// second load and a multiply-add inside sin(). Worth measuring if this knob
// proves to matter.
