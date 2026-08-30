# Java 1.2.5 to C++ Translation Fixes

This file is cumulative. Future audit passes should append a new dated section instead of replacing previous entries.

## 2026-08-25 - Release 1.2.5 fidelity audit

### Java semantics and numeric fidelity
- Restored Java `float`-before-`double` rounding in entity water checks, AI helpers, Iron Golem particles, falling-sand rendering, dragon-egg falling entities, End spike crystals, door AI offsets, projectile collision margins, remote-player camera pitch, and Ender Dragon vertical clamping.
- Restored the original `TextureFlamesFX` float operation sequence instead of the mathematically equivalent but bit-different simplified expression.
- Replaced signed C++ arithmetic with `JavaArithmetic` where Java requires 32-bit wraparound, including chunk dirty bounds, deferred client chunk coordinates, structure distance math, dragon-egg movement, End spike generation, and boat hurt animation.
- Kept the existing chunk payload/storage bounds checks and structure snapshots that prevent C++ out-of-bounds access or container invalidation; these safety fixes intentionally remain stronger than literal Java code.

### Release 1.2.5 APIs and runtime call paths
- Restored `ChunkCoordIntPair::chunkXZ2Int` to the Release 1.2.5 64-bit X/Z packing contract; retained `chunkXZ2Long` as a compatibility alias.
- Restored `Tessellator::draw()` returning the consumed byte count.
- Restored `FontRenderer::drawStringWithShadow()` / internal render return values and Release 1.2.5 bold (`§l`) width accounting.
- Restored `IProgressUpdate::displaySavingString()` and routed world saving through it; retained the older method name as a compatibility alias.
- Restored `LoadingScreenRenderer::func_597_c()` and the vanilla 100 ms progress redraw cadence.
- Restored `GameSettings::getKeyDisplayString()` and routed option key labels through the Release 1.2.5 API.
- Restored `RenderBlocks::renderCrossedSquares()` as the render-type 1 entry point; retained `renderBlockReed()` as a compatibility wrapper.
- Restored `PlayerController::windowClick()` returning the clicked `ItemStack` and ensured the multiplayer virtual `handleWindowClick()` path is actually used. GUI callers dispose ignored temporary results.
- Restored `World::createExplosion()` / `newExplosion()` returning an `Explosion` object as in Java; current ignored call sites explicitly release the C++ temporary after synchronous explosion processing.
- Corrected both the standalone network thread classes and the actual `NetworkManager` runtime loops to use the Release 1.2.5 2 ms cadence and writer flush ordering.

### Materials, UI, and compatibility
- Added the canonical Release 1.2.5 `Material::piston` singleton. `pistonBase` and `pistonSticky` remain aliases for Beta++ compatibility, without double deletion.
- Updated piston base, extension, and moving blocks to use the canonical piston material.
- Corrected the `GuiConfirmOpenLink` warning color to the Release 1.2.5 value.
- Preserved Beta++/OptiFine options, F3/UI extensions, console render budgets, PS2/Wii input/render backends, and console terrain optimizations instead of deleting them. Fidelity fixes are applied around those extensions.

### Audit status
- Conservative local/member initialization scans found no additional high-confidence uninitialized primitive/member bug in this pass; reviewed candidates were initialized by constructor delegation, shared initialization helpers, loop initializers, or complete branches.
- Core no-op methods that are also no-ops in the Release 1.2.5 Java source were left unchanged rather than being incorrectly filled in.
- The PS2 screenshot framebuffer-readback backend remains an explicit platform stub (`renderReadPixelsRgb` returns false). It is documented rather than replaced with an unverified GS readback implementation.
- Validation for this pass is static/source-level only. The project was not compiled automatically, per `CODESTYLE.MD`.

## 2026-08-25 - Current ZIP merge and conservative performance pass

### Applying the fidelity fixes
- Applied the previous Release 1.2.5 fidelity changes to the newer project ZIP with a three-way source merge (`previous base -> fidelity fixes` against `previous base -> current ZIP`) rather than overwriting the current `src` tree.
- Preserved the newer platform/refactor work already present in the supplied ZIP. The 49 fidelity-touched source files merged without textual conflicts.

