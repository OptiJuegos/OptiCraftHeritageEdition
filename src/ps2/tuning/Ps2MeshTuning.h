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

// Side-only partials are safe to clip on the existing VU1 clipped entry as
// long as the cluster is already proven fully in front of BOTH near hazards:
// the homogeneous OpenGL near plane (z + w >= 0) and the divide guard
// (w >= PS2_NEAR_CLIP_W). The clipped entry handles only w + left/right/
// down/up guard planes, so near/mixed-risk clusters must stay on VU0.
//
// This is intentionally separate from PS2_VU1_CLIPPED_PARTIALS: enabling the
// latter would make the clipped entry authoritative for near-plane geometry,
// which it does not currently clip correctly.
#define PS2_VU1_SIDE_CLIPPED_PARTIALS 0

// Classify cluster AABBs against the frustum and guard band on COP2 (VU0 macro
// mode) instead of the scalar FPU. See Ps2Vu0Math.h for the transposed plane
// layout the kernel needs.
//
// This is the densest block of per-frame float work left outside the emitters:
// every cluster is tested against six frustum planes plus six near/side guard
// planes, at PS2_MESH_CLUSTER_COUNT clusters per section and up to
// PS2_MAX_RENDERED_SECTIONS_PER_PASS sections per pass. The scalar form also
// recomputes the cluster centre and extent once per plane and takes three fabsf
// per plane per cluster; both are hoisted out by the transposed block, which is
// built once per frame.
//
// The arithmetic is the same expression in the same order, but VU0 floats are
// not IEEE-754 (denormals flush to zero, overflow clamps instead of producing an
// infinity), so a grazing cluster can in principle land on the other side of
// kClusterPlaneEpsilon. Turn this off first if terrain starts stretching or
// disappearing at screen edges: a misclassified guard risk is exactly the
// failure PS2_VU1_GUARD_BAND_PARTIALS documents, and 0 restores the scalar
// classifier bit for bit.
#define PS2_VU0_CLUSTER_CULL 1

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
// Keep only a small set of builds active at once. The scheduler may still spend
// up to 10 real build steps per frame, but it now revisits these active builders
// in multiple rounds instead of spreading one step across ten different sections.
// This trades staging concurrency for lower time-to-first-published-mesh while the
// existing 6 ms wall-clock budget remains the hard CPU guard.
//
// A leaked lease can still starve terrain permanently, so every abandon path
// must release through ps2ResetBuildState. Watch stage= in the frame line; pinned
// at 4 is expected while the terrain queue is saturated.
#define PS2_MESH_STAGING_SLOTS 4

// Retained CPU mesh high-water caps. Reuse is still preferred for normal
// sections, but a renderer/staging slot that once saw an unusually dense mesh
// must not keep that oversized allocation for the rest of the session. The
// stable log is ~5.2 MB total mesh while the OOM run grew past 9 MB, mostly in
// retained raw/packed capacities. These limits are per buffer/cache and are
// only applied when a renderer is recycled or a staging lease is returned, so
// they avoid per-rebuild shrink/copy churn.
#define PS2_MAX_RETAINED_RAW_MESH_BYTES    (128 * 1024)
#define PS2_MAX_RETAINED_PACKED_MESH_BYTES (128 * 1024)

// Global terrain-mesh budget. Per-renderer retention caps prevent one section
// from owning an extreme allocation, but they do not bound the sum across the
// renderer grid. The tutorial-world trace reached ~15.8 MB of terrain mesh with
// only 34-35 chunks resident, so the global policy trims inactive capacities and
// evicts far, off-screen published meshes before the arena reaches OOM.
//
// 6/7.5 MB (from 8/10): the mesh cache was the single largest heap consumer
// and the stable-state log sat at ~5.2 MB, so the budget only ever bought
// headroom for far, off-screen sections that the trimmer drops first anyway.
// Greedy meshing (PS2_ENABLE_GREEDY_MESH) also shrinks what a section needs.
// Raise back toward 8/10 if the [PS2] mesh line shows evictions while the
// player stands still.
#define PS2_MESH_RAM_TARGET_BYTES             (6u * 1024u * 1024u)
#define PS2_MESH_RAM_HARD_BYTES               (7680u * 1024u)
#define PS2_MESH_RAM_EMERGENCY_FREE_KB        2048u
#define PS2_MESH_TRIM_SOFT_MAX_PER_FRAME      2
#define PS2_MESH_TRIM_HARD_MAX_PER_FRAME      4
#define PS2_MESH_TRIM_KEEP_RADIUS_BLOCKS      16.0f

