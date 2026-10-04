#pragma once

// -----------------------------------------------------------------------------
// Async chunk-generation scheduler
// -----------------------------------------------------------------------------
#if PLATFORM_ASYNC_CHUNK_GENERATION
#  if PLATFORM_PC_LEGACY
#    define PLATFORM_ASYNC_GENERATION_QUEUE_LIMIT        PC_LEGACY_ASYNC_GENERATION_QUEUE_LIMIT
#    define PLATFORM_ASYNC_GENERATION_REQUESTS_PER_TICK  PC_LEGACY_ASYNC_GENERATION_REQUESTS_PER_TICK
#    define PLATFORM_ASYNC_GENERATION_PUBLISH_PER_TICK   PC_LEGACY_ASYNC_GENERATION_PUBLISH_PER_TICK
#    define PLATFORM_ASYNC_GENERATION_THREAD_PRIORITY    PC_LEGACY_ASYNC_GENERATION_THREAD_PRIORITY
#    define PLATFORM_ASYNC_GENERATION_AFFINITY_MASK      PC_LEGACY_ASYNC_GENERATION_AFFINITY_MASK
#    define PLATFORM_ASYNC_ISOLATED_BIOME_SOURCE         PC_LEGACY_ASYNC_ISOLATED_BIOME_SOURCE
#    define PLATFORM_ASYNC_GENERATION_REQUESTS_PER_FRAME PC_LEGACY_ASYNC_GENERATION_REQUESTS_PER_FRAME
#    define PLATFORM_ASYNC_GENERATION_PUBLISH_PER_FRAME  PC_LEGACY_ASYNC_GENERATION_PUBLISH_PER_FRAME
// The desktop worker keeps Chunk construction on the game thread (see the
// scheduler construction in ChunkProvider.cpp), so saved chunks stay there too.
#    define PLATFORM_ASYNC_CHUNK_DECODE                  0
#    define PLATFORM_ASYNC_NEAREST_FIRST                 0
#  elif PLATFORM_WII
#    define PLATFORM_ASYNC_GENERATION_QUEUE_LIMIT        PLATFORM_WII_ASYNC_GENERATION_QUEUE_LIMIT
#    define PLATFORM_ASYNC_GENERATION_REQUESTS_PER_TICK  PLATFORM_WII_ASYNC_GENERATION_REQUESTS_PER_TICK
#    define PLATFORM_ASYNC_GENERATION_PUBLISH_PER_TICK   PLATFORM_WII_ASYNC_GENERATION_PUBLISH_PER_TICK
#    define PLATFORM_ASYNC_GENERATION_THREAD_PRIORITY    PLATFORM_WII_ASYNC_GENERATION_THREAD_PRIORITY
#    define PLATFORM_ASYNC_GENERATION_AFFINITY_MASK      0
// The worker samples biomes for terrain, caves and ravines through its own
// WorldChunkManager; the world's one has a BiomeCache the game thread mutates.
#    define PLATFORM_ASYNC_ISOLATED_BIOME_SOURCE         1
#    define PLATFORM_ASYNC_GENERATION_REQUESTS_PER_FRAME PLATFORM_WII_ASYNC_GENERATION_REQUESTS_PER_FRAME
#    define PLATFORM_ASYNC_GENERATION_PUBLISH_PER_FRAME  PLATFORM_WII_ASYNC_GENERATION_PUBLISH_PER_FRAME
// Decode saved chunks (NBT parse, block/light arrays, heightmap, skylight
// regeneration) on the worker, the same way it already constructs generated
// chunks; only entity construction is left for publish. Before this, a saved
// world paid the whole decode on the main thread per published column.
#    define PLATFORM_ASYNC_CHUNK_DECODE                  1
// The worker takes the queued column nearest the player's chunk instead of
// the oldest request. The queue is at most QUEUE_LIMIT entries, so the scan
// is a handful of compares per column.
#    define PLATFORM_ASYNC_NEAREST_FIRST                 1
#  elif PLATFORM_3DS
#    define PLATFORM_ASYNC_GENERATION_QUEUE_LIMIT        PLATFORM_3DS_ASYNC_GENERATION_QUEUE_LIMIT
#    define PLATFORM_ASYNC_GENERATION_REQUESTS_PER_TICK  PLATFORM_3DS_ASYNC_GENERATION_REQUESTS_PER_TICK
#    define PLATFORM_ASYNC_GENERATION_PUBLISH_PER_TICK   PLATFORM_3DS_ASYNC_GENERATION_PUBLISH_PER_TICK
#    define PLATFORM_ASYNC_GENERATION_THREAD_PRIORITY    PLATFORM_3DS_ASYNC_GENERATION_THREAD_PRIORITY
#    define PLATFORM_ASYNC_GENERATION_AFFINITY_MASK      PLATFORM_3DS_ASYNC_GENERATION_AFFINITY_MASK
// Same as the Wii: the worker samples biomes through its own
// WorldChunkManager; the world's one has a BiomeCache the game thread
// mutates.
#    define PLATFORM_ASYNC_ISOLATED_BIOME_SOURCE         PLATFORM_3DS_ASYNC_ISOLATED_BIOME_SOURCE
#    define PLATFORM_ASYNC_GENERATION_REQUESTS_PER_FRAME PLATFORM_3DS_ASYNC_GENERATION_REQUESTS_PER_FRAME
#    define PLATFORM_ASYNC_GENERATION_PUBLISH_PER_FRAME  PLATFORM_3DS_ASYNC_GENERATION_PUBLISH_PER_FRAME
// Decode saved chunks (NBT parse, block/light arrays, heightmap, skylight
// regeneration) on the worker, the same way it already constructs generated
// chunks; only entity construction is left for publish.
#    define PLATFORM_ASYNC_CHUNK_DECODE                  PLATFORM_3DS_ASYNC_CHUNK_DECODE
// The worker takes the queued column nearest the player's chunk instead of
// the oldest request.
#    define PLATFORM_ASYNC_NEAREST_FIRST                  PLATFORM_3DS_ASYNC_NEAREST_FIRST
#  else
#    define PLATFORM_ASYNC_GENERATION_QUEUE_LIMIT        0
#    define PLATFORM_ASYNC_GENERATION_REQUESTS_PER_TICK  0
#    define PLATFORM_ASYNC_GENERATION_PUBLISH_PER_TICK   0
#    define PLATFORM_ASYNC_GENERATION_THREAD_PRIORITY    64
#    define PLATFORM_ASYNC_GENERATION_AFFINITY_MASK      0
#    define PLATFORM_ASYNC_ISOLATED_BIOME_SOURCE         0
#    define PLATFORM_ASYNC_GENERATION_REQUESTS_PER_FRAME 0
#    define PLATFORM_ASYNC_GENERATION_PUBLISH_PER_FRAME  0
#    define PLATFORM_ASYNC_CHUNK_DECODE                  0
#    define PLATFORM_ASYNC_NEAREST_FIRST                 0
#  endif
#else
#  define PLATFORM_ASYNC_GENERATION_QUEUE_LIMIT        0
#  define PLATFORM_ASYNC_GENERATION_REQUESTS_PER_TICK  0
#  define PLATFORM_ASYNC_GENERATION_PUBLISH_PER_TICK   0
#  define PLATFORM_ASYNC_GENERATION_THREAD_PRIORITY    64
#  define PLATFORM_ASYNC_GENERATION_AFFINITY_MASK      0
#  define PLATFORM_ASYNC_ISOLATED_BIOME_SOURCE         0
#  define PLATFORM_ASYNC_GENERATION_REQUESTS_PER_FRAME 0
#  define PLATFORM_ASYNC_GENERATION_PUBLISH_PER_FRAME  0
#  define PLATFORM_ASYNC_CHUNK_DECODE                  0
#  define PLATFORM_ASYNC_NEAREST_FIRST                 0
#endif

// -----------------------------------------------------------------------------
// Network worker placement
// -----------------------------------------------------------------------------
// The 3DS pins its NetworkManager read/write threads to the secondary core
// (see DsWorldTuning.h); 0 -- the OS default scheduler -- everywhere else.
#ifndef PLATFORM_NETWORK_THREAD_AFFINITY_MASK
#  define PLATFORM_NETWORK_THREAD_AFFINITY_MASK      0
#endif