### Performance changes
- Changed the PS2/Wii mesh update scheduler from a full `std::sort` of every pending renderer to `std::partial_sort` of only the candidate prefix that can actually be attempted this frame. Candidate priority and the processed prefix remain identical; the unused tail is no longer sorted every frame.
- Reused one `SpawnerAnimals` eligible-chunk iteration snapshot for all creature categories in a spawn pass instead of reconstructing the same ordered vector once per category.
- Replaced `Pathfinder`'s dynamically allocated 32-entry `pathOptions` vector with inline `std::array` storage. The capacity remains the same as the translated Java array.
- Pre-reserved `Pathfinder::allocatedPoints` on bounded PS2/Wii pathfinding using the existing platform node budget, reducing vector growth during A* searches.
- Added `MCHash::reserveEntries()` and pre-sized the pathfinding hash table on bounded PS2/Wii profiles, avoiding repeated 16 -> 32 -> 64 -> ... rehash growth during a normal A* search. Hash keys, lookup semantics, and path ordering are unchanged.

### Investigated but intentionally deferred
- Wii can potentially use the existing `PLATFORM_FAST_CHUNK_BLOCK_READS` direct `ExtendedBlockStorage` path already used by PS2. It was not enabled here because Wii incremental mesh builds can span frames; cached section pointers need a dedicated lifetime/update audit first.
- Many hot paths currently perform `chunkExists()` followed immediately by `getChunkFromChunkCoords()`, causing two provider lookups. A non-generating `getLoadedChunk()` interface could remove the duplicate lookup, but it touches all chunk providers and must preserve last-access, async generation, population, and blank-chunk semantics. It is a good candidate for a separate focused pass.
- `Pathfinder` still allocates individual `PathPoint` and `MCHashEntry` objects. A pooled/intrusive pathfinding arena could reduce heap fragmentation further, but it is architectural enough to require dedicated validation instead of being bundled into this conservative pass.

### Validation
- The Release 1.2.5 static fidelity checks pass on the merged tree.
- New static checks for the mesh scheduler, spawn snapshot reuse, inline path options, bounded point reserve, and path hash pre-sizing pass.
- No project build was run automatically, in accordance with `CODESTYLE.MD`.

## 2026-08-25 - Deep Release 1.2.5 semantic and ownership audit (v11)

### Java integer/long semantics and persistent world state
- Extended `JavaArithmetic` use across world time, scheduled ticks, chunk population, chunk cache distances, entity IDs, village clocks, structure coordinates, biome `GenLayer` areas, region bookkeeping, and persistent world generators. These paths now reproduce Java 32-bit/64-bit wrap semantics instead of depending on signed C++ overflow.
- Added Java-compatible integer absolute-value handling, including the `Integer.MIN_VALUE` case, and removed unsafe `std::abs(x - y)` patterns from structure/chunk-distance paths where the subtraction itself could overflow first.
- Corrected scheduled tick serialization/deserialization and tick scheduling arithmetic so `worldTime`, saved `TileTicks`, client time, moon phase calculations, and `NextTickListEntry` IDs use Java long wrap semantics.
- Hardened `StructureBoundingBox`, `StructureComponent`, and `StructureStart` as central structure helpers. Bounding-box construction, offsets, sizes, centers, liquid-border expansion, coordinate transforms, and random-height calculations now preserve Java int semantics for villages, strongholds, mineshafts, and Nether structures.

### World generation fidelity
- Corrected signed-overflow-prone coordinate arithmetic in `GenLayer` island/zoom/river/shore/swamp/biome pipelines without changing seed mixing or RNG call order.
- Corrected persistent generators including normal/forest/swamp/huge trees, big trees, ores, clay, sand, reeds, cactus, pumpkins, vines, mushrooms, waterlilies, End spikes, and dragon-egg movement to use Java coordinate wrap and Java float/double-to-int conversion semantics where required.
- Restored the exact Java float operation ordering in cave/ravine reach checks. The previous algebraic simplification was not bit-equivalent and could change which cave/ravine nodes excavate with ordinary random values.
- Preserved PS2/Wii population scheduling and `BiomeDecorator` incremental work. Its private task RNG was verified to preserve the synchronous Java RNG sequence, so the console optimization was not removed.