// Chunk renderers stepped per frame. 1 was sized against the old meshing cost;
// measured 2026-07-27 the stats build reports "renderers updated=1 pending=56"
// with "chunk build avg=1.6ms" inside a 42ms EE frame, i.e. 56 of the 72
// renderers waiting on a budget that spends under 4% of the frame. That
// backlog IS the terrain (and water) streaming in with holes: nothing is being
// culled — "terrain pass1 culled=0 rendered=listed=withGeom" — the sections
// simply have no mesh yet. 4 costs ~6ms/frame while the queue drains and
// nothing once it is empty.
//
// Raised 4 -> 10: the measurement above was taken while the bail-out still
// consumed a slot. It no longer does (see PS2_RENDERER_UPDATE_CANDIDATES_PER_FRAME),
// so this number now counts only renderers that actually meshed, and a backlog of
// 70 sections drains ten times per frame instead of four. PS2_CHUNK_BUILD_BUDGET_MS
// is the real ceiling either way -- it stops the loop on wall-clock -- so this
// cannot overrun the frame, and it costs nothing once the queue is empty.
#define PS2_MAX_RENDERER_UPDATES_PER_FRAME 10

// Keep an active incremental section build alive when deferred population dirties
// it. The published mesh remains visible and one final rebuild is left pending
// after the staging mesh completes. Distant already-published sections may also
// wait for population to finish before starting that final rebuild; near sections
// stay immediate so block interaction remains responsive.
#define PS2_COALESCE_MESH_REBUILDS 1

// Edit latency. A block placed or broken used to take about a second to show:
// the section queued behind active streaming builds and the four staging
// leases, and every frame of the light propagation re-dirtied it, which
// restarts an active build (DirtyRestart). Three measures, each off at 0:
//
// A block change within this distance of the viewer marks the section urgent.
// Urgent sections sort ahead of active builds and run to completion inside
// their own wall-clock budget, before the shared 6 ms budget is spent. Two
// sections (32 blocks) covers what the player can reach or is looking at.
#define PS2_URGENT_MESH_DISTANCE_SQ 1024.0f
// A dense section is ~10-20 ms; one frame of hitch on an edit is the trade
// early Pocket Edition made with its synchronous rebuild.
// 32 (from 20): measured 2026-09-16 with the urgent-lane log, a surface
// section is 35-45 steps at ~0.7 ms, i.e. 25-30 ms. At 20 the lane always
// yielded after ~30 steps and the edit landed a frame later (plus the vsync
// wait between tick and render), ~100 ms from click to mesh; 32 fits the
// whole section in the frame of the edit. Sections dirtied on a border still
// queue behind the first one.
#define PS2_URGENT_MESH_BUDGET_MS 32
// Step cap of the same lane; the clock is the real bound, this only matters
// on a board whose monotonic clock reads 0. 96 covers a section with margin.
#define PS2_URGENT_MESH_STEP_CAP 96
// The mark is issued only inside World::PlayerEditMarkScope (the player
// controller's break / place / use). Measured 2026-09-16 before that scope
// existed: while flying over new terrain the distance test alone also caught
// sections dirtied by flowing springs, settling gravel and the neighbour's
// deferred populate, and each ran to completion in this lane on top of the
// 6 ms budget -- the build phase averaged 12 ms against a 6 ms budget.
// Staging leases kept free of streaming builds so an urgent build can begin
// the frame it is marked. Out of PS2_MESH_STAGING_SLOTS (below).
#define PS2_MESH_STAGING_RESERVE_FOR_URGENT 1
#define PS2_DEFER_MESH_DURING_POPULATE 1
// With chunk-local decoration (PS2_CHUNK_LOCAL_DECORATION, Ps2WorldTuning.h;
// expanded at the use site, so the later include is fine) the deferred
// populate only places structures, lakes, dungeons, springs and animals, none
// of which is worth holding a section's first mesh for; a structure that lands
// later remeshes it. Without it, the 32-block deferral keeps trees from
// double-meshing.
#define PS2_POPULATE_MESH_DEFER_DISTANCE_SQ (PS2_CHUNK_LOCAL_DECORATION ? 1.0e12f : 1024.0f)
#define PS2_MIN_RENDERER_UPDATES_PER_FRAME 4

