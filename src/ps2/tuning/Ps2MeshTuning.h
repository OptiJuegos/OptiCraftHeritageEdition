#pragma once

// ---- Pass-0 mesh reordering (face buckets + atlas tile grouping) ----
//
// Measured 2026-07-28 in the [PS2] draw stats: of ~4000 triangle-equivalents
// pushed through the transform each frame only 617 reach the GS, and the batch
// health line reported quadsPerFlush=12 against a 32 maximum with
// clampSet=2894 tracking stripFlush=2900 one for one -- i.e. EVERY strip packet
// was being cut short by a texture-clamp change, because the per-block
// RenderBlocks path emits quads in block-scan order and consecutive quads
// almost never share an atlas cell.
//
// PS2_MESH_FACE_SORT reorders a finished opaque section mesh once, at build
// completion, into:
//     atlas tile (the 16x16 cell ps2_select_clamp keys REGION_REPEAT on)
//       -> bucket (face direction: +X -X +Y -Y +Z -Z, then "other")
//         -> 8x8x8 spatial cluster
//
// It moves whole quads only, so nothing about the geometry changes; pass 0 is
// opaque and depth-tested, so draw order is free. Pass 1 (water/ice/glass) is
// deliberately NOT reordered -- blending needs its back-to-front order.
//
// Compact range metadata retains both the face bucket and exact cluster AABB.
// PS2_FACE_BUCKET_CULL then uses the buckets at draw time: every terrain face
// is axis-aligned, so a +Y bucket whose lowest face sits above the eye is 100%
// backfacing and can be dropped with ONE float compare instead of 4 VU0
// transforms + 4 reciprocals + 4 projections + a signed-area test per quad.
//
// The two are separate knobs on purpose. The reorder is a pure win and cannot
// change what is drawn; the cull depends on the sign convention of the outward
// normal (derived from (p1-p0)x(p2-p0), verified against RenderBlocks::
// renderTopFace/renderBottomFace). If terrain ever renders inside-out, set
// PS2_FACE_BUCKET_CULL to 0 first -- that isolates the sign from the reorder.
#define PS2_MESH_FACE_SORT 1
#define PS2_FACE_BUCKET_CULL 1

// Spatial subdivision used by the opaque terrain mesh cache. Four cells per
// axis produce 64 4x4x4 clusters in a 16-block WorldRenderer section. The finer
// bounds let many edge sections keep their interior geometry on direct VU1 while
// only the actually intersecting clusters fall back to clipped VU0. The sorter
// keeps exact geometry bounds, so faces crossing a cell boundary remain
// conservative and cannot be clipped incorrectly.
#define PS2_MESH_CLUSTER_AXIS 4

// Direct VIF1/VU1/XGKICK terrain is enabled only when the optional VU1 backend
// is compiled (the stats build). It handles opaque terrain directly; the fast
// entry consumes fully-inside clusters and the optional clipped entry consumes
// partial/near clusters. VU0 remains the fallback and handles all translucent
// terrain. The original BASE/OFFSET/TOPS feed produced missing
// chunks on hardware, so the current A/B uses explicit ping-pong addresses and
// ITOP instead. The normal PS2 build disables the VU1 backend in CMake and
// therefore retains the production VU0 path without the extra SoA mesh.
#define PS2_DIRECT_VU1_TERRAIN 1

// Diagnostic-only VU1 canary. When the VU1 terrain backend is compiled, draw
// one fixed magenta quad after the translucent terrain pass through an
// absolute-address VIF1 upload and a single XGKICK. It deliberately avoids the
// terrain MVP, texture, fog, ADC strip restarts and VIF TOPS double buffering,
// so its visibility isolates the base VIF1 -> VU1 -> GIF Path1 -> GS contract.
// Set this to 1 only when isolating the base Path1 contract. The explicit-buffer
// terrain A/B below keeps it disabled so the overlay and its per-frame barriers
// do not contaminate performance measurements.
#define PS2_VU1_TERRAIN_CANARY 0

