# OptiFine 1.2.5 HD C6 Phase 2 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Port OptiFine C6 performance scheduling and renderer optimizations without breaking PS2/Wii hard budgets or reintroducing backend-specific code into Minecraft rendering logic.

**Architecture:** `RenderGlobal` implements C6 scheduling and renderer-grid policy, `WorldRenderer` owns deferred section/occlusion state, `RenderAPI` abstracts Smooth FPS synchronization, and `Tessellator` grows batches instead of flushing during chunk compilation.

**Tech Stack:** Existing C++ Minecraft 1.2.5 port, RenderAPI, RenderTerrainAPI, platform tuning macros, Python source-regression tests.

**Spec:** `docs/superpowers/specs/2026-08-25-optifine-c6-phase2-design.md`

## Global Constraints

- Do not compile automatically.
- Do not use git or create commits.
- Keep PS2/Wii renderer-grid and mesh-budget hard limits authoritative.
- Preserve Phase 1 behavior and all explicitly excluded OptiFine systems.

---

### Task 1: C6 chunk scheduler and Dynamic Updates

**Files:**
- Modify: `src/net/minecraft/src/Config.h`
- Modify: `src/net/minecraft/src/Config.cpp`
- Modify: `src/net/minecraft/src/RenderGlobal.h`
- Modify: `src/net/minecraft/src/RenderGlobal.cpp`
- Test: `tests/optifine_phase2_source_test.py`

- [ ] Add failing assertions for Smooth FPS/Input queries, movement/action helpers, 2-second stationary grace, x3 Dynamic Updates, frustum penalty, and queue compaction.
- [ ] Implement platform-neutral movement/action detection and C6 desktop scheduling.
- [ ] Apply effective update count inside console hard mesh ceilings.
- [ ] Re-run Phase 1 and Phase 2 source tests.

### Task 2: Load Far and Preloaded Chunks

**Files:**
- Modify: `src/net/minecraft/src/RenderGlobal.h`
- Modify: `src/net/minecraft/src/RenderGlobal.cpp`
- Test: `tests/optifine_phase2_source_test.py`

- [ ] Add failing assertions for desktop-only grid expansion and preloaded reposition threshold.
- [ ] Add `prevRepos*` state and C6 renderer-grid sizing on desktop.
- [ ] Keep fixed console grids unchanged.
- [ ] Re-run source tests.

### Task 3: Smooth FPS RenderAPI hook

**Files:**
- Modify: `src/platform/RenderAPI.h`
- Modify: `src/platform/RenderAPI_GL.cpp`
- Modify: `src/platform/RenderAPI_GX_WII.cpp`
- Modify: `src/platform/RenderAPI_GS_PS2.cpp`
- Modify: `src/net/minecraft/src/RenderGlobal.cpp`
- Test: `tests/optifine_phase2_source_test.py`

- [ ] Add failing assertions for one backend-neutral finish hook.
- [ ] Implement desktop `glFinish` and no-op native-console implementations.
- [ ] Invoke it only for Smooth FPS on opaque terrain pass.
- [ ] Re-run source tests.

### Task 4: Fancy Occlusion and frustum reuse

**Files:**
- Modify: `src/net/minecraft/src/WorldRenderer.h`
- Modify: `src/net/minecraft/src/WorldRenderer.cpp`
- Modify: `src/net/minecraft/src/RenderGlobal.h`
- Modify: `src/net/minecraft/src/RenderGlobal.cpp`
- Test: `tests/optifine_phase2_source_test.py`

- [ ] Add failing assertions for full-frustum state and visibility-position cache.
- [ ] Reuse the existing three-way frustum classifier for Fancy Occlusion.
- [ ] Save positive occlusion-query positions and skip redundant queries near them.
- [ ] Use C6-style bounded occlusion batches rather than exponential batches.
- [ ] Re-run source tests.

### Task 5: Deferred occlusion box and Tessellator growth

**Files:**
- Modify: `src/net/minecraft/src/WorldRenderer.h`
- Modify: `src/net/minecraft/src/WorldRenderer.cpp`
- Modify: `src/net/minecraft/src/Tessellator.h`
- Modify: `src/net/minecraft/src/Tessellator.cpp`
- Test: `tests/optifine_phase2_source_test.py`

- [ ] Add failing assertions for deferred box rebuild and grow-before-write behavior.
- [ ] Move desktop occlusion-box list rebuild from `setPosition` into the next renderer update.
- [ ] Add Tessellator capacity growth and remove buffer-full implicit `draw()`.
- [ ] Re-run all source tests and package the tree.