### Numeric conversion and floating-point fidelity
- Restored Java-defined `float`/`double -> int` conversion behavior in terrain generation, explosions, instant potion effects, enchantments, potion durations, particles, village siege offsets, hash thresholds, and other audited gameplay paths.
- Restored Java float-before-double rounding in additional entities, particles, projectile collision margins, rendering bounds, Iron Golem/dragon behavior, and remote-player interpolation.
- Corrected `MathHelper::floor_float`, `floor_double`, `floor_double_long`, `abs(float)`, and `abs_max()` to match Java comparison behavior for NaN instead of using superficially equivalent inverted comparisons that produced different results.
- Corrected `MathHelper::getRandomIntegerInRange()` so range arithmetic follows Java int wrap instead of invoking signed C++ overflow.
- Corrected remaining Java `Math.PI`/float-PI distinctions where the translated type changed the result.

### Entity query ownership and reentrancy
- Restored Java's independent-list semantics for piston pushes, explosions, and splash-potion entity queries. The C++ `World` query helpers reuse scratch vectors; iterating those vectors while callbacks such as `moveEntity()` or `attackEntityFrom()` perform another AABB query could overwrite the collection being iterated. These callers now snapshot the result before invoking reentrant entity code.
- Restored village villager/Iron Golem counting through typed `World::getEntitiesWithinAABB()` queries and entity bounding-box intersection, replacing the non-equivalent manual `loadedEntityList` position test.
- Routed `EntityPlayer::alertWolves()` through the existing typed `EntityWolf` AABB query instead of scanning every nearby entity.
- Audited the remaining scratch-vector query callers; no additional reentrant case with the same high-confidence aliasing pattern was found in this pass.

### Chunks, saves, and region safety
- Corrected Java-wrap coordinate arithmetic around chunk population neighbors, deferred population, client chunk-distance checks, unload ages, and asynchronous population snapshots.
- Kept the stronger C++ chunk/Anvil payload validation already present in the port: malformed section arrays and truncated/overlapping region sectors are rejected rather than copied out of bounds.
- Kept explicit chunk/tile-entity unload ownership cleanup. The reviewed normal unload/destructor paths remove tile entities from world/render lists before destruction and did not show a new high-confidence double-free/UAF in this pass.
- Corrected region size/offset bookkeeping arithmetic that could otherwise depend on signed C++ overflow while retaining the port's copy-on-write sector safety logic.

### APIs, stubs, and false-positive review
- Verified that `Material::isLiquid()` already delegates to the translated `getIsLiquid()` override, so water/lava material behavior did not require another compatibility method.
- Verified that `EntityFishHook` functionality is present through the port's `EntityFish` naming, villager profession textures are selected through the C++ `getEntityTexture()` API, and `BlockFlowing` liquid movement semantics are supplied by the existing fluid hierarchy.
- Verified that the old five-argument `TexturedQuad` constructor is not used by vanilla model boxes in the current tree; Release 1.2.5 model geometry uses the texture-dimension-aware constructor, so the compatibility overload was retained rather than deleted.
- `FontRenderer::bidiFlag` remains a known fidelity gap. Java 1.2.5 relies on `java.text.Bidi`; this codebase has no ICU/FriBidi-equivalent implementation, so an incomplete custom bidi algorithm was intentionally not introduced.
- PS2 framebuffer screenshot readback and the disabled PS2 online HTTP path remain explicit platform limitations. The HTTP implementation is excluded from the current `NO_NETWORK` PS2 build and was not enabled as part of a vanilla fidelity fix.

### Validation
- Ran every accumulated v11 static regression plus the merged Release 1.2.5 fidelity and conservative performance suites on the final tree: 355 explicit PASS checks, 0 FAIL checks, and 0 failing test scripts.
- Relative to the v10 comparison tree, 101 C/C++ source/header files differ in this v11 pass. All changed C/C++ files passed the independent brace-balance scan; no merge markers or accidental nested replacement patterns were found.
- Validation remains source/static only. The project was not compiled automatically, in accordance with `CODESTYLE.MD`.

## 2026-08-25 - Deep ownership, collection-order and buffer-safety audit (v12)

