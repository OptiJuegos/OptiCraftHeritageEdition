# Remove Beta World Generation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the PS2 Beta population duplicate with the shared Minecraft 1.2.5 decorator and remove all identified dead Beta compatibility components.

**Architecture:** `BiomeDecorator` owns one resumable Java 1.2.5 decoration state machine. Its synchronous entry point drains that state machine, while PS2 advances it one bounded generator attempt per population step; `ChunkProviderGenerate` remains responsible only for chunk-level phases around biome decoration.

**Tech Stack:** C++17, CMake, PowerShell static regression checks, Java 1.2.5 reference sources.

**Spec:** `docs/superpowers/specs/2026-08-24-remove-beta-worldgen-design.md`

## Global Constraints

- Preserve the exact Java 1.2.5 decoration order, counts, coordinates, and RNG consumption.
- Keep `BiomeGenBase::sky` registered as `BiomeGenEnd` at biome ID 9.
- Keep PS2 population incremental without duplicating decoration rules.
- Remove `MCBETA_JAVA_PARITY`; do not rename or replace it.
- Do not compile automatically, inspect Git state, commit, or reformat adjacent code.
- Production identifiers and comments are English and retain the existing code style.

---

### Task 1: Static regression contract

**Files:**
- Create: `tests/RemoveBetaWorldgenChecks.ps1`

**Interfaces:**
- Consumes: repository production sources and CMake configuration.
- Produces: a standalone static regression command that exits nonzero for any surviving Beta component or missing shared incremental decorator contract.

- [ ] **Step 1: Write the failing static regression test**

Create a PowerShell check which asserts the obsolete eight header/source pairs are absent, forbidden identifiers are absent from `src` and `CMakeLists.txt`, `BiomeGenBase::sky` and End-provider routing remain, PS2 has no manual decorator generators or Beta biome comparisons, and the three decorator lifecycle methods exist and are used by both synchronous and PS2 paths.

- [ ] **Step 2: Run the test to verify RED**

Run: `powershell -NoProfile -ExecutionPolicy Bypass -File tests/RemoveBetaWorldgenChecks.ps1`

Expected: exit 1 listing current legacy files, aliases, parity macro, and missing incremental lifecycle.

- [ ] **Step 3: Keep the test unchanged while production is implemented**

The production change that makes this test pass is removal of the listed compatibility surface plus use of `beginDecoration`, `advanceDecoration`, and `finishDecoration` in production.

---

### Task 2: Shared incremental Minecraft 1.2.5 decorator

**Files:**
- Modify: `src/net/minecraft/src/BiomeDecorator.h`
- Modify: `src/net/minecraft/src/BiomeDecorator.cpp`

**Interfaces:**
- Consumes: `World`, `Random`, `BiomeGenBase`, and existing owned `WorldGenerator` instances.
- Produces: `void beginDecoration(World *, Random &, int_t, int_t)`, `bool advanceDecoration()`, `void finishDecoration()`, and synchronous `void decorate(World *, Random &, int_t, int_t)`.

- [ ] **Step 1: Add the resumable operation state**

Add a private `DecorationStage` enum covering ores through lava springs, an operation index, cached tree count, and helpers for advancing stages and clearing borrowed state. Starting while active throws `std::runtime_error("Already decorating!!")`.

- [ ] **Step 2: Implement one-attempt advancement**

Move each Java 1.2.5 loop body into its corresponding `advanceDecoration()` stage. One call performs at most one generator attempt; zero-work transitions may continue internally. Cache the random bonus tree count exactly once. Preserve generator release/ownership behavior.

- [ ] **Step 3: Make the synchronous entry point drain the same machine**

`decorate(...)` calls `beginDecoration(...)`, loops until `advanceDecoration()` reports completion, and calls `finishDecoration()` on both success and exception. No second decoration algorithm remains.

- [ ] **Step 4: Run the static test and inspect expected remaining failures**

Run the Task 1 command. Expected: lifecycle-related failures disappear; legacy PS2/files/config failures remain.

---

### Task 3: PS2 population coordinator

**Files:**
- Modify: `src/net/minecraft/src/ChunkProviderGenerate.h`
- Modify: `src/net/minecraft/src/ChunkProviderGeneratePopulatePs2.cpp`

**Interfaces:**
- Consumes: `BiomeGenBase::theBiomeDecorator`, `BiomeDecorator::beginDecoration`, `advanceDecoration`, and `finishDecoration`.
- Produces: a PS2 state machine containing only `Structures`, `WaterLake`, `LavaLake`, `Dungeons`, `BiomeDecoration`, `Spawning`, `Snow`, and `Done`.

- [ ] **Step 1: Reduce PS2 task state**

Delete manual ore/vegetation/spring stages and their counters from `PopulateStage` and `PopulateTask`. Keep chunk coordinates, biome, village result, seeded `Random`, profiling duration, and the shared phase index.

- [ ] **Step 2: Advance the shared decorator**

On first entry to `BiomeDecoration`, begin the biome decorator operation. On later calls, invoke one `advanceDecoration()` work unit. Finish and transition to spawning only after completion. Ensure `finishPopulateTask()` clears an active decorator operation before resetting the task.

