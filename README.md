# BetaPlusPlus

A faithful **1:1 C++17 port of Minecraft Beta 1.7.3**, rebuilt from the Java
decompilation and extended with features from **OptiFine 1.7.3 HD MT G2**.

The goal is a clean, portable native build that preserves vanilla Beta 1.7.3
behaviour while opening the door to modern improvements: cross-platform support,
performance work, controller input and low-spec targets.

> **Heads-up:** large parts of this port were produced **automatically** with AI
> tooling — primarily **Kimi (K2)** and **Claude Opus 4.8** — translating the
> Java decomp into C++ class by class. Some implementations are still rough,
> stubbed, or being corrected. See [TODO](#todo) before relying on a subsystem.

---

## Tech stack

- **Language:** C++17
- **Windowing / input / audio:** SDL2 (replaces LWJGL)
- **Networking:** SDL_net
- **OpenGL loader:** glad
- **Compression:** zlib
- **Image / misc:** stb, miniaudio
- **Build system:** CMake (≥ 3.21)

Platform layer lives under `src/pc/` (`win32/` and `linux/` are selected
automatically by CMake; only one is ever compiled in).

---

## Building

### 1. Clone with submodules

The third-party dependencies live in `external/` as git submodules:

```sh
git clone --recursive <repo-url>
# already cloned without --recursive:
git submodule update --init --recursive
```

### 2. Configure & build

Requirements: CMake ≥ 3.21 and a C++17 compiler — **MSVC 2022** or
**MinGW-w64 / MSYS2** on Windows, GCC/Clang on Linux.

**Using CMake presets (recommended):**

```sh
cmake --preset gcc-release        # configure
cmake --build --preset gcc-release --parallel
```

Available presets: `gcc-debug`, `gcc-release`, `gcc32-debug`, `gcc32-release`
(the 32-bit ones target a multilib MinGW / MSYS2 i686 toolchain — useful for the
low-spec goals below).

**Plain CMake (MSVC):**

```sh
cmake -S . -B build
cmake --build build --config Release --target BetaPlusPlus --parallel
```

**Convenience scripts (Windows):** `build gcc - release.bat`,
`build gcc - release32.bat`, `build msvc - release.bat`, `build gcc - debug.bat`.

### 3. Run

The executable is written to:

```
bin/<Config>/BetaPlusPlus(.exe)
```

It looks for game assets (`terrain.png`, `resources/`, `assets/`) relative to the
working directory — run it from a folder that contains them.

---

## TODO

Roadmap, roughly in priority order. Contributions welcome.

- [ ] **Cross-platform support** — finish and harden the Linux path; broaden
      beyond Windows/Linux (macOS, BSD). The CMake platform split exists but the
      non-Windows backend needs real testing.
- [ ] **Fix bad / stubbed implementations** — several classes were AI-translated
      and are incomplete, approximate, or marked with `TODO`/`field_*`/`func_*`
      placeholders. Audit against the Java decomp and correct behaviour.
- [ ] **Performance boosts** — render-path optimization (chunk rebuilds, draw
      submission, fewer vertices / overdraw), memory-footprint reduction, and
      general hot-loop cleanup.
- [ ] **Gamepad support** — controller input via SDL2's gamepad API, with a
      usable on-screen / remappable control scheme.
- [ ] **PS2 support** — bring-up on the PlayStation 2: software/fixed-function
      rendering path, tight RAM budget (~32 MB), and a controller-first UI.
- [ ] **Misc** — better asset/resource-pack handling, configurable low-spec
      presets, and polishing the OptiFine-derived options.

---

## Credits & provenance

- **Minecraft Beta 1.7.3** — © Mojang. This is a fan/educational port; no assets
  are distributed here.
- **OptiFine 1.7.3 HD MT G2** — source used as reference for ported optimisations
  and options.
- **Automated porting** — Kimi (K2) and Claude Opus 4.8 performed the bulk of the
  Java → C++ translation; human review and fixes are ongoing.

// WII

Confirmado el solapamiento exacto: la mano y el paperdoll comparten las tres cosas — glClear(GL_DEPTH_BUFFER_BIT) a mitad de frame, enableStandardItemLighting(), y listas de ModelRenderer. Y GL_NORMALIZE/GL_RESCALE_NORMAL no están implementados en opengx. Verifico la matriz de normales.

Compilá y probá. Resumen de dónde estamos:

Lo resuelto
El OOM era el heap, y la causa estaba documentada al revés. Desensamblé _sbrk_r de libogc. Su rama MEM1 termina así:


bl SYS_GetArena1Hi ; cmplw ; ble ok
li r9,12 /*ENOMEM*/ ; li r31,-1 ; return -1
No hay ninguna referencia a Arena2 en ese camino. MALLOC_MEM2 se lee al entrar y elige un arena para toda la ejecución — no es una política de desbordamiento. Con su valor por defecto (0) el heap eran las sobras de MEM1: ~8 MB tras el ELF de 14,5 MB, los framebuffers y el FIFO. Tus propios logs mostraban secciones pidiendo 190 KB de display list; cuatro de esas y se acabó, incluidas las texturas.

Lo incómodo: el README del port y mi nota de memoria afirmaban lo contrario y decían haberlo verificado con este mismo objdump. Por eso "falló un malloc" se venía descartando como hipótesis débil sesión tras sesión mientras era el bug. Corregí ambos textos.

Arreglado con MALLOC_MEM2 = 1 en WiiEarlyInit.cpp → ~50 MB. Tiene que ser un símbolo de datos inicializado, no una asignación en constructor: libc y la init estática ya reservan antes.

Steve pasó de mancha negra a silueta por dos bugs de normales, ambos demostrables:

El Tessellator empaqueta normales como bytes con signo (±127) y opengx sólo mapeaba GL_SHORT/GL_FLOAT, así que GL_BYTE caía en un float(*ptr) crudo: a GX le llegaba una normal de 127 unidades de largo. Añadí NormalVertexReader<T>, calcado del ColorVertexReader<T> que ya existía por la misma razón.
GL_NORMALIZE y GL_RESCALE_NORMAL no existían en todo el árbol de opengx — cero apariciones, caían en el default: de glEnable. Misma familia que el GL_FOG de la ronda pasada. El juego los necesita: el paperdoll se dibuja bajo glScalef(-30, 30, 30).
Lo que acabo de tocar y aún no probaste
MAX_MODV_STACK era 16; la spec de GL exige mínimo 32. Y el modo de fallo es peor que perder un push: glPushMatrix se niega y marca GL_STACK_OVERFLOW (que este motor nunca consulta) mientras glPopMatrix sí saca igual. Un solo desbordamiento deja la pila desincronizada de forma permanente, y las transformaciones se filtran entre partes del modelo. Lo subí a 32 y ahora ambos extremos gritan en el log.

Es mi mejor candidato para las púas, pero no lo confirmé — te lo digo claro, no vendido como resuelto.

Lo que descarté de las púas