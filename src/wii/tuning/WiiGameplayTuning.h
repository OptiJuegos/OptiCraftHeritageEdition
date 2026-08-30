#pragma once

// -----------------------------------------------------------------------------
// Entity rendering cost (PLATFORM_SKIP_ENTITY_SHADOWS)
// -----------------------------------------------------------------------------

// Off here, unlike the PS2. Render::renderShadow() (Render.cpp) has a real
// per-entity, per-frame cost -- a 3D block scan (up to (2*shadowSize+1)^2
// columns under the entity) plus a light lookup per hit column, entirely CPU
// and independent of GPU hardware -- but the PS2's OTHER reason to skip it
// does not apply here: that console's GS framebuffer is PSMCT16 with a single
// alpha bit, which collapses shadow.png's soft radial gradient into hard
// black squares, so PS2_SKIP_ENTITY_SHADOWS was fixing a visual bug as much
// as a CPU one (see Ps2Tuning.h's own PS2_SKIP_ENTITY_SHADOWS comment).
// GX has real alpha, so the shadow already renders correctly here --
// this knob is purely a CPU lever, which is why it defaults to off (keep the
// vanilla look) instead of on.
//
// Wii enables this now: the PERF log shows worlds with roughly 260 entities,
// so paying the block scan for every visible entity is no longer a small visual
// luxury. This does not disable entities or their simulation; it only omits the
// soft blob projected onto the blocks below them.
#undef  PLATFORM_SKIP_ENTITY_SHADOWS
#define PLATFORM_SKIP_ENTITY_SHADOWS             1

// Optional lightweight terrain generation. This reuses the PS2 all-float 2D
// heightmap path without opting Wii into the rest of PLATFORM_CONSOLE_LOW. It
// changes generated terrain, so keep it disabled by default for existing-world
// compatibility and enable it explicitly when testing streaming performance.
#ifndef WII_USE_HEIGHTMAP_TERRAIN
#define WII_USE_HEIGHTMAP_TERRAIN                 0
#endif
#undef  PLATFORM_USE_HEIGHTMAP_TERRAIN
#define PLATFORM_USE_HEIGHTMAP_TERRAIN            WII_USE_HEIGHTMAP_TERRAIN

// -----------------------------------------------------------------------------
// Deliberately NOT overridden
// -----------------------------------------------------------------------------
// Recorded here because "why is this not in the PS2 list?" is the question this
// file will be read to answer:
//
//   PLATFORM_FLOAT_BIOME_NOISE, PLATFORM_FLOAT_ORE_VEINS,
//   PLATFORM_SKIP_CAVE_GENERATION
//       Broadway has a hardware FPU. These exist because the EE emulates
//       `double` in software, and every one of them CHANGES the generated world.
//       Vanilla generation stays.
//
//   PLATFORM_FORCE_FULLBRIGHT_TERRAIN
//       Paired on the PS2 with skipping the light flood-fill entirely. Keeping
//       it off is what keeps real lighting.
//
//   PLATFORM_VISIBLE_CHUNK_RADIUS, PLATFORM_VERTICAL_CHUNK_COUNT
//       Only read under PLATFORM_CONSOLE_LOW, where they replace the render grid
//       with a fixed small one. The Wii derives its grid from renderDistance so
//       the Options entry keeps working.
//
//   PLATFORM_DISABLE_RUNTIME_AUTOSAVE, PLATFORM_SKIP_NEW_WORLD_FULL_SAVE
//       Memory Card workarounds. libfat is fast enough to save normally -- but
//       not fast enough to save every two seconds, which is what the vanilla
//       period asked for; see PLATFORM_AUTOSAVE_PERIOD_TICKS above. Autosave
//       stays ON here, it just fires far less often.
//
//   PLATFORM_FORCE_GUI_SCALE
//       The framebuffer is 640x480, and ScaledResolution's auto-scale keeps
//       scale 2 at 480/2 = 240. The PS2 needs the override because 448/2 = 224
//       falls under the threshold; the Wii does not.
//
//   PLATFORM_MAX_RENDERED_SECTIONS_PER_PASS, PLATFORM_CHUNK_BUILD_BLOCKS_PER_STEP
//       The two of the four frame budgets that really are PS2-only.
//       _PER_PASS caps sections drawn in the PS2's immediate render path; the
//       Wii goes through RenderList and native GX chunk-list replay, which never reaches that
//       loop. _BLOCKS_PER_STEP sizes one step of the partial-build state
//       machine, and there are no partial builds here.
//
//       The other two -- MAX_RENDERER_UPDATES_PER_FRAME and
//       CHUNK_BUILD_BUDGET_MS -- ARE set now, see PLATFORM_MESH_BUDGET above.
//       They turned out not to need the capture-slot machinery this note used to
//       assume: only the "did this step do any work" question was entangled with
//       it, and that question does not arise when a build is atomic.
