# OptiFine C6 Phase 4 Design

## Scope

Finish the remaining OptiFine 1.2.5 HD C6 behavior that does not require the excluded TextureHD hierarchy, update checker, Forge reflection, Show Capes, LWJGL DisplayMode-specific fullscreen handling, or PC-only fullscreen modes.

Phase 4 includes:

- Close the two Phase 3 gaps: connected glass panes and Natural Textures transform isolation.
- Mipmap level/type options and actual backend-aware mipmap sampling.
- Anisotropic filtering option with backend capability clamping.
- Antialiasing option on desktop through SDL multisample context creation; consoles expose no unsupported AA path.
- Texture dimension tracking and high-resolution atlas-aware dynamic texture placement without TextureHD classes.
- C6 custom animation properties (`anim/*.properties`) and legacy `custom_*.png` replacement frames using ordinary RenderEngine uploads.
- Custom font width properties and texture-pack refresh handling.
- Fine render-distance setting only where it can be represented without bypassing console resident-radius hard limits.

## Architecture

RenderEngine remains responsible for decoded image dimensions, mip chains, texture-pack-derived custom animations, and dynamic texture uploads. RenderAPI exposes capability and sampling controls; GL implements mipmap/AF support, GX implements mipmaps and GX anisotropy where supported, and GS/PS2 cleanly reports unsupported sampling features rather than accepting settings it cannot execute.

GameSettings stores C6-compatible values while Config is the read-only policy layer. The GUI clamps values to backend capabilities so PS2/Wii never enable a setting that silently consumes memory or changes renderer residency beyond platform limits.

Custom animations are implemented as focused `CustomAnimation`/`CustomAnimationFrame` classes and use `RenderEngine::updateTextureSubImageRegion`; no TextureHD class is introduced. FontRenderer reads optional `<font texture>.properties` widths from the selected pack when Custom Fonts is enabled, falling back to the default pack otherwise.

## Safety constraints

- Preserve all existing EnumOptions ordinals; append new options only.
- Never upload mip levels on PS2 because Ps2Texture supports level 0 only.
- Keep console chunk/radius limits authoritative over fine render distance.
- Do not compile automatically per CODESTYLE.MD.
- Do not add VersionCheckThread, Reflector, TextureHD*, Show Capes, or DisplayMode/fullscreen mode code.