### Java collection order and deterministic runtime behavior
- Added Java `HashSet`/`HashMap` iteration-order indexes where iteration is observable while retaining the existing C++ maps for fast lookup and ownership. This now covers client entity sets, active potion effects, `DataWatcher` metadata, scheduled ticks, structure starts, and biome-cache cleanup order.
- Corrected `WorldClient` entity-set equality to follow `Entity.equals()` / `entityId` semantics instead of pointer identity, and stabilized the server entity ID before inserting the C++ ownership entry so its hash cannot change while stored.
- Preserved Java scheduled-tick `HashSet` iteration when collecting `TileTicks` for chunk saves. Entries are removed from the parallel order index when executed or explicitly removed, so equal-time tick ordering after save/reload no longer depends on STL bucket order.
- Added a free-list to `JavaHashSet` so removed entries reuse storage. Long-running client/tick/potion/structure workloads are now bounded by peak live membership instead of total historical insertions.
- Preserved Java `HashMap<Long, StructureStart>` iteration for structure generation/query passes so shared `Random` consumption does not vary with `std::unordered_map` bucket order across PC/Wii/PS2.

### Entity and TileEntity lifecycle / reentrancy
- Restored Java list-removal order in `World::updateEntities()` instead of swap-with-back removal, preventing entity tick order from changing whenever an entity dies.
- Preserved `unloadedEntityList` insertion order while removing unloaded entities from the active list; the list is subsequently reused for chunk removal, skin release, and removal callbacks just as in Java.
- Restored `TileEntity` tick guards: `updateEntity()` now requires a valid tile entity, a world, and a loaded block/chunk. Invalid/pending cleanup no longer forces chunk generation just to remove/install a tile entity.
- Restored `markTileEntityForDespawn()` semantics with a deferred removal queue. The Java method only removes the tile entity from the ticking list after the scan; it does not invalidate/delete it.
- Added independent snapshots before reentrant entity callbacks in additional lightning/dragon/player collision paths where the C++ `World` query helpers reuse scratch vectors.

### Heap and buffer safety
- Added checked `BufferedImage` pixel/RGBA-size calculations, region validation, and guarded image copies. Negative/overflowed dimensions can no longer become undersized allocations followed by oversized writes.
- Hardened `RenderEngine` texture setup/subimage uploads against dimension overflow, pixel-count mismatch, and SDL surfaces with padded `pitch`; row copies now respect the source stride.
- Hardened screenshot allocation/stride/index calculations before RGB readback and PNG writes.
- Hardened PS2 PNG decode and texture upload/sub-upload bounds. Input length, pixel/byte counts and subrectangles are validated before allocation or touching CPU texture memory.
- Hardened Wii texture creation/sub-upload bounds, including GX `u16` width/height and `u8` mip-level representability plus 64-bit rectangle end checks.

### Java arithmetic, world tick and persistent generation
- Extended Java wrap/shift/conversion semantics through world active-chunk/random-tick logic, light/cave-sound probes, lightning, snow/ice, ray-trace decrements, entity update chunk ranges, health/damage/armor/resistance, experience/XP-cap math, entity IDs, pathfinding coordinates and navigation counters.
- Corrected `World::getSkyColor()` to use the biome's `getFloatTemperature()` path used by Release 1.2.5 instead of the non-equivalent manager temperature query.
- Corrected the liquid eye-height threshold to use the exact Java `1.0F / 9.0F` float result rather than a bit-different decimal approximation.
- Corrected sign/nametag scale to the exact `(float)(1.0D / 60.0D)` value used by Java.
- Hardened additional persistent world generation and biome sizing (`WorldGenLakes`, leaves, silverfish/chest coordinate paths, Overworld/Nether/End noise-volume allocation, biome-area counts) without changing normal RNG order or geometry.
- Corrected timer/system-time arithmetic and Java float-to-int conversion paths so clock wrap/NaN/out-of-range behavior no longer depends on C++ signed overflow or casts.

### API/stub and false-positive review
- Re-audited suspiciously short methods against `srcorigjava`. `EntityPlayer::getItemIcon`, `BlockCrops`, `World::tickUpdates`, `ChunkCache`, `RenderPlayer`, `BiomeCache`, and several obfuscated aliases were confirmed to retain their Release 1.2.5 logic through overloads/helpers and were not replaced.
- Confirmed `BiomeCache` now keeps insertion order during cleanup while retaining map ownership, matching the Java `LongHashMap + ArrayList` design.
- The known `FontRenderer` bidi gap remains intentionally deferred because Java relies on `java.text.Bidi` and the project has no equivalent Unicode bidi library; an incomplete home-grown implementation was not added.
- PS2 framebuffer screenshot readback and disabled PS2 online HTTP remain platform limitations rather than vanilla Release 1.2.5 translation fixes.