// A cluster can intersect the visible frustum while still fitting entirely
// inside the wider GS XY guard band. Those clusters need no clipping at all:
// classifyClusterGuardRisk proves, for the whole cluster AABB, that w >= the
// near epsilon and that |x| <= gx*w and |y| <= gy*w, so every vertex projects
// inside the guard band by construction and the GS scissor finishes the job.
//
// This is the single largest lever in the port. Measured 2026-08-27, standing
// still with the world loaded, per frame:
//     pass0            36.5 ms   (77% of a 47.3 ms render, in a 62 ms EE frame)
//     VU1              21948 vertices, 335 batches, 0.009 ms of wait
//     VU0/EE           15232 vertices
//     guardSafePartial 158 clusters / 15796 vertices
// guardSafePartial and vu0Verts agree to within 4%: essentially ALL the terrain
// the EE still transforms is guard-safe partial clusters held back by this
// switch alone, and the VU1 it would move them to is idle 99.99% of the frame.
// The EE side of that geometry measured 10.3 ms of counted xform/project/emit
// plus an uncounted remainder dominated by the 244 KB/frame gather memcpy that
// feeds it through an 8 KB D-cache.
//
// The first attempt at this produced cluster-sized terrain holes at screen
// edges and the switch was turned off. The suspected mechanism is not the
// classification but FTOI4: the EE paths clamp XY through gsKit's
// __gsKit_float_to_int_xy, the microprogram did not, and an out-of-range vertex
// therefore WRAPPED in the 16-bit XY field instead of clamping -- which, with
// ADC strip restarts, loses a whole strip rather than one quad. Ps2Vu1Terrain.vsm
// now applies that bound itself (MAXx/MINIw before FTOI4.xy), so a
// classification error can only stretch a primitive, not delete a cluster.
//
// That mechanism is a hypothesis until this runs on hardware. If the holes come
// back, set this to 0 -- that restores the previous behaviour exactly and the
// clamp is inert -- and the next thing to check is whether the cluster bounds
// and the guard planes are built in the same space, since the planes come from
// the translated MVP in ps2_renderer_prepare_translated_context.
//
// Watch in the [PS2] terrain clusters line: vu0Verts should fall to near zero
// and vu1Verts absorb it. Note also that the fast VU1 entry does no backface
// culling and no offscreen reject, both of which the EE path was doing per
// quad: of the 4102 quads/frame it used to transform only 1655 reached the GS.
// So the moved geometry arrives at the GS ~2.5x over, which is about +34% of
// total terrain primitives (7142 -> 9589 quads/frame at the measurement above).
// Face-bucket culling still applies at range granularity. GS wait was vblank
// rather than fill, so there should be room -- but that is the number to read
// if the frame does not improve as much as the EE saving predicts.
#define PS2_VU1_GUARD_BAND_PARTIALS 1

// Clip all partial opaque terrain directly on VU1. Keep this disabled until the
// bounded shadow probe below validates the clipped microprogram without making
// it authoritative for visible coverage.
#define PS2_VU1_CLIPPED_PARTIALS 0

// Submit at most this many clipped VU1 batches during an opaque terrain pass,
// while still drawing the same geometry through the established VU0 path. This
// validates the long MPG upload, clipped entry and XGKICK contract without
// allowing a missing VU1 packet to open a terrain hole. The first runtime
// probe submitted every batch successfully, but exposed invalid terrain/depth
// coverage. Keep it disabled until the probe packet rejects all pixel and
// depth writes; 0 restores the exact fast-only VU1 upload/layout.
#define PS2_VU1_CLIPPED_PROBE_BATCHES_PER_FRAME 0

// Safety margin, in blocks, added to both sides of the bucket visibility test.
//
// The eye position the test uses is RenderGlobal's d1/d2/d3, i.e. the interpolated
// render-view entity position. In Beta that IS the eye: Entity::setPosition builds
// the bounding box downward from posY by yOffset (1.62 for a player), so posY sits
// at eye level and the first-person test is exact.
//
// Third person is not: EntityRenderer::orientCamera pulls the camera up to
// thirdPersonDistance (4.0) back along the view vector inside the modelview
// matrix, and nothing outside that function sees the offset. Without a margin,
// a bucket could be dropped while the pulled-back camera can still see it --
// which reads as terrain vanishing when F5 is pressed. 4.0 covers the whole
// third-person arc; the cull only loses power for geometry within 4 blocks of
// the eye plane, which is a small slice of a 16-block section.
#define PS2_FACE_CULL_EYE_MARGIN 4.0f

// Largest opaque section mesh, in quads, that the reorder above will handle.
// Over this the mesh is stored unsorted (and drawn as one range, i.e. exactly
// the old behaviour). It bounds three static scratch arrays in Ps2MeshSort.cpp,
// all keyed by quad rather than by sort key: two u32 key arrays and a u16 write
// cursor, 40KB at 4096 quads.
//
// The sort key itself spans 256 atlas tiles x PS2_MESH_CLUSTER_COUNT clusters x
// PS2_FACE_GROUP_COUNT buckets. Indexing scratch by that instead would be 224KB
// with the current 4x4x4 clustering -- which is what the sorter used to do, and
// what this comment used to describe as "~11KB" from back when the cluster
// index was not part of the key.
//
// A 16^3 section of solid chequerboard is the theoretical worst case at 24576
// quads; real terrain sections measure 500-1500.
#define PS2_MESH_SORT_MAX_QUADS 4096

