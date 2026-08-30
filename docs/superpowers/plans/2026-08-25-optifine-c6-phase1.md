# OptiFine 1.2.5 HD C6 Phase 1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Complete the existing OptiFine C6 settings layer and low-risk runtime toggles while preserving PS2/Wii constraints and excluding the user-declared unsupported systems.

**Architecture:** Extend the existing `GameSettings` + `EnumOptions` + `Config` path rather than adding a parallel configuration layer. Runtime consumers use `Config` so platform-specific backends remain isolated. GUI additions follow the existing OptiFine sub-screen pattern already used by Details/Animations/Other.

**Tech Stack:** C++17-style existing project code, Minecraft 1.2.5 port, RenderAPI abstraction, Python source-regression tests.

**Spec:** `docs/superpowers/specs/2026-08-25-optifine-c6-phase1-design.md`

## Global Constraints

- Do not compile automatically.
- Do not use git or create commits.
- Preserve existing console tuning and current saved option compatibility.
- Exclude VersionCheckThread/update checking, TextureHD/mipmaps, Forge reflection, LWJGL display modes, PC-specific fullscreen mode selection, and Show Capes.

---

### Task 1: Settings and Config parity

**Files:**
- Modify: `src/net/minecraft/src/GameSettings.h`
- Modify: `src/net/minecraft/src/GameSettings.cpp`
- Modify: `src/net/minecraft/src/EnumOptions.h`
- Modify: `src/net/minecraft/src/EnumOptions.cpp`
- Modify: `src/net/minecraft/src/Config.h`
- Modify: `src/net/minecraft/src/Config.cpp`
- Test: `tests/optifine_phase1_source_test.py`

- [ ] Write source-regression expectations for all new fields, enum options, Config queries, and options.txt keys.
- [ ] Run the test and verify it fails because Phase 1 symbols are missing.
- [ ] Add defaults, option cycling, display strings, load/save persistence, Config queries, and `setAllAnimations`.
- [ ] Run the source-regression test and verify the settings assertions pass.

### Task 2: C6 option screens

**Files:**
- Create: `src/net/minecraft/src/GuiPerformanceSettingsOF.h`
- Create: `src/net/minecraft/src/GuiPerformanceSettingsOF.cpp`
- Create: `src/net/minecraft/src/GuiQualitySettingsOF.h`
- Create: `src/net/minecraft/src/GuiQualitySettingsOF.cpp`
- Modify: `src/net/minecraft/src/GuiAnimationSettingsOF.cpp`
- Modify: `src/net/minecraft/src/GuiDetailSettingsOF.cpp`
- Modify: `src/net/minecraft/src/GuiOtherSettingsOF.cpp`
- Modify: `src/net/minecraft/src/GuiVideoSettings.cpp`
- Test: `tests/optifine_phase1_source_test.py`

- [ ] Add failing source assertions for C6 screen files, option arrays, and Video Settings routing.
- [ ] Add the two missing screens and expand existing screens using C6 option groupings minus excluded features.
- [ ] Add Animation All ON / All OFF buttons.
- [ ] Re-run source tests.

### Task 3: Runtime toggle hooks

**Files:**
- Modify: `src/net/minecraft/src/RenderGlobal.cpp`
- Modify: `src/net/minecraft/src/EntityRenderer.cpp`
- Modify: `src/net/minecraft/src/Profiler.cpp` or its existing caller only if needed
- Test: `tests/optifine_phase1_source_test.py`

- [ ] Add failing source assertions for particle gates, Sun/Moon rendering gate, Depth Fog gate, and profiler assignment.
- [ ] Gate only the matching particle classes in `RenderGlobal::doSpawnParticle`.
- [ ] Gate sunrise/sunset and sun/moon geometry with `Config::isSunMoonEnabled`, leaving Stars independent.
- [ ] Apply Depth Fog only to the no-sky height-fog branch.
- [ ] Synchronize the OptiFine profiler setting with the existing profiler implementation.
- [ ] Re-run source tests.

### Task 4: Better Snow and Smooth Biomes

**Files:**
- Modify: `src/net/minecraft/src/RenderBlocks.h`
- Modify: `src/net/minecraft/src/RenderBlocks.cpp`
- Modify: `src/net/minecraft/src/BlockGrass.cpp`
- Modify: `src/net/minecraft/src/BlockLeaves.cpp`
- Modify: `src/net/minecraft/src/BlockTallGrass.cpp`
- Modify: `src/net/minecraft/src/BlockVine.cpp`
- Test: `tests/optifine_phase1_source_test.py`

- [ ] Add failing assertions for Better Snow helper and Smooth Biomes gates.
- [ ] Add `hasSnowNeighbours` and use the existing snow-layer standard render path for the C6-compatible non-full-block render cases.
- [ ] Make existing biome averaging conditional on Smooth Biomes, preserving vanilla single-biome lookup when disabled.
- [ ] Re-run source tests.

### Task 5: Static verification and package

**Files:**
- Test: `tests/optifine_phase1_source_test.py`
- Output: `/mnt/data/mcbeta_optifine_c6_phase1.zip`

- [ ] Run all source-regression tests.
- [ ] Search for excluded systems accidentally added by Phase 1.
- [ ] Inspect modified files for malformed declarations, duplicate enum ordinals, and missing save/load keys.
- [ ] Package the isolated working tree without build artifacts.
