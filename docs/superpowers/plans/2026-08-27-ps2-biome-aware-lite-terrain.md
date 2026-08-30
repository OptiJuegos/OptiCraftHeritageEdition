# PS2 Biome-Aware Lite Terrain Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make PS2 Lite terrain respond to Minecraft 1.2.5 biome height parameters without adding double-precision work.

**Architecture:** Add a small header-only terrain-shape component that blends a 5 by 5 biome neighborhood and evaluates an all-float height. Load a 20 by 20 biome halo in `ChunkProviderGenerate`, pass it to the Lite generator, and leave all downstream biome surface, carving, structure, and population stages intact.

**Tech Stack:** C++17, CMake test targets, existing Minecraft 1.2.5 biome and PS2 platform abstractions.

**Spec:** `docs/superpowers/specs/2026-08-27-ps2-biome-aware-lite-terrain-design.md`

## Global Constraints

- Production identifiers and comments are English.
- Preserve existing RNG call order.
- Do not compile the project automatically.
- Do not use Git commands or create commits.
- Do not change the downstream structure or decoration pipeline.

---

### Task 1: Specify Lite terrain-shape behavior

**Files:**
- Create: `tests/LiteTerrainShapeTests.cpp`

**Interfaces:**
- Consumes: `LiteTerrain::BiomeShape`, `blendBiomeShape`, and `surfaceHeight` from the next task.
- Produces: Behavioral coverage for constant blending, water biomes, and biome-dependent relief.

- [ ] Write standalone tests using `MC_LOG_ERROR` proving a constant 5 by 5 neighborhood is preserved, ocean and river profiles stay below Y=63 at maximum positive noise, neutral plains remain above sea level, and hills have more relief than plains.
- [ ] Keep the focused test standalone instead of extending the global CMake test list.
- [ ] Confirm the test cannot compile yet because `LiteTerrainShape.h` does not exist; do not invoke the compiler under repository policy.

### Task 2: Implement reusable all-float terrain shaping

**Files:**
- Create: `src/net/minecraft/src/LiteTerrainShape.h`

**Interfaces:**
- Produces: `LiteTerrain::BiomeShape`, `blendBiomeShape(int, int, Getter)`, and `surfaceHeight(float, float, float, const BiomeShape&)`.

- [ ] Implement the fixed 5 by 5 blending kernel using only float arithmetic.
- [ ] Implement biome base elevation and relief scaling with explicit clamps.
- [ ] Perform a static signature and literal audit to ensure the hot calculation introduces no `double` literals or calls.

### Task 3: Feed a cross-chunk biome halo into Lite generation

**Files:**
- Modify: `src/net/minecraft/src/ChunkProviderGenerate.h`
- Modify: `src/net/minecraft/src/ChunkProviderGenerate.cpp`
- Modify: `src/net/minecraft/src/ChunkProviderGenerateLite.cpp`

**Interfaces:**
- Consumes: `LiteTerrainShape.h` and the existing `WorldChunkManager::loadBlockGeneratorData` API.
- Produces: Biome-aware 16 by 16 terrain columns derived from a 20 by 20 halo.

- [ ] Add a reusable biome-halo vector to `ChunkProviderGenerate` only for the Lite configuration.
- [ ] Load the halo from world coordinates `(chunk * 16 - 2)` with dimensions 20 by 20.
- [ ] Pass the halo and stride explicitly to `generateTerrainHeightmap`.
- [ ] Blend the biome `minHeight` and `maxHeight` for each column and feed the result into `LiteHeightNoise::surfaceY`.
- [ ] Keep temperature lookup, biome surface replacement, carving, structures, and population in their current order.
- [ ] Enable the selective float/Lite defaults without enabling the decoration-reducing master profile.

### Task 4: Static verification and handoff

**Files:**
- Inspect all modified files.

**Interfaces:**
- Consumes: Completed implementation and test source.
- Produces: Verification evidence that does not compile the project.

- [ ] Search the Lite height path for accidental `double` arithmetic.
- [ ] Verify all halo accesses remain within the 20 by 20 allocation.
- [ ] Verify calls to caves, ravines, villages, mineshafts, strongholds, and population are unchanged.
- [ ] Provide a direct standalone test command for manual execution without modifying the global CMake test list.