// Staging buffers shared by in-progress section meshes (Ps2MeshStagingPool.h).
//
// Every renderer used to own its pair, which meant the grid held 75 of them
// while the completion rate said only a handful of builds run at a time.
// Measured 2026-08-21: mesh= in the [PS2][FRAME] line accounted for 82-94% of
// the arena's growth across four periods, reaching 4.2 MB at 57 KB per section,
// and roughly half of that was staging for builds that were idle.
//
// Runtime logs with a 12-slot pool peaked at 4 simultaneous leases even while
// the renderer backlog was saturated. Keep two slots of headroom and stop paying
// the high-water capacity of six buffers that never become active. A build that
// finds the pool full simply defers to a later frame, which affects streaming
// latency rather than correctness.
//
// A leaked lease can still starve terrain permanently, so every abandon path
// must release through ps2ResetBuildState. Watch stage= in the frame line; pinned
// at 6 means this should be raised again.
#define PS2_MESH_STAGING_SLOTS 6

// Retained CPU mesh high-water caps. Reuse is still preferred for normal
// sections, but a renderer/staging slot that once saw an unusually dense mesh
// must not keep that oversized allocation for the rest of the session. The
// stable log is ~5.2 MB total mesh while the OOM run grew past 9 MB, mostly in
// retained raw/packed capacities. These limits are per buffer/cache and are
// only applied when a renderer is recycled or a staging lease is returned, so
// they avoid per-rebuild shrink/copy churn.
#define PS2_MAX_RETAINED_RAW_MESH_BYTES    (192 * 1024)
#define PS2_MAX_RETAINED_PACKED_MESH_BYTES (160 * 1024)

// Chunk renderers stepped per frame. 1 was sized against the old meshing cost;
// measured 2026-07-27 the stats build reports "renderers updated=1 pending=56"
// with "chunk build avg=1.6ms" inside a 42ms EE frame, i.e. 56 of the 72
// renderers waiting on a budget that spends under 4% of the frame. That
// backlog IS the terrain (and water) streaming in with holes: nothing is being
// culled — "terrain pass1 culled=0 rendered=listed=withGeom" — the sections
// simply have no mesh yet. 4 costs ~6ms/frame while the queue drains and
// nothing once it is empty.
#define PS2_MAX_RENDERER_UPDATES_PER_FRAME 4
#define PS2_MIN_RENDERER_UPDATES_PER_FRAME 4

// Candidates the update loop may TRY per frame, versus the budget above which
// now counts only renderers that actually meshed something.
//
// ps2BuildRendererStep bails out early (doing nothing but a 3x3 chunkExists
// sweep) whenever one of the section's source chunks has not been generated yet
// -- the generation gate. That bail-out used to consume one of the 4 slots, so
// with the outer cache ring streaming in, most frames spent their whole meshing
// budget on renderers that could not mesh: the FRAME log showed "renderers
// updated=1 pending=48" holding steady for thousands of frames while
// "chunk build" sat at 2-4ms, far under budget. Trying more candidates and
// charging the budget only for real work lets the queue actually drain.
#define PS2_RENDERER_UPDATE_CANDIDATES_PER_FRAME 32

// A visible edge section may legitimately build while the next chunk column is
// still outside the bounded PS2 cache. ChunkCache represents that absent column
// as air, which is correct for general reads but would expose every stone/dirt
// face along the 16x128 boundary as a giant temporary wall. Hide only default-
// culled opaque full-cube faces against a non-resident source column; when that
// neighbour is later published RenderGlobal invalidates only renderers that
// recorded that cardinal dependency.
#define PS2_CULL_MISSING_CHUNK_BOUNDARY_FACES 1

// Hard wall-clock ceiling on meshing per frame, in milliseconds, checked
// between renderers. PS2_CHUNK_BUILD_BLOCKS_PER_STEP bounds a step by BLOCK
// COUNT, which is only a proxy for time: measured 2026-07-28 the same 512-block
// budget produced "chunk build avg=2.7ms max=55.4ms" -- a 55ms step is a 4 fps
// frame on its own. This cuts the tail.
//
// NOTE: this degrades safely. System::nanoTime can be dead on some BIOS/emulator
// combinations (see the boot USABLE/DEAD probe), in which case the elapsed time
// reads 0, the ceiling never trips, and PS2_MAX_RENDERER_UPDATES_PER_FRAME alone
// governs exactly as it does today. Never make this the only limit.
#define PS2_CHUNK_BUILD_BUDGET_MS 6

