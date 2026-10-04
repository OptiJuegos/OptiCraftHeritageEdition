# OptiCraft Heritage

[![Build](https://github.com/OptiJuegos/OptiCraftHeritageEdition/actions/workflows/build.yml/badge.svg)](https://github.com/OptiJuegos/OptiCraftHeritageEdition/actions/workflows/build.yml)

OptiCraft Heritage is a heavily modified, clean-room C++ implementation of classic Minecraft-era gameplay designed around portability, low-end hardware, and console-specific optimization.

This repository is not intended to be a line-for-line source translation. The runtime, platform layers, rendering paths, input backends, storage systems, user interface, asset loading, memory policies, and console support have been extensively reworked for the needs of this project.

## Project goals

- Keep the implementation portable across desktop PC, PlayStation 2, Nintendo Wii, and Nintendo 3DS.
- Preserve the intended classic gameplay and visual behavior where practical while allowing platform-specific adaptations.
- Run on constrained hardware through aggressive memory, rendering, chunk, and asset-loading optimizations.
- Keep platform code isolated behind explicit backends instead of scattering host-specific logic through the game code.
- Maintain a debuggable and production-oriented C++17 codebase.

## Clean-room implementation

OptiCraft Heritage is developed as a clean-room implementation. The project code is independently implemented in C/C++ and is heavily modified around its own runtime and platform architecture.

The project does not rely on original proprietary game source code as part of its implementation. Compatibility-oriented behavior may be reproduced from observable behavior, documented formats, protocol behavior, and independently developed interfaces.

This project is not affiliated with, endorsed by, or sponsored by Mojang Studios or Microsoft.

## Supported targets

### PC (Windows, Linux, macOS)

The desktop build uses SDL2, OpenGL, and the shared platform abstraction layer. It is continuously built on Windows (x64, x86, and ARM64, across GCC, Clang, and MSVC toolchains, including a legacy low-end x86 profile and CPU-tuned variants), Linux (x64, x86, ARM64, and ARMv7 with GCC, plus x86_64-v3/Zen-tuned profiles), and macOS (ARM64 and x64 with Clang).

A dedicated 32-bit legacy profile is available for older SSE2-class CPUs and legacy OpenGL hardware.

### PlayStation 2

The PS2 build uses a native platform backend with PS2SDK support, GS-specific rendering, console-aware memory policies, asynchronous asset loading, platform storage, controller input, and optional VU-assisted terrain paths.

The expected USB application directory is:

```text
mass:/OptiCraftHeritage/
```

### Nintendo Wii

The Wii build uses devkitPPC/libogc and a native GX rendering path. The Homebrew Channel layout remains:

```text
apps/OptiCraft/
```

### Nintendo 3DS

The 3DS build uses devkitARM with libctru and citro3d, with console-specific rendering, input, storage, and audio backends. Every build produces the `OptiCraft.3dsx` homebrew executable (with its `.smdh` cover) and an installable `OptiCraft.cia`, packaged straight from the `.elf` through the checked-in manifest `resources/3ds_cia.rsf`.

Runtime assets are read from the SD card instead of RomFS, so data can change without rebuilding or reinstalling the CIA. The expected runtime directory is:

```text
sd:/opticraft/
```

The `.cia` installs with FBI (or any CIA installer) and launches from the HOME menu on retail consoles, including Old 3DS/2DS models; the `.3dsx` runs from the Homebrew Launcher or in emulators such as Azahar.

#### 3DS controls

| Input | Menus / text entry | In game | Containers |
|---|---|---|---|
| Circle Pad | cursor and list navigation | movement | free cursor |
| C-Stick (New 3DS) | — | camera (deflect to look; same sensitivity setting as touch) | — |
| A | confirm; types the highlighted key on the on-screen keyboard | jump | pick up / place stack |
| B | back / cancel | place / use | close the container |
| X | space | attack / destroy | split stack / place one |
| Y | close the keyboard | inventory | close the keyboard |
| L | space | place / use | space |
| R | shift | attack / destroy | quick-move (shift-click) |
| ZL / ZR (New 3DS) | — | previous / next hotbar slot | — |
| D-Pad | menu navigation; keyboard cursor | left/right: hotbar slot, up: chat (multiplayer), down: perspective (F5) | move the selected slot |
| START | back; Enter while a text field is focused | pause | close the container |
| SELECT | space | sneak | space |
| Touch | pointer (the on-screen keyboard reads taps directly) | camera (drag to look) | pointer |

The shoulder buttons are deliberately swapped from the PC mouse convention: L places and R attacks. Text fields open the console's software keyboard first; if it is unavailable, the game draws an on-screen keyboard on the bottom screen. Confirming a keyboard session leaves the text in the field — for chat, pressing A then sends the message. In gameplay the touch panel moves the camera only; the triggers own both clicks, and the inventory is navigable end to end from the pad (D-pad to move the slot selection, A/B/R as above) the same way the other consoles do it.

**New 3DS extras** need no setup and no toggles: the C-Stick is a second camera stick in gameplay (its look rate follows how far it is pushed, and it shares the sensitivity setting with the touch camera), and ZL/ZR step the hotbar back/forward alongside the D-Pad's left/right, keeping the D-Pad free for the camera. Both are New 3DS/2DS hardware only — on Old models the inputs don't exist, the channels read as centred, and nothing changes.

Menus work from the pad alone on every screen, including the multiplayer server list: D-Pad or Circle Pad steps the selection through the server entries and the button rows (the list scrolls to keep the selection visible), A joins the highlighted server or presses the focused button, and B/START/Y go back. The touch panel keeps working alongside — while the finger is on the screen it owns the selection.

With the Legacy UI on, the inventory button opens the legacy crafting screen on every console, and its item strip is the inventory: D-pad **down** moves the pad cursor from the recipes into the 4x9 strip (up from the strip's top row goes back), and **A** on a strip slot grabs the stack — A on another slot swaps them, A on the same slot puts it back. Touch taps on the strip do the same. L/R stay on the category tabs and A on the recipes still crafts.

**Face-Button Camera** (toggle in OptiCraft Options) is an alternative control mode for gameplay: the A/B/X/Y diamond becomes a look pad (Y = left, A = right, X = up, B = down), attack and place live on the shoulders alone, and jump/sneak/inventory move to SELECT — tap SELECT or double-tap B to jump, hold SELECT to sneak, double-tap Y for the inventory. All three look inputs — face buttons, C-Stick and touch — share the sensitivity setting.

## Source layout

```text
src/
  3ds/          Nintendo 3DS implementation
  client/       Client-side shared code
  external/     Vendored sources compiled into the game (stb_image)
  java/         Java compatibility/runtime helpers
  locale/       Descriptive text helpers
  lwjgl/        LWJGL-style forwarding shims
  mods/         In-game mod loading framework and bundled mods
  net/          Game implementation
  pc/           Desktop-specific implementation
  platform/     Shared platform interfaces and backend selection
  ps2/          PlayStation 2 implementation
  util/         Shared utility code
  wii/          Nintendo Wii implementation

cmake/          Toolchains, source selection, and platform build logic
external/       Third-party dependencies
```

Platform targets deliberately select one implementation for each public backend. This keeps PC, PS2, Wii, and 3DS implementations from accidentally entering the same link target.

## Building

CMake 3.21 or newer is required. Presets are defined in `CMakePresets.json`.

Most dependencies are vendored under `external/`. The one exception is SDL_net, which is gitignored and must be cloned separately before a desktop configure:

```sh
git clone --depth 1 https://github.com/libsdl-org/SDL_net.git external/SDL_net
git -C external/SDL_net fetch --depth 1 origin b1085eed744eab0c894a5306d822c9241bcf0228
git -C external/SDL_net checkout FETCH_HEAD
```

Game code uses bare includes such as `#include "Minecraft.h"`, resolved against `src/net/minecraft/src`. Console toolchain files add that include path themselves; on desktop, add `-DCMAKE_CXX_FLAGS=-I<prefix>/src/net/minecraft/src` to the configure if you hit missing-header errors.

### Desktop (Windows)

```text
cmake --preset gcc-debug
cmake --build --preset gcc-debug
```

For a normal optimized build:

```text
cmake --preset gcc-release
cmake --build --preset gcc-release
```

### Desktop (Linux, macOS)

The `gcc-*` presets are MinGW/Windows-specific. On Linux and macOS use a plain CMake configure, which is also what CI does:

```sh
cmake -B build/release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DMC_LOG_LEVEL=2 \
  "-DCMAKE_CXX_FLAGS=-I$PWD/src/net/minecraft/src"
cmake --build build/release
```

On macOS, add `-DCMAKE_OSX_ARCHITECTURES=<arm64|x86_64>` to select the target architecture.

### 32-bit / legacy PC

The 32-bit desktop builds are driven by the `gcc32-*` presets (MinGW-w64 with `-m32 -msse2 -mfpmath=sse`):

```text
cmake --preset gcc32-release
cmake --build --preset gcc32-release
```

Use `gcc32-debug` for a debug build, and `gcc32-legacy-release` for the low-end profile targeting SSE2-class CPUs and legacy OpenGL hardware (reduced logging, `bin/Release` output layout). The wrappers `build gcc - release32.bat` and `build gcc - release32 - ultralowend.bat` drive the `gcc32-release` and `gcc32-legacy-release` presets respectively, picking a native Windows CMake automatically.

### PlayStation 2

```text
cmake --preset ps2-release
cmake --build --preset ps2-release
```

Use `ps2-debug` for a debug build. Asset staging remains a separate step so large runtime data is not recopied after every link:

```text
cmake --build --preset ps2-release --target ps2-data
```

The maintained wrapper `build ps2.bat` drives the same flow (`build ps2.bat data` stages the assets). The ps2dev toolchain is expected at `./psdevwindows/`; the preset exports `PS2DEV`/`PS2SDK` itself.

### Nintendo Wii

```text
cmake --preset wii-release
cmake --build --preset wii-release
```

Use `wii-debug` for a debug build and `wii-bringup` for the minimal hardware/toolchain bring-up target. The maintained wrapper `build wii.bat game` drives the release build; devkitPPC is found at `C:\devkitPro` or via `DEVKITPRO`.

### Nintendo 3DS

```text
cmake --preset 3ds-release
cmake --build --preset 3ds-release
```

Use `3ds-debug` for a debug build and `3ds-bringup` for the minimal toolchain bring-up smoke test. The maintained wrapper `build 3ds.bat` drives the release build and packages the installable `OptiCraft.cia` with `makerom`. devkitARM is found at `C:\devkitPro` or via `DEVKITPRO`.

## Development notes

OptiCraft Heritage contains substantial platform-specific changes compared with the behavior it reproduces. Examples include custom render backends, legacy UI work, low-memory chunk policies, console input layers, asset streaming, platform storage, audio backends, profiling, and console-specific performance tuning.

When changing shared systems, keep the platform abstraction boundary intact and avoid introducing PC-only assumptions into common code. Likewise, console-specific optimizations should remain behind platform policies or dedicated backends whenever possible.

## Third-party software

Third-party libraries are kept under `external/` and retain their respective licenses and notices. Review those licenses independently before redistributing binaries.