### Final v12 closure fixes
- Fixed an unreachable `TileTicks` load path in `AnvilChunkLoader::readChunkFromLevel()`: a duplicated unconditional `continue` discarded every saved scheduled tick before `scheduleBlockUpdateFromLoad()` could run. Pending redstone/fluid/block ticks now survive save/reload again.
- Restored Java list ownership for typed entity queries. `World::getEntitiesWithinAABB(Class, AABB)` now returns an independent vector like Java's `new ArrayList()`, instead of sharing the scratch vector used by `getEntitiesWithinAABBExcludingEntity()`. This removes a whole class of nested-query aliasing bugs without adding copies to the vanilla scratch-query path.
- Hardened `RegionFile::write()` so compressed-length and sector-count arithmetic stays in checked `size_t` until representability is proven. Oversized/corrupt payloads are rejected before narrowing to `int_t`; normal `.mca` bytes and sector layout are unchanged.
- Fixed scheduled-tick NBT loading and retained the stronger C++ section/region validation already present in the port; malformed data is rejected rather than copied into undersized buffers.

### Validation
- Fresh v11/v12 regression run: 65 scripts, 568 explicit PASS checks, 0 explicit FAIL checks, and 0 failing scripts.
- Fresh legacy/final suite run against the v12 tree: 6 script executions, 154 additional PASS checks, 0 FAIL checks, and 0 failing scripts. Some of these legacy harnesses intentionally overlap/duplicate one another, so their check count is reported separately rather than presented as unique coverage.
- Relative to the published v11 archive, 50 C/C++ source/header files differ in v12. Delta-based structural validation found no merge markers, no newly introduced net-brace imbalance, and no newly introduced unresolved local include.
- Validation remains source/static only. The project was not compiled automatically, in accordance with `CODESTYLE.MD`.

## 2026-08-25 - Deterministic collections, remaining worldgen and heap-safety audit (v13)

### Persistent world generation and Java arithmetic
- Extended `JavaArithmetic` through the remaining Nether/decorator generators audited in this pass: fire, glowstone, lava/liquids, shrubs, big mushrooms, desert wells, dungeons, and taiga trees. Coordinate offsets, neighbour probes, height math, and loop bounds no longer depend on signed C++ overflow, while RNG call order and normal generated geometry remain unchanged.
- Hardened `NibbleArrayReader` index/shift arithmetic and the six-neighbour relight coordinates in `Chunk` so Java wrap/shift semantics are preserved instead of relying on signed C++ operations.
- Corrected additional player combat/XP arithmetic where Java uses 32-bit wrap, including potion damage modifiers, sprint knockback, critical-damage accumulation, fire-aspect duration, and experience-point calculations.

### Java collection order and multiplayer determinism
- Restored Java `HashMap`/`HashSet` iteration semantics for `EntityList.entityEggs`, so callers iterating spawn-egg IDs no longer receive `std::map`'s sorted numeric order.
- Restored Java `HashMap<String, Short>` iteration for `MapStorage.idCounts` while retaining the existing `std::map` lookup table. `idcounts` NBT serialization now follows Java bucket order instead of lexicographic key order.
- Restored `WorldClient.entitySpawnQueue` retry ordering: an entity is removed from the Java-order set before a spawn attempt and a failed attempt re-inserts it, matching Release 1.2.5 `HashSet` behavior. The C++ non-generating chunk-reattach safety path remains intact.
- Corrected multi-block and map-chunk network coordinate additions to use Java int wrap after packet-local offsets/shifts.

### Save conversion and heap/buffer safety
- Replaced the fixed 1 MiB one-shot gzip buffer in `SaveConverterMcRegion` with checked incremental zlib inflation. Large valid legacy chunks are no longer silently truncated, and corrupt/truncated/non-progressing streams are rejected before region writes.
- Added representability checks before narrowing converted McRegion chunk payload sizes to `int_t` for `RegionFile::write()`.
- Hardened the flattened `ChunkCache` storage introduced by the C++ optimization: dimensions are validated before flattening, cell count is computed in 64-bit and checked before `size_t` allocation, one validated `chunkCellCount` is reused, and all flat indexing goes through a size-safe helper. Oversized/corrupt spans can no longer wrap into a too-small backing store or an accidental huge allocation.
- Changed `ChunkCache` constructor iteration to use Java-safe coordinate increments and to avoid incrementing signed `int` beyond the final endpoint. The existing non-generating missing-chunk behavior and PS2/Wii fast block-read cache remain preserved.

