#pragma once

// -----------------------------------------------------------------------------
// Frustum / distance culling (PLATFORM_WII)
// -----------------------------------------------------------------------------

// Límite de secciones dibujadas por paso. sortedWorldRenderers ya está
// ordenado por distancia al jugador (de cerca a lejos), así que esto
// simplemente trunca las lejanas. Con TINY (200 secciones) no hace nada,
// pero con SHORT (648) o NORMAL (2312) evita saturar la FIFO de GX.
#undef  PLATFORM_MAX_RENDERED_SECTIONS_PER_PASS
#define PLATFORM_MAX_RENDERED_SECTIONS_PER_PASS  48

// Radio de culling por distancia, en bloques. Secciones más allá de este
// radio desde la cámara se marcan como fuera de frustum aunque pasen el
// test de los 6 planos. El frustum de 90° FOV en un grid de 17x17 chunks
// incluye secciones diagonales a ~70 bloques que apenas se ven.
#undef  PLATFORM_WII_DISTANCE_CULL_RADIUS
#define PLATFORM_WII_DISTANCE_CULL_RADIUS        48.0

// Background chunk prefetch shifts generation away from the exact frame where
// the player crosses a chunk boundary. Keep it deliberately slow: one missing
// chunk every four rendered frames.
#undef  PLATFORM_ENABLE_CHUNK_PREFETCH
#define PLATFORM_ENABLE_CHUNK_PREFETCH            1
#undef  PLATFORM_PREFETCH_CHUNK_INTERVAL_FRAMES
#define PLATFORM_PREFETCH_CHUNK_INTERVAL_FRAMES    4
#undef  PLATFORM_PREFETCH_CHUNKS_PER_STEP
#define PLATFORM_PREFETCH_CHUNKS_PER_STEP          1

// New terrain discovered outside the occupied chunk is streamed by a low-priority
// LWP worker. Region reads and zlib inflation happen on that worker; the main thread
// only parses the already-inflated NBT and publishes completed chunks per tick.
// Missing-on-disk chunks continue directly into world generation on the same worker.
//
// REQUESTS_PER_TICK moves items from ChunkProvider::wiiGenerationQueue into the
// worker's own queue -- it does not do the I/O/generation work itself, that stays
// on runWiiGenerationThread(). At 1/tick this can starve the worker: the memory
// log put a loaded Wii session at avgFps 5-7 / ~12-14 real ticks/s (see
// [[wii-gxlist-pool]]'s 2026-08-06 entry), so the worker -- lower priority than
// the main thread but otherwise free to run back-to-back -- could finish a chunk
// and then sit in its usleep(1000) idle branch waiting for the next tick to feed
// it one more, instead of draining several queued requests in a row. That idling
// is pop-in: terrain the player is already walking toward is not yet requested.
//
// PUBLISH_PER_TICK must move with it, not stay behind: drainWiiGeneratedChunks()
// is what actually adds a column to chunkMap (the ~80 KB/column cost). Results
// sitting in wiiGenerationResults are already fully generated/allocated but not
// yet counted against the resident cache or eligible for unload eviction, so if
// requests are fed faster than results are published, that queue grows
// unbounded instead of the memory being visible where the eviction logic can see
// it. Raising both together keeps arrival and drain matched -- 2 is a starting
// point, not a measurement; watch wiiGenerationResults staying small (add it to
// the [WII][RAM] line if it needs to be visible) and the existing free/ordblks
// figures across a walking session before going higher.
#define PLATFORM_WII_ASYNC_GENERATION_QUEUE_LIMIT       16
#define PLATFORM_WII_ASYNC_GENERATION_REQUESTS_PER_TICK  2
#define PLATFORM_WII_ASYNC_GENERATION_PUBLISH_PER_TICK   2
#define PLATFORM_WII_ASYNC_GENERATION_REQUESTS_PER_FRAME  1
#define PLATFORM_WII_ASYNC_GENERATION_PUBLISH_PER_FRAME   1

