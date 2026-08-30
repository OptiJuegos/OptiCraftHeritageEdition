#pragma once

// -----------------------------------------------------------------------------
// Generic low-console tuning aliases
// -----------------------------------------------------------------------------
// The game code should prefer these PLATFORM_* names for world/simulation/render
// budgets.  PS2-specific files may still use PS2_* directly.

#if PLATFORM_PS2
#  define PLATFORM_DEFAULT_RENDER_DISTANCE              PS2_DEFAULT_RENDER_DISTANCE
#  define PLATFORM_VISIBLE_CHUNK_RADIUS                 PS2_VISIBLE_CHUNK_RADIUS
#  define PLATFORM_VISIBLE_CHUNK_DIAMETER               PS2_VISIBLE_CHUNK_DIAMETER
#  define PLATFORM_VERTICAL_CHUNK_COUNT                 PS2_VERTICAL_CHUNK_COUNT
#  define PLATFORM_CENTER_VERTICAL_RENDERERS            PS2_CENTER_VERTICAL_RENDERERS
#  define PLATFORM_CHUNK_CACHE_RADIUS                   PS2_CHUNK_CACHE_RADIUS
#  define PLATFORM_CHUNK_UNLOAD_RADIUS                  PS2_CHUNK_UNLOAD_RADIUS
#  define PLATFORM_CHUNK_MAP_RESERVE                    PS2_CHUNK_MAP_RESERVE
#  define PLATFORM_MAX_CHUNK_UNLOADS_PER_TICK           PS2_MAX_CHUNK_UNLOADS_PER_TICK
#  define PLATFORM_EMERGENCY_CHUNK_UNLOADS_PER_TICK     PS2_EMERGENCY_CHUNK_UNLOADS_PER_TICK
#  define PLATFORM_MIN_UNUSED_TICKS_BEFORE_UNLOAD       PS2_MIN_UNUSED_TICKS_BEFORE_UNLOAD
#  define PLATFORM_PRELOAD_RADIUS_BLOCKS                PS2_PRELOAD_RADIUS_BLOCKS
#  define PLATFORM_PLAYER_UPDATE_CHUNK_RANGE_BLOCKS     PS2_PLAYER_UPDATE_CHUNK_RANGE_BLOCKS
#  define PLATFORM_DISABLE_RUNTIME_AUTOSAVE             PS2_DISABLE_RUNTIME_AUTOSAVE
#  define PLATFORM_RUNTIME_AUTOSAVE_LEVEL_DATA          0
#  define PLATFORM_SKIP_NEW_WORLD_FULL_SAVE             PS2_SKIP_NEW_WORLD_FULL_SAVE
// Irrelevant on PS2 -- runtime autosave is off entirely there -- but the field
// still has to be initialised to something, so it takes the vanilla value.
#  define PLATFORM_AUTOSAVE_PERIOD_TICKS                40
// Runtime incremental saves stay small on PS2 to avoid long synchronous I/O stalls.
#  define PLATFORM_INCREMENTAL_CHUNK_SAVE_LIMIT         2
#  define PLATFORM_MESH_BUDGET                          1
#  define PLATFORM_MIN_RENDERER_UPDATES_PER_FRAME       PS2_MIN_RENDERER_UPDATES_PER_FRAME
#  define PLATFORM_MAX_RENDERER_UPDATES_PER_FRAME       PS2_MAX_RENDERER_UPDATES_PER_FRAME
#  define PLATFORM_MAX_RENDERED_SECTIONS_PER_PASS       PS2_MAX_RENDERED_SECTIONS_PER_PASS
#  define PLATFORM_CHUNK_BUILD_BLOCKS_PER_STEP          PS2_CHUNK_BUILD_BLOCKS_PER_STEP
#  define PLATFORM_RENDERER_UPDATE_CANDIDATES_PER_FRAME PS2_RENDERER_UPDATE_CANDIDATES_PER_FRAME
#  define PLATFORM_CULL_MISSING_CHUNK_BOUNDARY_FACES     PS2_CULL_MISSING_CHUNK_BOUNDARY_FACES
#  define PLATFORM_CHUNK_BUILD_BUDGET_MS                PS2_CHUNK_BUILD_BUDGET_MS
#  define PLATFORM_CHUNK_BUILD_STEP_US                  PS2_CHUNK_BUILD_STEP_US
#  define PLATFORM_CHUNK_BUILD_TIME_CHECK_BLOCKS        PS2_CHUNK_BUILD_TIME_CHECK_BLOCKS
#  define PLATFORM_MESH_FACE_SORT                       PS2_MESH_FACE_SORT
#  define PLATFORM_FACE_BUCKET_CULL                     PS2_FACE_BUCKET_CULL
#  define PLATFORM_FACE_CULL_EYE_MARGIN                 PS2_FACE_CULL_EYE_MARGIN
#  define PLATFORM_MESH_SORT_MAX_QUADS                  PS2_MESH_SORT_MAX_QUADS
#  define PLATFORM_ENABLE_CHUNK_PREFETCH                PS2_ENABLE_CHUNK_PREFETCH
#  define PLATFORM_PREFETCH_CHUNK_INTERVAL_FRAMES       PS2_PREFETCH_CHUNK_INTERVAL_FRAMES
#  define PLATFORM_PREFETCH_CHUNKS_PER_STEP             PS2_PREFETCH_CHUNKS_PER_STEP
#  define PLATFORM_SKIP_TRANSPARENT_WORLD_PASS          PS2_SKIP_TRANSPARENT_WORLD_PASS
#  define PLATFORM_ENABLE_GREEDY_MESH                   PS2_ENABLE_GREEDY_MESH
#  define PLATFORM_SKIP_ENTITY_SHADOWS                  PS2_SKIP_ENTITY_SHADOWS
#  define PLATFORM_SKIP_WORLD_ENTITIES                  PS2_SKIP_WORLD_ENTITIES
#  define PLATFORM_SKIP_WORLD_PARTICLES                 PS2_SKIP_WORLD_PARTICLES
#  define PLATFORM_SKIP_RAIN_SNOW                       PS2_SKIP_RAIN_SNOW
#  define PLATFORM_SKIP_CLOUDS                          PS2_SKIP_CLOUDS
#  define PLATFORM_SKIP_BLOCK_SELECTION_BOX             PS2_SKIP_BLOCK_SELECTION_BOX
#  define PLATFORM_ENABLE_BLOCK_RAYTRACE                PS2_ENABLE_BLOCK_RAYTRACE
#  define PLATFORM_ENABLE_BLOCK_BREAK_OVERLAY           PS2_ENABLE_BLOCK_BREAK_OVERLAY
#  define PLATFORM_SKIP_HAND_RENDER                     PS2_SKIP_HAND_RENDER
#  define PLATFORM_SKIP_CAMERA_FX                       PS2_SKIP_CAMERA_FX
#  define PLATFORM_SKIP_MOB_SPAWNING                    PS2_SKIP_MOB_SPAWNING
#  define PLATFORM_RANDOM_TICK_CHUNK_RADIUS             PS2_RANDOM_TICK_CHUNK_RADIUS
#  define PLATFORM_RANDOM_BLOCK_TICKS_PER_CHUNK         PS2_RANDOM_BLOCK_TICKS_PER_CHUNK
#  define PLATFORM_RANDOM_TICK_CHUNKS_PER_TICK          PS2_RANDOM_TICK_CHUNKS_PER_TICK
#  define PLATFORM_RANDOM_TICK_PROFILE_INTERVAL         PS2_RANDOM_TICK_PROFILE_INTERVAL
#  define PLATFORM_MAX_LIVE_MOBS                        PS2_MAX_LIVE_MOBS
#  define PLATFORM_ENTITY_AI_NEAR_RADIUS_BLOCKS          PS2_ENTITY_AI_NEAR_RADIUS_BLOCKS
#  define PLATFORM_ENTITY_AI_FAR_RADIUS_BLOCKS           PS2_ENTITY_AI_FAR_RADIUS_BLOCKS
#  define PLATFORM_ENTITY_AI_MID_TICK_DIVISOR            PS2_ENTITY_AI_MID_TICK_DIVISOR
#  define PLATFORM_ENTITY_AI_FAR_TICK_DIVISOR            PS2_ENTITY_AI_FAR_TICK_DIVISOR
#  define PLATFORM_ENTITY_PUSH_COLLISION_RADIUS_BLOCKS   PS2_ENTITY_PUSH_COLLISION_RADIUS_BLOCKS
#  define PLATFORM_FLOAT_ENTITY_AI_MATH                  PS2_FLOAT_ENTITY_AI_MATH
#  define PLATFORM_FLOAT_ENTITY_CORE_MATH                PS2_FLOAT_ENTITY_CORE_MATH
#  define PLATFORM_FLOAT_FLUID_FLOW                       PS2_FLOAT_FLUID_FLOW
#  define PLATFORM_FAST_BIOME_BLEND                       0
#  define PLATFORM_FLOAT_EXPLOSION_MATH                   PS2_FLOAT_EXPLOSION_MATH
#  define PLATFORM_FLOAT_VECTOR_MATH                     PS2_FLOAT_VECTOR_MATH
#  define PLATFORM_FLOAT_COLLISION_SWEEP                 PS2_FLOAT_COLLISION_SWEEP
#  define PLATFORM_PATHFIND_BUDGET_PER_TICK             PS2_PATHFIND_BUDGET_PER_TICK
#  define PLATFORM_PATHFIND_MAX_NODES                   PS2_PATHFIND_MAX_NODES
#  define PLATFORM_MOB_SPAWN_INTERVAL_TICKS             PS2_MOB_SPAWN_INTERVAL_TICKS
#  define PLATFORM_FORCE_FULLBRIGHT_TERRAIN             PS2_FORCE_FULLBRIGHT_TERRAIN
#  define PLATFORM_SKIP_CAVE_GENERATION                 PS2_SKIP_CAVE_GENERATION
#  define PLATFORM_CAVE_SOURCE_RADIUS                   PS2_CAVE_SOURCE_RADIUS
#  define PLATFORM_CAVE_RARITY                          PS2_CAVE_RARITY
#  define PLATFORM_FLOAT_CAVE_GENERATION                PS2_FLOAT_CAVE_GENERATION
#  define PLATFORM_USE_HEIGHTMAP_TERRAIN                PS2_USE_HEIGHTMAP_TERRAIN
#  define PLATFORM_FAST_SURFACE_PASS                    PS2_FAST_SURFACE_PASS
#  define PLATFORM_PRECOMPUTE_INITIAL_HEIGHTMAP          PS2_PRECOMPUTE_INITIAL_HEIGHTMAP
#  define PLATFORM_BATCH_INITIAL_SKYLIGHT_RENDER_UPDATES PS2_BATCH_INITIAL_SKYLIGHT_RENDER_UPDATES
#  define PLATFORM_HEIGHTMAP_SEA_LEVEL                  PS2_HEIGHTMAP_SEA_LEVEL
#  define PLATFORM_HEIGHTMAP_BASE_HEIGHT                PS2_HEIGHTMAP_BASE_HEIGHT
#  define PLATFORM_HEIGHTMAP_AMPLITUDE                  PS2_HEIGHTMAP_AMPLITUDE
#  define PLATFORM_HEIGHTMAP_FREEZE_TEMP                PS2_HEIGHTMAP_FREEZE_TEMP
#  define PLATFORM_POPULATE_WATER_SPRINGS              PS2_POPULATE_WATER_SPRINGS
#  define PLATFORM_POPULATE_LAVA_SPRINGS               PS2_POPULATE_LAVA_SPRINGS
#  define PLATFORM_POPULATE_DUNGEONS                   PS2_POPULATE_DUNGEONS
#  define PLATFORM_POPULATE_TREE_BONUS                  PS2_POPULATE_TREE_BONUS
#  define PLATFORM_POPULATE_CLAY_VEINS                 PS2_POPULATE_CLAY_VEINS
#  define PLATFORM_POPULATE_DIRT_VEINS                 PS2_POPULATE_DIRT_VEINS
#  define PLATFORM_POPULATE_GRAVEL_VEINS               PS2_POPULATE_GRAVEL_VEINS
#  define PLATFORM_POPULATE_SNOW_PASS                  PS2_POPULATE_SNOW_PASS
#  define PLATFORM_INCREMENTAL_POPULATE               1
#  define PLATFORM_POPULATE_CHUNKS_PER_TICK            PS2_POPULATE_CHUNKS_PER_TICK
#  define PLATFORM_POPULATE_STEPS_PER_TICK             PS2_POPULATE_STEPS_PER_TICK
#  define PLATFORM_POPULATE_BUDGET_US                  PS2_POPULATE_BUDGET_US
#  define PLATFORM_POPULATE_SNOW_COLUMNS_PER_STEP      16
#  define PLATFORM_GENERATE_SYNC_RADIUS                PS2_GENERATE_SYNC_RADIUS
#  define PLATFORM_GENERATE_CHUNKS_PER_TICK            PS2_GENERATE_CHUNKS_PER_TICK
#  define PLATFORM_LIGHTING_UPDATES_PER_FRAME           PS2_LIGHTING_UPDATES_PER_FRAME
#  define PLATFORM_LIGHTING_BUDGET_US                   PS2_LIGHTING_BUDGET_US
#  define PLATFORM_LIGHTING_MERGE_SCAN                   PS2_LIGHTING_MERGE_SCAN
#  define PLATFORM_LIGHTING_QUEUE_HARD_CAP               PS2_LIGHTING_QUEUE_HARD_CAP
#  define PLATFORM_FLOAT_BIOME_NOISE                   PS2_FLOAT_BIOME_NOISE
#  define PLATFORM_FLOAT_ORE_VEINS                     PS2_FLOAT_ORE_VEINS
#  define PLATFORM_TERRAIN_DENSITY_OCTAVES              PS2_TERRAIN_DENSITY_OCTAVES
#  define PLATFORM_TERRAIN_SELECT_OCTAVES               PS2_TERRAIN_SELECT_OCTAVES
#  define PLATFORM_SIN_TABLE_BITS                      PS2_SIN_TABLE_BITS
#  define PLATFORM_FORCE_GUI_SCALE                     PS2_FORCE_GUI_SCALE
#  define PLATFORM_NEAR_PLANE                          PS2_NEAR_PLANE
#  define PLATFORM_SHOW_LAGOMETER                      PS2_SHOW_LAGOMETER
#  define PLATFORM_RENDERER_AABB_MARGIN                PS2_RENDERER_AABB_MARGIN
#  define PLATFORM_CURSOR_TEXTURE                      PS2_CURSOR_TEXTURE
#  define PLATFORM_CURSOR_SIZE                         PS2_CURSOR_SIZE
#  define PLATFORM_DYNAMIC_TEXTURE_INTERVAL_TICKS      PS2_DYNAMIC_TEXTURE_INTERVAL_TICKS
#  define PLATFORM_TESSELLATOR_CONVERT_QUADS             PS2_TESSELLATOR_CONVERT_QUADS
#  define PLATFORM_TESSELLATOR_BUFFER_INTS               PS2_TESSELLATOR_BUFFER_INTS
#  define PLATFORM_FLOAT_VERTEX_MATH                     PS2_FLOAT_VERTEX_MATH
#  define PLATFORM_FAST_CHUNK_BLOCK_READS                PS2_FAST_CHUNK_BLOCK_READS
#else
#  define PLATFORM_DEFAULT_RENDER_DISTANCE              0
#  define PLATFORM_VISIBLE_CHUNK_RADIUS                 8
#  define PLATFORM_VISIBLE_CHUNK_DIAMETER               (PLATFORM_VISIBLE_CHUNK_RADIUS * 2 + 1)
#  define PLATFORM_VERTICAL_CHUNK_COUNT                 8
#  define PLATFORM_CENTER_VERTICAL_RENDERERS            0
#  define PLATFORM_CHUNK_CACHE_RADIUS                   8
#  define PLATFORM_CHUNK_UNLOAD_RADIUS                  12
#  define PLATFORM_CHUNK_MAP_RESERVE                    0
// Vanilla eviction rate: 4 chunks per tick, and only after 200 ticks (10 s)
// untouched. Unchanged from the values this table replaced.
#  define PLATFORM_MAX_CHUNK_UNLOADS_PER_TICK           4
#  define PLATFORM_EMERGENCY_CHUNK_UNLOADS_PER_TICK     16
#  define PLATFORM_MIN_UNUSED_TICKS_BEFORE_UNLOAD       200
#  define PLATFORM_PRELOAD_RADIUS_BLOCKS                128
#  define PLATFORM_PLAYER_UPDATE_CHUNK_RANGE_BLOCKS     32
#  define PLATFORM_DISABLE_RUNTIME_AUTOSAVE             0
#  define PLATFORM_RUNTIME_AUTOSAVE_LEVEL_DATA          1
#  define PLATFORM_SKIP_NEW_WORLD_FULL_SAVE             0
// Vanilla Beta 1.7.3: World::autosavePeriod, in world ticks. 40 ticks is two
// seconds, and each firing rewrites level.dat and flushes up to two dirty chunks
// through the deflate path. Cheap against a desktop page cache; not cheap
// against a console filesystem, which is why it is a knob now.
#  define PLATFORM_AUTOSAVE_PERIOD_TICKS                40
// Release 1.2.5 saves at most 24 dirty chunks during an incremental save.
#  define PLATFORM_INCREMENTAL_CHUNK_SAVE_LIMIT         24
// Whether chunk meshing is bounded per frame. Off on the desktop, where
// EntityRenderer's own time-limited retry loop is the budget and there is plenty
// of headroom to overshoot it.
//
// Turning this ON does two things that have to happen together, or neither
// works: it bounds the work inside RenderGlobal::updateRenderers, and it stops
// EntityRenderer calling updateRenderers repeatedly within one frame. Bounding
// only the first is pointless -- the outer loop just calls it again until the
// same total work is done, one budget at a time.
#  define PLATFORM_MESH_BUDGET                          0
#  define PLATFORM_MIN_RENDERER_UPDATES_PER_FRAME       1
#  define PLATFORM_MAX_RENDERER_UPDATES_PER_FRAME       99
#  define PLATFORM_MAX_RENDERED_SECTIONS_PER_PASS       999999
#  define PLATFORM_CHUNK_BUILD_BLOCKS_PER_STEP          4096
#  define PLATFORM_RENDERER_UPDATE_CANDIDATES_PER_FRAME 99
#  define PLATFORM_CULL_MISSING_CHUNK_BOUNDARY_FACES     0
#  define PLATFORM_CHUNK_BUILD_BUDGET_MS                0
#  define PLATFORM_CHUNK_BUILD_STEP_US                  0
#  define PLATFORM_CHUNK_BUILD_TIME_CHECK_BLOCKS        32
#  define PLATFORM_MESH_FACE_SORT                       0
#  define PLATFORM_FACE_BUCKET_CULL                     0
#  define PLATFORM_FACE_CULL_EYE_MARGIN                 0.0f
#  define PLATFORM_MESH_SORT_MAX_QUADS                  0
#  define PLATFORM_ENABLE_CHUNK_PREFETCH                0
#  define PLATFORM_PREFETCH_CHUNK_INTERVAL_FRAMES       20
#  define PLATFORM_PREFETCH_CHUNKS_PER_STEP             1
#  define PLATFORM_SKIP_TRANSPARENT_WORLD_PASS          0
#  define PLATFORM_ENABLE_GREEDY_MESH                   0
#  define PLATFORM_SKIP_ENTITY_SHADOWS                  0
#  define PLATFORM_SKIP_WORLD_ENTITIES                  0
#  define PLATFORM_SKIP_WORLD_PARTICLES                 0
#  define PLATFORM_SKIP_RAIN_SNOW                       0
#  define PLATFORM_SKIP_CLOUDS                          0
#  define PLATFORM_SKIP_BLOCK_SELECTION_BOX             0
#  define PLATFORM_ENABLE_BLOCK_RAYTRACE                1
#  define PLATFORM_ENABLE_BLOCK_BREAK_OVERLAY           1
#  define PLATFORM_SKIP_HAND_RENDER                     0
#  define PLATFORM_SKIP_CAMERA_FX                       0
#  define PLATFORM_SKIP_MOB_SPAWNING                    0
#  define PLATFORM_RANDOM_TICK_CHUNK_RADIUS             9
#  define PLATFORM_RANDOM_BLOCK_TICKS_PER_CHUNK         80
#  define PLATFORM_RANDOM_TICK_CHUNKS_PER_TICK          0
#  define PLATFORM_RANDOM_TICK_PROFILE_INTERVAL         0
#  define PLATFORM_MAX_LIVE_MOBS                        0x7fffffff
#  define PLATFORM_ENTITY_AI_NEAR_RADIUS_BLOCKS          1.0e9f
#  define PLATFORM_ENTITY_AI_FAR_RADIUS_BLOCKS           1.0e9f
#  define PLATFORM_ENTITY_AI_MID_TICK_DIVISOR            1
#  define PLATFORM_ENTITY_AI_FAR_TICK_DIVISOR            1
#  define PLATFORM_ENTITY_PUSH_COLLISION_RADIUS_BLOCKS   1.0e9f
#  define PLATFORM_FLOAT_ENTITY_AI_MATH                  0
#  define PLATFORM_FLOAT_ENTITY_CORE_MATH                0
#  define PLATFORM_FLOAT_FLUID_FLOW                       0
#  define PLATFORM_FAST_BIOME_BLEND                       0
#  define PLATFORM_FLOAT_EXPLOSION_MATH                   0
// Desktop keeps the exact double Vec3D helpers, the double block ray cast and
// the double AABB sweep. See Ps2Tuning.h for what each console knob trades.
#  define PLATFORM_FLOAT_VECTOR_MATH                     0
#  define PLATFORM_FLOAT_COLLISION_SWEEP                 0
#  define PLATFORM_PATHFIND_BUDGET_PER_TICK             0x7fffffff
#  define PLATFORM_PATHFIND_MAX_NODES                   0x7fffffff
#  define PLATFORM_MOB_SPAWN_INTERVAL_TICKS             1
#  define PLATFORM_FORCE_FULLBRIGHT_TERRAIN             0
#  define PLATFORM_SKIP_CAVE_GENERATION                 0
#  define PLATFORM_CAVE_SOURCE_RADIUS                   8
#  define PLATFORM_CAVE_RARITY                          15
#  define PLATFORM_FLOAT_CAVE_GENERATION                0
#  define PLATFORM_USE_HEIGHTMAP_TERRAIN                0
#  define PLATFORM_FAST_SURFACE_PASS                    0
#  define PLATFORM_PRECOMPUTE_INITIAL_HEIGHTMAP          0
#  define PLATFORM_BATCH_INITIAL_SKYLIGHT_RENDER_UPDATES 0
#  define PLATFORM_HEIGHTMAP_SEA_LEVEL                  64
#  define PLATFORM_HEIGHTMAP_BASE_HEIGHT                64
#  define PLATFORM_HEIGHTMAP_AMPLITUDE                  24
#  define PLATFORM_HEIGHTMAP_FREEZE_TEMP                0.5f
#  define PLATFORM_POPULATE_WATER_SPRINGS              50
#  define PLATFORM_POPULATE_LAVA_SPRINGS               20
#  define PLATFORM_POPULATE_DUNGEONS                   8
#  define PLATFORM_POPULATE_TREE_BONUS                  0
#  define PLATFORM_POPULATE_CLAY_VEINS                 10
#  define PLATFORM_POPULATE_DIRT_VEINS                 20
#  define PLATFORM_POPULATE_GRAVEL_VEINS               10
#  define PLATFORM_POPULATE_SNOW_PASS                  1
#  define PLATFORM_INCREMENTAL_POPULATE               0
#  define PLATFORM_POPULATE_CHUNKS_PER_TICK            0
#  define PLATFORM_POPULATE_STEPS_PER_TICK             PLATFORM_POPULATE_CHUNKS_PER_TICK
#  define PLATFORM_POPULATE_BUDGET_US                  0
#  define PLATFORM_POPULATE_SNOW_COLUMNS_PER_STEP      256
#  define PLATFORM_GENERATE_SYNC_RADIUS                1
#  define PLATFORM_GENERATE_CHUNKS_PER_TICK            0
// MetadataChunkBlock jobs are the unit World::updatingLighting() processes on
// each rendered frame.  Vanilla's 500 is appropriate when the CPU is idle; a
// console profile overrides it to keep chunk streaming from monopolising a
// frame.  The preload screen still drains the queue deliberately.
#  define PLATFORM_LIGHTING_UPDATES_PER_FRAME           500
// Wall-clock ceiling on that same drain, in microseconds.  Zero disables it and
// leaves the job count as the only bound on the desktop profile. Console
// profiles can override both because a job count is a poor proxy
// for time here: one MetadataChunkBlock is a flood fill over a box, so its cost
// varies by more than an order of magnitude with what just streamed in.
#  define PLATFORM_LIGHTING_BUDGET_US                   0
#  define PLATFORM_LIGHTING_MERGE_SCAN                   5
#  define PLATFORM_LIGHTING_QUEUE_HARD_CAP               1000000
#  define PLATFORM_FLOAT_BIOME_NOISE                   0
#  define PLATFORM_FLOAT_ORE_VEINS                     0
// Vanilla octave counts. See PS2_TERRAIN_DENSITY_OCTAVES/PS2_TERRAIN_SELECT_OCTAVES
// in Ps2Tuning.h for the console trade-off this overrides.
#  define PLATFORM_TERRAIN_DENSITY_OCTAVES              16
#  define PLATFORM_TERRAIN_SELECT_OCTAVES               8
// Power-of-two size of MathHelper's sine table. 16 is vanilla's 65536 entries;
// a console profile shrinks it to keep the table inside a small data cache.
#  define PLATFORM_SIN_TABLE_BITS                      16
#  define PLATFORM_FORCE_GUI_SCALE                     0
#  define PLATFORM_NEAR_PLANE                          0.05f
// Desktop: the F3 frame-time histogram covers a third of the screen and the
// port has better instrumentation elsewhere. Set to 1 for vanilla behaviour.
#  define PLATFORM_SHOW_LAGOMETER                      0
// Vanilla margin: sized for the occlusion-query box the desktop path uses.
#  define PLATFORM_RENDERER_AABB_MARGIN                6.0f
#  define PLATFORM_CURSOR_TEXTURE                      0
#  define PLATFORM_CURSOR_SIZE                         16
#  define PLATFORM_DYNAMIC_TEXTURE_INTERVAL_TICKS      1
#  define PLATFORM_TESSELLATOR_CONVERT_QUADS             1
#  define PLATFORM_TESSELLATOR_BUFFER_INTS               0x200000
// Desktop keeps the exact double vertex/UV math and the virtual chunk accessors:
// both console knobs trade a documented precision or structural allowance for EE
// time that no desktop CPU needs. See Ps2Tuning.h for what each one gives up.
#  define PLATFORM_FLOAT_VERTEX_MATH                     0
#  define PLATFORM_FAST_CHUNK_BLOCK_READS                0
#  define PLATFORM_MP_DEFERRED_CHUNKS                    0
#  define PLATFORM_MP_COMPRESSED_CHUNK_CACHE_BYTES       0u
#  define PLATFORM_MP_CHUNK_PROMOTIONS_PER_TICK          0
#  define PLATFORM_MP_MAX_DEFERRED_CHUNKS                768u
#  define PLATFORM_MP_MAX_CHANGES_PER_CHUNK              256u
#  define PLATFORM_MP_MAX_DEFERRED_CHANGES               8192u
#endif
