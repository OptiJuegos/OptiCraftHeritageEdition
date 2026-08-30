# SPEC: Salir de opengx en el renderer de Wii

Fecha: 2026-08-12
Estado: análisis / no iniciado

## Pregunta

¿Qué tan complicado sería que el renderer del port de Wii deje de depender de opengx
(la capa que traduce llamadas OpenGL 1.x a GX)?

## Situación actual

### Dónde vive opengx
- Vendorizado completo en `src/wii/opengx/` (upstream `github.com/devkitPro/opengx`).
- No es submódulo, es código de terceros commiteado tal cual, compilado con `-w`
  (warnings suprimidos) como lib estática propia. Wiring en `cmake/wii.cmake:118-139`.
- Tamaño: ~33,970 líneas totales; ~15,481 son headers de spec GL (`gl.h`/`glext.h`),
  quedan ~18,489 líneas de lógica de traducción real
  (`gc_gl.c` 3381, `call_lists.c` 1638, `shader.c` 1100, etc.).

### Estructura del renderer
- No existe un "gles_wii.cpp" propio: la abstracción que usa el código del juego
  **es la API OpenGL 1.1 estándar**, igual en todas las plataformas.
- Cada plataforma pone su propio backend GL debajo:
  - Wii: `src/wii/opengx/` (traductor GL→GX completo, de terceros)
  - PS2: `src/ps2/gles_ps2.cpp` + `.h` (2448 + 481 líneas, backend GL→GS escrito a mano)
  - PC: GL real vía SDL
- `src/wii/gx_wii.cpp` / `.h` (267 + 82 líneas) **no** es una implementación de GL —
  es solo bring-up de video/GX (VIDEO_Init, GX_Init, FIFO, flip EFB↔XFB), equivalente
  a lo que hacen SDL/GLUT en desktop.
- Comentario en `gx_wii.cpp`: hubo un backend GX nativo hecho a mano de ~1500 líneas
  que llegó a arrancar hasta el menú principal, y se abandonó antes de adoptar opengx.

### Call sites en el código del juego
- `src/net/` (lógica de juego multiplataforma) tiene ~1203 llamadas `gl*()` en
  76 archivos, usando ~77 entry points únicos (comentario en cmake: "All 70 GL
  entry points this game uses").
- Casi todo pasa por API limpia: `glVertexPointer` / `glDrawArrays`, no immediate mode.
  La única excepción es `CanvasMojangLogo.cpp` (logo de boot), que usa
  `glBegin`/`glVertex2i`/`glEnd`.
- Display lists (`glNewList`/`glCallList`/`glGenLists`) aparecen en 6 archivos:
  `FontRenderer`, `GLAllocation`, `ModelRenderer`, `RenderGlobal`, `RenderList`,
  `WorldRenderer.cpp` — la compilación de chunks es el caso grande.

### Qué necesita el juego de opengx
Immediate mode, matrix stack, texture env, lighting, fog, y sobre todo
**display lists que graban geometría Y cambios de estado a la vez** — esta fue
la razón explícita por la que se abandonó el backend hecho a mano anterior
(`cmake/wii.cmake:108-113`).

### Bugs ya encontrados en opengx (parches locales existentes)
- Falta el case `GL_FOG` en `glDisable` (`gc_gl.c:811-832`).
- El campo u16 de conteo de vértices de `GX_Begin` desincroniza el FIFO al
  desbordarse por encima de 65535 (`utils.h:360-375`, arreglado con batching en
  `call_lists.c`/`gc_gl.c`).
- El bug de "normales" (`Tessellator.cpp:483-489`) resultó ser un bug de
  endianness del lado del juego, NO de opengx.
- `ModelRenderer.h:27-29` dice explícitamente: "opengx's recorded path is the
  one this port keeps finding bugs in."

### Precedente de código GX nativo
- Ninguno en el código del juego: grep de `GX_`/`gccore.h` fuera de
  `gx_wii.cpp`/opengx no da resultados (salvo comentarios).
- El único precedente de backend hecho a mano es `gles_ps2.cpp` en PS2, pero
  PS2 evita el problema difícil: renderiza el mundo en modo inmediato
  (`PS2_IMMEDIATE_WORLD_RENDER=1`), sin display lists reales.

## Evaluación

**No tan complicado del lado del código del juego**: ya está desacoplado detrás
de la API GL estándar. Reemplazar opengx no implicaría tocar Tessellator,
WorldRenderer, entity renderers, GUI, etc. — solo el backend.

**Sí complicado por un punto concreto**: el bloqueo real es escribir un backend
GX nativo que soporte display lists que graban geometría + estado (usados para
compilar chunks, fuentes y modelos). Esto **ya se intentó antes** y fue
abandonado precisamente por esta razón — no es territorio nuevo, es retomar un
problema ya conocido.

**Tamaño esperado del backend nuevo**: análogo a `gles_ps2.cpp`, del orden de
2000-4000 líneas.

## Conclusión

Viable pero no trivial. El riesgo principal y determinante del esfuerzo total
es la implementación de display lists nativos en GX (chunks + fuentes +
modelos), no la reescritura de call sites en el juego.