### Pathfinding and translated array growth
- Hardened `Path` heap growth/index arithmetic so the translated Java `count << 1` and child-index calculations use defined Java int semantics. Wrapped/non-growing capacities are rejected instead of becoming corrupt C++ allocations or indices.
- Kept the existing bounded pathfinder reserves and inline path option storage from the previous performance pass; this v13 change only addresses arithmetic/heap correctness around the heap container itself.

### Audit notes
- Re-audited suspicious float constants in boats, minecarts, dispensers, projectiles, animals, Nether caves, dragons, and Hell provider code. Values such as `0.25F`, `8.0F`, `32.0F`, and several `0.5D`/`0.2D` candidates were confirmed bit/type-correct against `srcorigjava` and intentionally left unchanged.
- Re-audited remaining `std::map`/`unordered_*` containers for observable iteration. `MapData`, `FurnaceRecipes`, NBT compounds, potion effects, scheduled ticks, structures, biome cache, and `DataWatcher` either already preserve Java order through a parallel index/vector or use the map only for lookup; no blanket container replacement was performed.
- Kept the stronger C++ bounds/ownership protections in packet decompression, chunk loading, region files, and console backends where literal Java behavior would be less safe.
- Known deferred gaps remain unchanged: full Unicode bidi ordering still needs a `java.text.Bidi`-equivalent implementation; PS2 framebuffer screenshot readback and the disabled PS2 online HTTP path remain explicit platform limitations.

### Validation
- Fresh accumulated static regression run on the v13 tree: 74 scripts, 700 explicit PASS checks, 0 FAIL checks, and 0 failing scripts (v11: 314, v12: 254, v13: 91, final fidelity/performance suites: 41).
- Relative to the published v12 tree, 24 C/C++ source/header files differ in v13. Independent structural validation found 0 merge markers, 0 lexical delimiter imbalances, and 0 newly unresolved local includes in those files.
- Validation remains source/static only. The project was not compiled automatically, in accordance with `CODESTYLE.MD`.

## 2026-08-25 - Noise-volume, serialization and stable-order audit (v14)

### Heap/buffer safety in images, regions, and noise fields
- Hardened `ImageBufferDownload` skin/image copies by reusing `BufferedImage::checkedRgbaByteCount()` instead of evaluating `width * height * 4` in signed `int` before allocation. Invalid or overflowed dimensions can no longer become an undersized RGBA buffer followed by an oversized copy.
- Added the shared `checkedNoiseVolumeSize()` helper and routed Overworld, Nether, End, and octave-noise allocations through it. The helper validates each dimension and the true 64-bit mathematical volume before narrowing to `size_t`; this catches both negative overflow and the more dangerous positive-wrap case that could otherwise allocate a small vector while the generation loops write the full volume.
- `NoiseGeneratorOctaves` now rejects negative octave counts before `reserve()` can reinterpret them as a huge unsigned allocation.
- Hardened `RegionFileChunkBuffer::write()` against negative lengths, null non-empty sources, unrepresentable growth, and pointer arithmetic before validation. `close()` now validates representability before narrowing the backing vector size to the `int_t` length accepted by `RegionFile`.

### Java numeric and noise semantics
- Replaced the remaining manual Perlin floor conversions with the already Java-compatible `MathHelper::floor_double()`. This preserves Java saturation/NaN behavior and avoids decrementing an out-of-range C++ cast while keeping ordinary noise values and RNG sequencing unchanged.
- Preserved all normal terrain dimensions and noise ordering; the stronger volume checks only reject sizes that cannot be represented safely by the translated Java/C++ array contract.

### Serialization and deterministic collection behavior
- Restored `DataOutputStream.writeUTF`'s Modified UTF-8 payload limit: encoded strings larger than 65535 bytes are rejected before writing the 16-bit length. The previous C++ code truncated the length field but still wrote the entire payload, which could desynchronize subsequent NBT data.
- Replaced non-stable `std::sort` with `std::stable_sort` where the Java source uses stable `Collections.sort`: nearest-attackable-target selection, world selection, statistics slots, and profiler results. Equal-distance AI targets now preserve their original order instead of depending on STL/platform tie reordering.
- Kept custom RenderGlobal sorting paths unchanged where the port intentionally supplies strict-weak-order/tie-break behavior for the console renderer rather than directly mirroring a Java `Collections.sort` call.