// Mesh the nearest terrain while the loading screen is still visible. The same
// per-call 6 ms MeshBudget remains active, so this bounds total load latency
// without allowing one dense section to run unbounded.
#define PS2_LOAD_TERRAIN_WARMUP_MS 2500

// Per-renderer wall-clock slice. Disabled after the first runtime trial: the
// startup sample completed 72 build calls with zero published vertices and no
// terrain reached either render backend. The deterministic 512-block limit
// below and the 6ms inter-renderer budget remain active, matching the last
// known-good configuration while the inner slicing is redesigned.
#define PS2_CHUNK_BUILD_STEP_US 0
#define PS2_CHUNK_BUILD_TIME_CHECK_BLOCKS 32

// The PS2 renderer grid is 5x3x5 = 75 sections. The old cap of 64 silently
// dropped the tail of the sorted list, so a section could stay invisible until
// player movement re-sorted it into the first 64 entries. Keep a tiny amount of
// headroom while covering the whole grid; frustum/pass tests still reject work.
#define PS2_MAX_RENDERED_SECTIONS_PER_PASS 80

// Chunk renderer meshing is the main source of PS2 hitching.  A full
// 16x16x16 section build can take a visible chunk of one frame, so the PS2
// WorldRenderer builds a dirty section in small batches and swaps the finished
// mesh only when complete.  Lower = smoother movement, higher = chunks appear
// faster.
// Measured 2026-07-27 at 128: "chunk build avg=0.4-1.2ms" with n=240 per 120
// frames, i.e. the 2 steps/frame budget saturated permanently, inside an EE
// frame averaging 50ms. The throttle was sized when meshing a section was
// expensive; float biome noise, float ore veins and native quad meshes have
// since cut that, and 128 now just starves terrain streaming — a section needs
// 4096 blocks x 2 passes / 128 = 64 steps, so 75 renderers take ~2400 frames to
// rebuild and the world visibly streams in with holes. 512 remains the
// cheap-section ceiling; the wall-clock slice above stops dense sections earlier
// instead of allowing a single expensive batch to monopolize the frame.
// Lower = smoother movement, higher = chunks appear faster.
#define PS2_CHUNK_BUILD_BLOCKS_PER_STEP 512

// Optional slow background prefetch.  This shifts terrain generation away from
// the exact frame where the player crosses a chunk border.  Keep it disabled
// during heavy renderer backlog, and only do one chunk every N frames.
#define PS2_ENABLE_CHUNK_PREFETCH 1
// Lowered from 20 -> 10: with GENERATE_SYNC_RADIUS 0 the ring is streamed rather
// than force-generated, so prefetching ahead more often keeps chunks ready before
// the player reaches them. Each prefetched chunk is also cheaper now (float biome
// noise + trimmed replaceBlocksForBiome). Raise back toward 20 if the background
// generation itself causes a periodic hitch on the render thread.
#define PS2_PREFETCH_CHUNK_INTERVAL_FRAMES 10
#define PS2_PREFETCH_CHUNKS_PER_STEP 1

// Transparent world pass (water, ice, glass, portals).  It costs a second mesh
// sweep per chunk and extra per-frame vertex upload, but without it water/ice/
// glass are invisible, so keep it enabled.  PC keeps vanilla behavior.
#define PS2_SKIP_TRANSPARENT_WORLD_PASS 0

// Greedy meshing of opaque full cubes. Merges adjacent identically-shaded faces
// into bigger quads, cutting the vertex count the EE transforms each frame (PS2
// is transform-bound, not fill-bound). Non-cube/transparent blocks still use the
// per-block RenderBlocks path. Works best with
// PS2_FORCE_FULLBRIGHT_TERRAIN (uniform brightness => far more faces merge).
#define PS2_ENABLE_GREEDY_MESH 0

// Largest run the greedy mesher may merge, in cells, along either axis of a
// face plane. 16 = a whole section face; 1 = never merge.
//
// This exists to bisect greedy-mesh visual bugs, which have two independent
// possible sources that PS2_ENABLE_GREEDY_MESH alone cannot tell apart:
//
//   PS2_ENABLE_GREEDY_MESH 0   -> greedy code path gone entirely; every face is
//                                 drawn by RenderBlocks per block.
//   GREEDY 1 + MAX_MERGE 1     -> greedy code path ACTIVE but emitting exactly
//                                 one quad per visible face, i.e. the same
//                                 geometry the per-block path produces. Any
//                                 artifact that survives this is in makeFaceKey
//                                 or emitQuad (face selection, texture index,
//                                 shade, winding), NOT in the merging.
//   GREEDY 1 + MAX_MERGE 16    -> normal operation. An artifact that appears
//                                 only here is in the merge itself or in the
//                                 REGION_REPEAT tiling that merged quads need.
//
// Keep merged rectangles small on PS2. Large 8x8/16x16 quads occasionally
// intersect the camera/near guard and one of their strip triangles projects as
// a long textured spike (most visible in third person). 2x2 still removes up to
// 75% of equal faces on broad terrain while keeping the primitive close to the
// per-block geometry for which both the VU0 clipper and direct VU1 path are
// stable. Raise only for diagnostic testing.
#define PS2_GREEDY_MAX_MERGE 0

