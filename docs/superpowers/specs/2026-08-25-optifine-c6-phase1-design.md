# OptiFine 1.2.5 HD C6 Phase 1 Design

## Scope

Complete the existing OptiFine settings layer and the low-risk C6 runtime toggles without porting the update checker, TextureHD/mipmap pipeline, Forge reflection, LWJGL-specific display mode management, PC-specific fullscreen modes, or Show Capes.

## Compatibility constraints

- Keep the existing platform render backends and console tuning unchanged.
- Preserve the existing `ofFogFancy`/`ofFogOff` representation and saved keys for backward compatibility.
- Keep unsupported HD/AA/AF features out of this phase.
- Do not force fine render distances on PS2/Wii; the console resident-chunk limits remain authoritative.
- New options must round-trip through `options.txt` while unknown lines remain preserved.
- Runtime hooks must stay platform-neutral and use existing `Config`, `RenderAPI`, and Minecraft render paths.

## Phase 1 features

- C6 settings: Sun & Moon, Depth Fog, Debug Profiler, Better Snow, Swamp Colors, Smooth Biomes.
- C6 animation/particle toggles: Void Particles, Water Particles, Rain Splash, Portal Particles, Dripping Water/Lava, Terrain Animated, Items Animated, Textures Animated.
- Complete `Config` query methods for the above features.
- Add C6-style Performance and Quality screens and wire them from Video Settings.
- Expand Animations, Details, and Other screens to the relevant C6 option sets, excluding explicitly out-of-scope items.
- Add All ON / All OFF animation controls.
- Hook particle filtering, celestial rendering, depth fog, and profiler toggle into existing runtime code.
- Implement Better Snow using the existing RenderBlocks path without introducing backend-specific rendering.
- Use existing biome color averaging where possible; Smooth Biomes controls whether averaging is used.

## Verification

The project instructions prohibit automatic compilation. Verification therefore uses source-level regression tests, structural checks, and targeted source inspection. No build is run automatically.