### Audit notes
- Re-audited `MapData` player bookkeeping. Iteration already follows the translated `playersOnMap` list (matching Java's `ArrayList`) and the map is lookup-only; the unusual `doesXZShowFog()` call is a compatibility alias for `hasItemStack()`, so no change was required.
- Re-audited `ChunkProviderGenerateLite` constructor/state candidates reported by initialization scans; `seed`, `offX`, and `offZ` are initialized/assigned before their first read, so no speculative initialization change was made.
- Re-audited entity damage-source wrappers and remaining dynamic-array candidates; no additional high-confidence lost override or uninitialized `new[]` buffer was found in this pass.
- Known deferred gaps remain unchanged: full Unicode bidi ordering still needs a `java.text.Bidi`-equivalent implementation; PS2 framebuffer screenshot readback and the disabled PS2 online HTTP path remain explicit platform limitations.

### Validation
- Fresh accumulated static/source regressions on the final v14 source tree: 81 scripts, 736 explicit PASS checks, 0 explicit FAIL checks, and 0 failing scripts (v11: 314, v12: 254, v13 + final suites + v14: 168).
- Relative to the published v13 archive, 13 C/C++ source/header files differ in v14. Independent structural validation found 0 merge markers, 0 lexical delimiter imbalances, and 0 unresolved local includes in those files.
- Validation remains source/static only. The project was not compiled automatically, in accordance with `CODESTYLE.MD`.

## 2026-08-25 - Stable entity order, biome-area and region-write safety audit (v15)

### Java collection/list ordering
- Removed the premature `std::sort` from `AnvilSaveConverter::getSaveList()`. Release 1.2.5 returns the save-directory enumeration order there and applies the stable `Collections.sort` later in `GuiSelectWorld`; preserving that split keeps the Java tie order for worlds with equal timestamps.
- Restored stable `ArrayList` removal semantics in the runtime `World::updateEntityList()` path. Unloaded/dead entities are erased without swap-with-back, so surviving entities keep their relative Java tick/callback order.
- Kept the Java-order collection indexes added in earlier passes, while adding capacity/index guards to `JavaHashSet` so pathological growth cannot silently narrow a vector index to `int` or blindly exceed the Java `HashMap` maximum bucket capacity.

### Biome and GenLayer heap safety
- Added 64-bit mathematical validation to `WorldChunkManager::checkedBiomeAreaCount()`. Negative dimensions and areas above the Java array/int range are rejected before conversion to `size_t`, including positive-wrap cases that a 32-bit-only overflow check could miss.
- Added `IntCache::checkedAreaSize(width, height)` and routed all audited 2D `GenLayer` result allocations through it. A wrapped positive `width * height` can no longer produce an undersized array followed by out-of-bounds layer writes.
- `IntCache::getIntCache()` now rejects negative requested sizes and avoids narrowing vector sizes during capacity comparisons.
- Added consumer-side validation in `WorldChunkManager` so a malformed/custom `GenLayer` result or undersized cached biome block is rejected before indexing/copying. Normal Release 1.2.5 dimensions, seeds, RNG order, and biome output are unchanged.

### Region-file durability
- Corrected the whole-file-buffer `RegionFile::close()` path used by console storage so `flushed` is set only after a successful platform write. A failed write therefore remains retryable instead of being silently treated as already persisted.
- Any successful new region mutation now clears `flushed` again when it marks the region dirty, allowing a reused `RegionFile` to be written after an earlier close/flush.
- Retained the stronger copy-on-write sector ownership and malformed-region checks from earlier passes; this change only fixes the dirty/flushed state transition.

### Audit notes
- Re-audited `BiomeCache`; its current C++ implementation already keeps the Java `LongHashMap + ArrayList` insertion/cleanup ordering and Java long elapsed-time arithmetic, so no replacement was needed.
- Re-audited remaining obvious persistent `float/double -> int` casts, dynamic products, and `memcpy` sites after the v14 buffer/noise fixes. The remaining high-visibility direct casts found by this pass were HUD/particle paths or already bounded operations; no additional high-confidence persistent corruption fix was applied blindly.
- Kept console render/population optimizations, Beta++/OptiFine UI changes, and existing C++ safety checks intact where they do not change Release 1.2.5 observable semantics.

### Validation
- Fresh accumulated static regression run on the final v15 tree: 88 scripts, 778 explicit PASS checks, 0 FAIL checks, and 0 failing scripts.
- Relative to the published v14 archive, 22 C/C++ source/header files differ in v15. Independent delta validation found 0 merge markers, 0 lexical delimiter imbalances, and 0 unresolved local includes in those files.
- Validation remains source/static only. The project was not compiled automatically, in accordance with `CODESTYLE.MD`.

## 2026-08-25 - C++ lifetime, entity-state and desktop input audit (v16)

### C++ stream lifetime / undefined behavior
- Corrected `MusInputStream` construction so the `std::istream` base is no longer initialized with the address of the `MusStreamBuf` member before that member has been constructed. The base now starts with a null buffer and receives `rdbuf(&streamBuf)` after member construction.
- Applied the same lifetime fix to the PC and Wii socket input/output stream wrappers in `JavaNetwork.cpp` / `JavaNetwork_wii.cpp`. Base stream objects are initialized with `nullptr` and connected to their member streambuf only after construction. This removes C++ lifetime UB from packet/socket I/O without changing framing, buffering, or flush behavior.
- Re-audited the remaining custom stream wrappers. The chunk/region loaders already use the safe null-base-plus-`rdbuf()` construction pattern; no remaining `std::istream/std::ostream` base initialization from an unconstructed member streambuf was found.

### Entity base-state fidelity
- Restored Release 1.2.5 `Entity::velocityChanged`. The translated `setBeenAttacked()` had been writing a C++-only `beenAttacked` flag even though Java sets `velocityChanged`; the Java field is now present, initialized to `false`, and updated by `setBeenAttacked()`.
- Restored the missing `isAirBorne = true` side effect in `Entity::addVelocity()`. Java marks an entity airborne whenever velocity is added, while the previous C++ translation only changed the three motion components.
- Retained the existing compatibility `beenAttacked` member rather than deleting adjacent legacy code, but vanilla behavior now uses the restored `velocityChanged` contract.
- Audited the Java-constructor `entityInit()` dispatch workaround. All 16 concrete entity classes with their own non-empty `entityInit()` currently reach `ensureEntityInit()` after base construction; abstract base classes continue to defer final dispatch to their concrete descendants.

### Desktop GUI / legacy applet semantics
- Made clipboard support explicit on desktop builds by including the SDL clipboard API directly and guarding it by platform instead of relying on an incidental `SDL_VERSION` macro from transitive headers. `GuiScreen::getClipboardString()` / `setClipboardString()` therefore no longer silently compile into no-ops on PC when that macro is absent. PS2/Wii retain the intentional no-system-clipboard behavior.
- Restored Java `Integer.parseInt`-style validation for the legacy applet server port. Invalid or overflowing port text is rejected instead of being accepted as zero or narrowed from `strtol` into an implementation-dependent `int` value.

### Audit notes
- Re-ran the primitive/member initialization audit after restoring `velocityChanged`. Remaining reported candidates were initializer-list, nested-member, or explicit-default paths rather than a new high-confidence uninitialized vanilla primitive.
- Re-audited suspicious constructor virtual-dispatch cases instead of blindly adding `ensureEntityInit()` calls. Delegating constructors and abstract entity bases account for the scanner false positives; no additional concrete entity with a non-empty own `entityInit()` was found without the post-base dispatch helper.
- Existing Beta++/OptiFine UI changes, PS2/Wii render/population tuning, stronger chunk/region validation, and platform-specific ownership safeguards remain intact.

### Validation
- Fresh accumulated source/static regression run on the final v16 tree: 97/97 regression scripts completed without a failing script (v11: 26/26, v12: 39/39, v13-v16 plus current final suites: 32/32). The six v16-specific regressions contribute 30 explicit PASS checks and 0 FAIL checks.
- The final v16 tree differs from the published v14 comparison tree in 29 C/C++ source/header files; v16 itself adds changes to 7 source/header files on top of the v15 working tree. Independent structural validation found 0 merge markers, 0 delimiter-balance errors, and 0 unresolved quoted local includes in the changed C/C++ set.
- Validation remains source/static only. The project was not compiled automatically, in accordance with `CODESTYLE.MD`.
