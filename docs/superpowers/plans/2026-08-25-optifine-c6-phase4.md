# OptiFine C6 Phase 4 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Finish the non-excluded HD/sampling/custom-animation feature set from OptiFine 1.2.5 HD C6 on PC, Wii and PS2 without introducing TextureHD classes.

**Architecture:** Texture quality is expressed through RenderAPI capabilities and applied by RenderEngine. Custom animations and font properties stay above the backend. Console hard limits remain authoritative.

**Tech Stack:** C++17, SDL2/OpenGL desktop backend, libogc GX Wii backend, PS2SDK/GS backend, Python source-verification tests.

**Spec:** `docs/superpowers/specs/2026-08-25-optifine-c6-phase4-design.md`

## Global Constraints

- No TextureHD hierarchy.
- No update checker or VersionCheckThread.
- No Forge reflection/Reflector.
- No Show Capes.
- No LWJGL DisplayMode or PC-only fullscreen mode work.
- Preserve existing EnumOptions ordinals.
- Do not compile automatically.

---

### Task 1: Close Phase 3 regressions

**Files:** `tests/optifine_phase3_source_test.py`, `src/net/minecraft/src/ConnectedTextures.*`, `src/net/minecraft/src/RenderBlocks.cpp`, `src/net/minecraft/src/RenderBlocks125.cpp`

- [ ] Add failing regressions for connected glass panes and face-local Natural transforms.
- [ ] Implement C6 glass-pane CTM atlas selection through sub-tessellators.
- [ ] Reset Natural rotation/flip state before and after each face resolution.
- [ ] Run Phase 3 tests and verify all pass.

### Task 2: Settings and backend capabilities

**Files:** `GameSettings.*`, `EnumOptions.*`, `Config.*`, `GuiQualitySettingsOF.cpp`, `RenderAPI.*`, `WiiNativeTexture.*`, `src/pc/lwjgl/GLContext.*`, `src/pc/Main.cpp`

- [ ] Add failing tests for Mipmap Level/Type, AF, AA, Custom Fonts and fine render distance persistence/UI.
- [ ] Add RenderFeature capability queries and texture-quality application API.
- [ ] Implement GL mipmap/AF capability limits, GX mipmap/anisotropy limits, PS2 unsupported behavior.
- [ ] Implement desktop SDL multisample request from the persisted AA setting.
- [ ] Run Phase 4 settings/capability tests.

### Task 3: RenderEngine texture dimensions and mip chains

**Files:** `RenderEngine.*`, `Config.*`

- [ ] Add failing tests for dimension tracking, configurable mip count and atlas tile width.
- [ ] Track uploaded texture dimensions and terrain/items icon widths.
- [ ] Generate only the selected/capability-clamped mip count and apply requested sampling.
- [ ] Scale dynamic 16x16 TextureFX output to the active atlas tile width without TextureHD.
- [ ] Run Phase 4 texture tests.

### Task 4: Custom animations without TextureHD

**Files:** create `CustomAnimationFrame.*`, `CustomAnimation.*`; modify `RenderEngine.*`, `OptiFineResource.*`

- [ ] Add failing tests for frame timing, animation property discovery and regional upload.
- [ ] Port C6 frame sequencing/property parsing.
- [ ] Discover `anim/*.properties` resources through the selected texture pack abstraction.
- [ ] Upload active frames into destination textures and support legacy `custom_terrain_*`, `custom_item_*`, water/lava/fire/portal replacement images.
- [ ] Clear/reload animation caches on texture-pack refresh.
- [ ] Run Phase 4 animation tests.

### Task 5: Custom fonts and render distance fine

**Files:** `FontRenderer.*`, `GameSettings.*`, `Config.*`, `GuiVideoSettings.cpp`

- [ ] Add failing tests for `.properties` custom widths and pack fallback.
- [ ] Load `width.<char>` values when Custom Fonts is enabled and refresh when texture pack changes.
- [ ] Add fine render-distance value while preserving console platform caps.
- [ ] Run Phase 4 tests.

### Task 6: Final verification and package

- [ ] Run Phase 1, 2, 3 and 4 source suites.
- [ ] Scan for excluded classes/systems.
- [ ] Package the verified tree as `mcbeta_optifine_c6_phase4.zip`.