- [ ] **Step 3: Remove obsolete includes and manual implementation**

Retain only includes needed for chunk-level structures, lakes, dungeons, spawning, snow, timing, and the shared decorator. Delete every manual biome comparison and generator count.

- [ ] **Step 4: Run the static test and inspect expected remaining failures**

Run the Task 1 command. Expected: PS2 duplication failures disappear; dead-file/config/biome failures remain.

---

### Task 4: Remove dead Beta biome surface

**Files:**
- Modify: `src/net/minecraft/src/BiomeGenBase.h`
- Modify: `src/net/minecraft/src/BiomeGenBase.cpp`
- Delete: `src/net/minecraft/src/BiomeGenRainforest.h`
- Delete: `src/net/minecraft/src/BiomeGenRainforest.cpp`
- Delete: `src/net/minecraft/src/BiomeGenSky.h`
- Delete: `src/net/minecraft/src/BiomeGenSky.cpp`

**Interfaces:**
- Consumes: current 1.2.5 biome registry.
- Produces: only the Java 1.2.5 biome registry and lookup API, including `sky = new BiomeGenEnd(9)`.

- [ ] **Step 1: Remove aliases and lookup table declarations**

Delete the six Beta alias fields, `biomeLookupTable`, and its legacy lookup methods from the header and implementation, including initialization and shutdown assignments.

- [ ] **Step 2: Remove obsolete classes and include**

Delete the four class files and the unused `BiomeGenSky.h` include. Keep `BiomeGenEnd` registration and all numeric biome IDs unchanged.

- [ ] **Step 3: Run the static test and inspect expected remaining failures**

Run the Task 1 command. Expected: biome failures disappear; dead noise/Sky provider and parity failures remain.

---

### Task 5: Remove dead generators and Sky dimension

**Files:**
- Delete: `src/net/minecraft/src/NoiseGenerator2.h`
- Delete: `src/net/minecraft/src/NoiseGenerator2.cpp`
- Delete: `src/net/minecraft/src/NoiseGeneratorOctaves2.h`
- Delete: `src/net/minecraft/src/NoiseGeneratorOctaves2.cpp`
- Delete: `src/net/minecraft/src/WorldProviderSky.h`
- Delete: `src/net/minecraft/src/WorldProviderSky.cpp`
- Delete: `src/net/minecraft/src/ChunkProviderSky.h`
- Delete: `src/net/minecraft/src/ChunkProviderSky.cpp`
- Modify: `scripts/c2027.tsv`
- Modify: `src/ps2/render/Ps2Tuning.h`

**Interfaces:**
- Consumes: active End routing through `WorldProviderEnd` and `ChunkProviderEnd`.
- Produces: no production or active tracking references to removed Beta classes.

- [ ] **Step 1: Delete dead class files**

Remove all eight noise/provider header/source files after confirming their only production references are self-contained.

- [ ] **Step 2: Remove active tracking references**

Delete obsolete rows from `scripts/c2027.tsv` and rewrite the PS2 tuning comment so it describes biome noise generically without naming removed classes.

- [ ] **Step 3: Run the static test and inspect expected remaining failures**

Run the Task 1 command. Expected: dead-class failures disappear; parity configuration failures remain.

---

### Task 6: Remove Beta parity configuration

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `src/platform/PlatformConfig.h`
- Modify: `src/platform/PlatformTuning.h`

**Interfaces:**
- Consumes: independently named platform capability macros.
- Produces: no `MCBETA_JAVA_PARITY` option, define, default, conditional override, or Beta parity comments.

- [ ] **Step 1: Remove the CMake option**

Delete the option and global compile-definition block at the top of `CMakeLists.txt`.

- [ ] **Step 2: Remove platform overrides**

Delete the macro default and both parity conditionals. Preserve ordinary PS2/Wii defaults and bounded-world implications unchanged.

- [ ] **Step 3: Run the complete static regression test for GREEN**

Run: `powershell -NoProfile -ExecutionPolicy Bypass -File tests/RemoveBetaWorldgenChecks.ps1`

Expected: exit 0 with every check reported as passed.

---

### Task 7: Final non-build verification

**Files:**
- Verify: all modified production and test files.

**Interfaces:**
- Consumes: completed Tasks 1-6.
- Produces: fresh evidence for removal completeness, include resolution, balanced preprocessor structure, and exact Java decoration ordering.

- [ ] **Step 1: Run forbidden-reference scans**

Run focused `rg` scans over `src`, `scripts`, and `CMakeLists.txt`, excluding historical update documents and the design/plan documents. Expected: no forbidden production references.

- [ ] **Step 2: Compare decorator stage order with Java**

Read `srcorigjava/net/minecraft/src/BiomeDecorator.java` beside the C++ stage sequence and verify every RNG expression and generator attempt remains in order.

- [ ] **Step 3: Check local includes and structural balance**

Use a read-only PowerShell check for quoted include targets and balanced braces/preprocessor directives in modified C++ files. Expected: no unresolved local includes or imbalances.

- [ ] **Step 4: Re-run the static regression suite**

Run the Task 1 command again. Expected: exit 0. Do not compile automatically.
