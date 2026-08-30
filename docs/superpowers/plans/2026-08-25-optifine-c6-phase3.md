# OptiFine C6 Phase 3 Implementation Plan

**Goal:** Port the non-TextureHD visual feature set from OptiFine 1.2.5 HD C6: custom colors, connected textures, natural textures, random mobs, and complete Better Snow integration.

**Architecture:** Keep visual selection logic above RenderAPI. Texture-pack parsing lives in focused OptiFine classes. RenderBlocks owns face-level CTM/Natural decisions; RenderEngine refreshes texture-pack-derived state; entity renderers ask RandomMobs for a deterministic variant. Console backends receive ordinary texture IDs and geometry and remain free of OptiFine-specific logic.

**Global Constraints:** No TextureHD, update checker, Forge reflection, Show Capes, LWJGL DisplayMode, or PC-only fullscreen work. Preserve existing option ordinals. Do not compile automatically. Keep PS2/Wii hard limits and backend paths intact.

### Task 1: Settings and Config
Add Random Mobs, Custom Colors, Connected Textures, and Natural Textures to GameSettings, EnumOptions, Config, persistence, and Quality UI.

### Task 2: Texture-pack resource utilities
Add small parsing helpers for text properties and deterministic Java-compatible position hashing needed by Natural/CTM.

### Task 3: Natural Textures
Port NaturalProperties/NaturalTextures and hook per-face rotation/flip through RenderBlocks without backend-specific code.

### Task 4: Connected Textures
Port ConnectedProperties/ConnectedTextures matching logic. Route custom CTM atlas selections through a platform-neutral face texture selection path; preserve the terrain atlas path when no CTM rule matches.

### Task 5: Custom Colors
Port CustomColorizer colormap loading and the main C6 consumers: block/grass/foliage/fluid/redstone/stem/lilypad plus particle and sky/fog hooks where the current class APIs permit it.

### Task 6: Random Mobs
Port texture-pack variant discovery and deterministic entity texture selection, reset on texture-pack refresh, and use it from RenderLiving.

### Task 7: Verification
Run Phase 1, 2 and 3 static suites, scan for excluded systems, and package the modified tree.