// Whether a section waits for a source chunk that is missing but still inside the
// streaming cache radius, instead of meshing immediately against the air
// ChunkCache reports for it.
//
// Waiting was correct while PS2_CHUNK_CACHE_RADIUS equalled PS2_VISIBLE_CHUNK_RADIUS:
// every chunk a visible section needed was then OUTSIDE the load radius, so
// World::isChunkInLoadRadius returned false, the gate fell through, and edge
// sections meshed at once. Now that the cache is deliberately one ring larger than
// the render window, the chunks a visible section samples sit INSIDE the load
// radius, the gate takes the waiting branch, and the whole outer ring of the render
// window blocks on a streaming ring that is never complete while the player moves.
// That is the "renderers updated=1 pending=48 for thousands of frames" backlog
// described above, which is what terrain-with-holes actually looks like.
//
// Meshing early is safe here because both halves of the recovery already exist:
// PS2_CULL_MISSING_CHUNK_BOUNDARY_FACES hides the default-culled opaque faces that
// would otherwise appear as a wall along the absent column, and publishing that
// column runs World::notifyChunkPublishedForRender -> RenderGlobal::onChunkPublished,
// which marks exactly the renderers that recorded the dependency and re-queues them
// at priority. The cost is one extra rebuild per edge section, paid off the critical
// path, instead of an unbounded wait on the critical path.
//
// Set to 1 to restore the waiting behaviour.
#define PS2_MESH_WAIT_FOR_PENDING_SOURCES 0

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

// Skip opaque unit cubes whose six neighbours are opaque. The code path already
// existed in WorldRendererPs2 but was never wired to a tuning define, so it was
// compiled out on every PS2 build. This removes interior stone/dirt work before
// RenderBlocks performs color, brightness, texture and face setup.
#define PS2_SKIP_ENCLOSED_OPAQUE_CUBES 1

// Standard opaque cubes can reuse the neighbour mask already computed for the
// enclosed-cube test. RenderBlocks then emits only those faces instead of asking
// shouldSideBeRendered six more times. Special bounds/culling/alpha blocks stay
// on the normal renderer path.
#define PS2_FAST_SIMPLE_CUBE_RENDER 1

// Conservative section portal visibility. Only definitely solid, unit opaque
// cubes participate as occluders; all special or uncertain blocks are treated as
// open. A false negative only draws extra terrain, while a false positive is
// structurally avoided.
#define PS2_CPU_SECTION_OCCLUSION 0

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

// Keep a short visible loading phase for normal local world entry, but spend it
// on renderer warm-up rather than a blind sleep. The minimum is measured from
// Minecraft::changeWorld() entering the loading screen; the warm-up value is a
// hard ceiling on the extra mesh work after RenderGlobal has created its grid.
#define PS2_LOAD_TERRAIN_MIN_MS 1200
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
#define PS2_ENABLE_GREEDY_MESH 1

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
#define PS2_GREEDY_MAX_MERGE 2

// One face direction contains 16 independent planes. Scanning all 16 in one
// WorldRenderer step still produced 44-56ms tail samples while terrain was
// streaming. Four planes bound a greedy call to 1024 source cells without
// changing the rectangles it emits (faces on different planes never merge).
// 16 restores the old one-face-per-step behavior.
#define PS2_GREEDY_SLICES_PER_STEP 4

