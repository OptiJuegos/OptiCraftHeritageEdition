# OptiFine 1.2.5 HD C6 Phase 2 Design

## Scope

Port the performance-oriented OptiFine C6 behavior that maps cleanly to the current C++ renderer: chunk update scheduling, Dynamic Updates, Load Far/Preloaded Chunks behavior, Smooth FPS, fancy occlusion/frustum behavior, deferred occlusion-box rebuilds, and growable Tessellator batches.

## Architecture

- `RenderGlobal` owns renderer scheduling and distance/reposition policy.
- `WorldRenderer` owns section state and deferred occlusion-box maintenance, but not scheduling policy.
- `RenderAPI` owns the backend-specific GPU synchronization primitive used by Smooth FPS.
- `Frustrum`/`ClippingHelper` remain the only frustum math layer; the existing three-way classifier is reused for Fancy Occlusion.
- `Tessellator` grows a batch instead of flushing it in the middle of a chunk compilation.

## Platform constraints

- PS2/Wii keep their fixed renderer grids; Load Far and Preloaded Chunks must not increase resident renderer counts there.
- Console mesh budgets remain hard ceilings. OptiFine Chunk Updates/Dynamic Updates can reduce or raise work only inside those ceilings.
- Smooth FPS performs a real GPU finish on desktop OpenGL only. Native Wii/PS2 backends expose a no-op because an unconditional GS/GX finish would add stalls without matching the desktop driver-queue problem.
- Existing PS2 per-frame frustum classification remains intact.
- No update checker, TextureHD, Forge reflection, LWJGL display-mode management, PC fullscreen mode selection, or Show Capes.

## Behavior

- Dynamic Updates multiplies the requested chunk-update count by three after the player has been stationary for two seconds, matching C6.
- Active block/item interaction makes nearby dirty renderers eligible immediately on the desktop scheduler.
- Out-of-frustum dirty renderers receive a 4x distance penalty when selecting the next renderer to rebuild.
- Sparse update queues are compacted instead of repeatedly retaining null entries.
- Load Far keeps the larger renderer grid across render-distance changes on desktop; Preloaded Chunks enlarges that grid and delays renderer-grid repositioning.
- Fancy Occlusion only queries renderer boxes fully inside the frustum and reuses recent positive visibility results while the player remains near the last visible position.
- Smooth FPS invokes a RenderAPI GPU-finish hook before opaque terrain submission on desktop.
- Tessellator grows before a write that would exceed the current buffer; it no longer calls `draw()` merely because the CPU batch filled.

## Verification

Project instructions prohibit automatic compilation. Verification uses Python source-regression tests plus targeted source inspection. Phase 1 tests must remain green.