// A renderer can depend on several neighbouring chunk columns because ChunkCache
// samples a one-block border. Queue several missing dependencies in one pass
// instead of serialising them across separate renderer attempts.
#define PLATFORM_WII_RENDERER_DEPENDENCY_REQUESTS_PER_STEP 3

// Scheduler priority of that worker. libogc's range is LWP_PRIO_IDLE 0 ..
// LWP_PRIO_HIGHEST 127 and a HIGHER number means MORE priority (ogc/lwp.h), so
// the 96 this used to pass was not low priority at all: it was the highest
// priority in the whole program, above the network threads at 64
// (NetworkManager.cpp), above the audio stream at 80 (SoundManager.cpp), and
// above libogc's own main thread. The comment above already said
// "low-priority"; only the constant disagreed.
//
// That single number is what made the async generator fail to be async. LWP is
// strictly priority-preemptive and does not time-slice, and
// runWiiGenerationThread() blocks nowhere while
// ChunkProviderGenerate::provideChunk() runs -- terrain noise, biome
// replacement, MapGenCaves' 17x17 sweep and generateSkylightMap, all CPU. So
// every generated chunk froze the main thread for its full duration. The stall
// was relocated off the world tick, not removed, and it could now land in the
// middle of a render instead of only inside a tick.
//
// It has to be strictly BELOW the main thread, and "equal" would not do:
// without time-slicing, an equal-priority CPU-bound worker is never preempted
// either. Below it, the worker gets the gap the frame already contains -- the
// main thread sleeps in GX_DrawDone() and VIDEO_WaitVSync() every frame (see
// wiigl_end_frame in gx_wii.cpp), which at 60 fps is several milliseconds of
// genuine leftover CPU.
//
// The trade is pop-in, not a stall: terrain now streams in over several frames
// rather than freezing one. Raise it toward the main thread if chunks cannot
// keep up with a walking player -- never to it or past it.
#define PLATFORM_WII_ASYNC_GENERATION_THREAD_PRIORITY   32

// Face sorting, now that the Wii does store each direction in a separate GX
// list. This used to be off because sorting one combined list adds rebuild work
// and could not cull anything at draw time -- a display list is a command stream
// and a draw cannot skip a span inside one.
//
// wii_native_compile_chunk_list() records the opaque section as one display
// sub-list per direction instead, each at a 32-byte boundary so GX_CallDispList
// can be pointed at it, and the cull below skips the ones facing away. GX would
// reject those triangles at the backface cull regardless; what is saved is
// pushing their vertices through the write-gather pipe and the transform unit
// first. The split costs one extra GX_Begin plus alignment padding per
// direction, which is why faceCull= in the [WII][RAM] line reports how many
// sub-lists were actually skipped -- if that number is small, this is overhead.
#undef  PLATFORM_MESH_FACE_SORT
#define PLATFORM_MESH_FACE_SORT                    1

#undef  PLATFORM_FACE_BUCKET_CULL
#define PLATFORM_FACE_BUCKET_CULL                  1

// Slack, in blocks, on the eye-vs-plane test. The test is per section and per
// direction against the extent of that direction's quads, so a camera sitting
// exactly on a face plane is the ambiguous case; half a block of margin keeps it
// on the drawing side. Same role as the PS2's, smaller because the Wii tests
// section-local planes rather than cluster bounds.
#undef  PLATFORM_FACE_CULL_EYE_MARGIN
#define PLATFORM_FACE_CULL_EYE_MARGIN              0.5f
#undef  PLATFORM_MESH_SORT_MAX_QUADS
#define PLATFORM_MESH_SORT_MAX_QUADS               4096

// Use the PS2 all-float biome simplex path on Wii as well. This lowers world-gen
// CPU cost but can move biome boundaries slightly versus Java double arithmetic.
#undef  PLATFORM_FLOAT_BIOME_NOISE
#define PLATFORM_FLOAT_BIOME_NOISE                 0
#undef  PLATFORM_FLOAT_ORE_VEINS
#define PLATFORM_FLOAT_ORE_VEINS                   0