// Extra in-world frame budget cuts.  These are PS2-only visual compromises that
// keep the first-person world path from spending time on non-critical effects.
#define PS2_SKIP_WORLD_ENTITIES 0
#define PS2_SKIP_WORLD_PARTICLES 0
// Particle budget for the profile above with particles on. Each particle used
// to pay Entity::moveEntity (AABB gather and sweep) for a 0.2 cube: a broken
// block spawned 64 of them and the `effects` tick phase read 11-21 ms right
// after an edit (2026-09-16). Fast physics is a per-axis point test against
// the block collision box; brightness is sampled every N ticks instead of
// every frame; the destroy grid is per axis (2 -> 8 fragments, vanilla 4 ->
// 64); the per-layer cap replaces vanilla's 4000.
#define PS2_FAST_PARTICLE_PHYSICS 1
#define PS2_PARTICLE_BRIGHTNESS_INTERVAL 4
#define PS2_BLOCK_DESTROY_PARTICLE_GRID 2
#define PS2_MAX_PARTICLES_PER_LAYER 256
// The full rain/snow curtains are already disabled below, but vanilla still
// spawns up to 100 ground-impact EntityRainFX objects every tick. Their average
// lifetime keeps hundreds of alpha-tested quads alive around the camera; on the
// PS2 that is enough to halve the frame rate over open water. Four attempts per
// tick still provide a continuous splash effect (~80 new particles/second at
// 20 TPS) while bounding both particle physics and GS overdraw.
#define PS2_RAIN_SPLASH_PARTICLES_PER_TICK 4
// Entity::isBurning() draws a stack of heavily overlapping fire billboards. A
// normal zombie produces about five layers with vanilla's 0.45 step. Keep three
// broader-spaced layers on PS2: the silhouette remains covered, but fill-rate
// and generic tessellator vertex work are reduced for every burning mob.
#define PS2_ENTITY_FIRE_MAX_LAYERS 3
#define PS2_ENTITY_FIRE_LAYER_STEP 0.70f
#define PS2_SKIP_RAIN_SNOW 1
#define PS2_SKIP_CLOUDS 1
#define PS2_SKIP_BLOCK_SELECTION_BOX 1
// The star field is the one piece of sky geometry with a real RAM price. It has
// no display list to live in on this backend, so RenderGlobal captures it into
// a RenderCapturedMesh at boot: ~1400 quads x 4 vertices x 24 bytes = ~134 KB held
// for the whole session. The sky dome and the void plane are ~16 KB each and
// are not gated. Set to 0 to get the 134 KB back; Config::isStarsEnabled()
// (the in-game setting) still applies on top of this.
// 0: the performance profile already starts with stars off (GameDefaults), so
// the capture only ever paid its 134 KB for a setting the player has to opt
// back into. Turning the setting on with this at 0 draws no stars.
#define PS2_ENABLE_SKY_STARS 0

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

// GS mipmaps for the tile atlases (terrain.png, gui/items.png).
//
// The GS texture cache is 8 KB. Sampling a 256x256 atlas at level 0 for a
// section twenty blocks away fetches a fresh page for nearly every quad, and
// the texel-to-pixel ratio there is 2-4:1, which is also what shimmers. Levels
// 1..N are a quarter, a sixteenth... of the level-0 footprint, so the cache
// holds them and the fetch cost drops with distance instead of staying flat.
//
// RenderEngine already builds the chain: ofMipmapLevel > 0 makes setupTexture
// upload levels 1..N through renderTextureImageRgba and TextureFX re-sends the
// animated tiles per level through renderTextureSubImageRgba. This backend used
// to reject anything but level 0; with this on, Ps2Texture keeps the extra
// levels for tile atlases only (PSMT8, sharing the level-0 CLUT) and the two
// native terrain emitters program TEX1/MIPTBP1. Every other draw still goes
// through gsKit primitives, which write a fixed-LOD TEX1 of their own, so the
// GUI, entities and the held item keep sampling level 0 unchanged.
//
// Level selection is TEX1 LCM=0: LOD = log2(1/|Q|) * 2^L + K/16 with Q = 1/w
// from the perspective divide, i.e. LOD = log2(distance in blocks) + K/16. At
// 640x448 and a 70 degree vertical FOV a 16-texel tile covers 320/w pixels, so
// one texel is one pixel at w = 20 blocks. K below is tuned for
// NEAREST_MIPMAP_NEAREST (the GS rounds LOD): level 1 from w = 16, level 2 from
// w = 32, which lands inside the 48-block render window. Raise K toward 0 to
// hold level 0 further out; lower it for a softer far field.
//
// First thing to check on hardware: REGION_REPEAT tiles at distance. The GS
// shifts UMSK/UFIX by LOD for mip levels, which is what keeps a 16x16 tile
// addressed correctly at 8x8 and 4x4; if far terrain shows the wrong tile,
// set this to 0 -- that restores level-0-only sampling bit for bit.
#define PS2_TERRAIN_MIPMAPS 1
// Levels beyond 0 the profile asks for by default (ofMipmapLevel). MIPTBP1
// addresses three, so the backend caps at PS2_TERRAIN_MIP_MAX_LEVELS whatever
// the option says; a 256 atlas at level 3 is 32x32, 2x2 texels per tile.
#define PS2_TERRAIN_MIP_LEVELS 2
#define PS2_TERRAIN_MIP_MAX_LEVELS 3
// TEX1.K, signed, 1/16 units.
#define PS2_TERRAIN_MIP_LOD_BIAS_K -56
// TEX1.L, 0..3: LOD slope shift. 0 is the plain log2(distance) curve.
#define PS2_TERRAIN_MIP_LOD_L 0
// 1 = NEAREST_MIPMAP_LINEAR (blend between the two nearest levels; two texel
// fetches per sample). 0 = NEAREST_MIPMAP_NEAREST, one fetch.
#define PS2_TERRAIN_MIP_LINEAR 0
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