// One face direction contains 16 independent planes. Scanning all 16 in one
// WorldRenderer step still produced 44-56ms tail samples while terrain was
// streaming. Four planes bound a greedy call to 1024 source cells without
// changing the rectangles it emits (faces on different planes never merge).
// 16 restores the old one-face-per-step behavior.
#define PS2_GREEDY_SLICES_PER_STEP 0

// Extra in-world frame budget cuts.  These are PS2-only visual compromises that
// keep the first-person world path from spending time on non-critical effects.
#define PS2_SKIP_WORLD_ENTITIES 0
#define PS2_SKIP_WORLD_PARTICLES 1
#define PS2_SKIP_RAIN_SNOW 1
#define PS2_SKIP_CLOUDS 1
#define PS2_SKIP_BLOCK_SELECTION_BOX 1
// The star field is the one piece of sky geometry with a real RAM price. It has
// no display list to live in on this backend, so RenderGlobal captures it into
// a RenderCapturedMesh at boot: ~1400 quads x 4 vertices x 24 bytes = ~134 KB held
// for the whole session. The sky dome and the void plane are ~16 KB each and
// are not gated. Set to 0 to get the 134 KB back; Config::isStarsEnabled()
// (the in-game setting) still applies on top of this.
#define PS2_ENABLE_SKY_STARS 1

// Re-send only the GS page rows an animated tile actually touched, instead of
// the whole atlas, when TextureFX dirties terrain.png or items.png.
//
// Every frame the water/lava/fire/portal tiles rewrite themselves, and the next
// draw that binds the atlas used to DMA all 64KB of it again -- twice over, once
// per atlas. Those tiles cluster: on terrain.png portal and fire sit in the
// first 64-row GS page row and water and lava in the last, so two of four page
// rows move; on items.png the compass and the clock share one. About 128KB a
// frame becomes about 48KB.
//
// Set to 0 to go back to whole-atlas uploads. That is the first thing to try if
// terrain or item textures come out scrambled in 64-row bands, because the
// partial path computes its own GS destination address.
#define PS2_TEXTURE_PARTIAL_UPLOAD 1
// Keep rayTrace and breaking overlay enabled even when the white selection box is skipped.
#define PS2_ENABLE_BLOCK_RAYTRACE 1
#define PS2_ENABLE_BLOCK_BREAK_OVERLAY 1
#define PS2_SKIP_HAND_RENDER 0

// World simulation cuts. Rendering entities is already disabled above, so do
// not spend EE time scanning hundreds of chunks for mob spawning. Likewise,
// vanilla random block/cave-sound ticks scan a 19x19 chunk area every tick; on
// PS2 keep them inside the resident cache and use fewer attempts per chunk.
//
// Mobs were crashing while walking: a freed Entity left in a chunk's entities[]
// bucket was requeued by onChunkUnload->World::unloadEntities and deleted a second
// time, so the next updateEntities() jumped through its freed/poisoned vtable
// (R5900 "jump to unaligned address", PC near 0x0). Fixed in World::unloadEntities
// by skipping any pointer no longer present in loadedEntityList (= already deleted)
// before requeueing. Mobs can run again; flip back to 1 only for the EE perf budget
// (each mob is an A* + collision cost per tick), not for stability.
//
// DISABLED (1) at 32 MB: after populate was disabled, per-frame profiling showed
// SpawnerAnimals::performSpawning ("mobSpawn" phase) is now the dominant tick cost
// (575-1670 ms) AND a RAM leak that reached a 3rd OOM. Even with the eligible-chunk
// radius bounded to the resident cache, the spawn cluster's +-6 block spread probes
// into radius-3 (out-of-cache) chunks and force-generates them, plus the per-chunk
// biome lookup is costly. On a 32 MB budget mobs are unaffordable for now; re-enable
// with a cheaper spawn path (loaded-chunks-only probes, float biome) once headroom
// allows. See [[ps2-oom-reserve]].
