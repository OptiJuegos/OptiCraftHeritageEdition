# Remove Beta World Generation Compatibility Design

## Goal

Make Minecraft 1.2.5 the only gameplay and world-generation implementation in the C++ port. Remove dead Beta biome, noise, Sky-dimension, and compatibility code without removing legitimate Minecraft 1.2.5 symbols or the PS2 incremental population budget.

## Scope

This change removes:

- The manual legacy decoration stages in `ChunkProviderGeneratePopulatePs2.cpp`.
- The Beta-only biome aliases `rainforest`, `seasonalForest`, `savanna`, `shrubland`, `iceDesert`, and `tundra`.
- `BiomeGenBase::biomeLookupTable` and its legacy lookup methods.
- `BiomeGenRainforest` and `BiomeGenSky`.
- `NoiseGenerator2` and `NoiseGeneratorOctaves2`.
- `WorldProviderSky` and `ChunkProviderSky`.
- The `MCBETA_JAVA_PARITY` CMake option, macro, overrides, and Beta differential-test comments.

This change preserves legitimate Minecraft 1.2.5 symbols:

- `BiomeGenBase::plains`, `desert`, and `taiga`.
- `BiomeGenBase::sky`, which is the Java 1.2.5 registry field holding a `BiomeGenEnd` instance.
- `WorldProviderEnd` and `ChunkProviderEnd`.
- Platform memory and scheduling capabilities that do not change Minecraft rules.

## Architecture

### Single 1.2.5 biome-decoration implementation

`BiomeDecorator` becomes the only implementation of biome decoration. It exposes an incremental lifecycle in addition to its existing synchronous entry point:

- `beginDecoration(World *, Random &, int_t, int_t)` initializes one decoration operation.
- `advanceDecoration()` executes one bounded unit of work and reports completion.
- `finishDecoration()` clears borrowed operation state.
- `decorate(...)` calls the same lifecycle repeatedly until completion.

The state machine follows the Java 1.2.5 `BiomeDecorator.decorate()` order exactly: ores, sand/clay patches, trees, mushrooms, flowers, grass, dead bushes, water lilies, reeds, pumpkins, cacti, water springs, and lava springs. Each RNG draw remains in the same left-to-right order as Java. A work unit is one generator attempt, except for constant-time state transitions.

Only one decoration may be active on a `BiomeDecorator`. Starting another operation while one is active remains an error, matching the current reentrancy guard.

### Shared chunk-population sequence

`ChunkProviderGenerate` retains the Java 1.2.5 population sequence:

1. Mineshaft, village, and stronghold generation.
2. Water and lava lakes.
3. Eight dungeon attempts.
4. The biome's `BiomeDecorator` operation.
5. World-generation creature spawning.
6. Ice and snow placement.

Desktop and Wii execute this sequence synchronously. PS2 keeps `populateStep()`, but its task only coordinates these shared phases and advances `BiomeDecorator`; it contains no duplicate ore, tree, plant, or spring rules.

The PS2 scheduler may limit how many work units run per call, but may not change counts, ordering, biomes, RNG consumption, or generator types.

## Deletion and build integration

The dead class header/source pairs are deleted from `src/net/minecraft/src`. Because builds discover sources recursively, no replacement exclusion entries are needed. All includes, forward declarations, scripts, and generated/manual tracking references that treat those classes as active C++ components are updated.

`MCBETA_JAVA_PARITY` is removed rather than renamed. `PLATFORM_CONSOLE_LOW`, `PLATFORM_BOUNDED_WORLD`, and narrowly named platform capabilities remain independent. Existing optimization switches are not automatically disabled or enabled by this cleanup; any optimization that changes 1.2.5 gameplay requires a separate explicit decision.

## Ownership and failure handling

The decoration task borrows `World`, `Random`, and `BiomeGenBase` for the duration of one operation. Generator ownership remains in `BiomeDecorator` through its existing `std::unique_ptr` members. Temporary biome-selected generators continue to use the existing release/ownership contract.

If a generator throws, the decorator clears all borrowed pointers and returns to an inactive state before propagating the exception. PS2 task cancellation also clears the active decorator operation so a later chunk cannot inherit stale state.

## Compatibility consequences

Removing the Beta-only classes intentionally drops source compatibility for code that directly references them. It does not change the numeric IDs or names of registered Minecraft 1.2.5 biomes. Existing 1.2.5 Anvil worlds remain compatible.

The old `WorldProviderSky` and `ChunkProviderSky` are not used to load the End; the active dimension path already selects `WorldProviderEnd` and `ChunkProviderEnd`.

## Verification

Verification is test-first and does not automatically compile the project, per repository rules.

Static regression checks must prove:

- All removed class files and class names are absent from production source.
- All six Beta biome aliases and the lookup table API are absent.
- `BiomeGenBase::sky` still registers a `BiomeGenEnd` at ID 9.
- Dimension 1 still selects `WorldProviderEnd` and `ChunkProviderEnd`.
- `MCBETA_JAVA_PARITY` has no remaining references.
- PS2 population contains no manual biome comparisons or duplicated decoration counts.
- Both synchronous and incremental paths call the same `BiomeDecorator` state machine.
- Local includes resolve and modified files have balanced structure.

Manual runtime verification should cover the same seeds on desktop and PS2, including normal terrain, superflat, negative chunk coordinates, villages, ores, biome vegetation, springs, snow, and entry into the End.

## Non-goals

- Reintroducing Beta 1.7.3 seed parity.
- Removing legitimate Minecraft 1.2.5 legacy-named fields.
- Redesigning console rendering, storage, chunk caching, or memory budgets.
- Changing world-generation counts to improve performance.
