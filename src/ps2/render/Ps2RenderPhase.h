#pragma once

#ifdef PS2_PLATFORM

// Coarse attribution of the render frame.
//
// "render avg" in the FRAME log is the whole of EntityRenderer::updateCameraAndRender,
// and the fast-draw counters only cover the three inner blocks of the terrain
// path (transform, project, packet build). Measured 2026-07-28 those three summed
// to 7.7ms against a render avg of 23.3ms, so two thirds of the render frame was
// unattributed and could not be reasoned about. These brackets close that gap by
// naming the phases updateCameraAndRender actually runs.
//
// Counted in EE cycles from COP0 Count (one tick per CPU cycle at ~294MHz, wraps
// every ~14.6s, which is far longer than the log period). Reading it is a single
// instruction, so bracketing a phase is free at this granularity.

enum Ps2RenderPhase
{
    PS2_RPHASE_SKY = 0,     // renderSky
    PS2_RPHASE_FRUSTUM,     // clipRenderersByFrustrum
    PS2_RPHASE_BUILD,       // updateRenderers -> chunk meshing steps
    PS2_RPHASE_PASS0,       // sortAndRender pass 0 (opaque terrain)
    PS2_RPHASE_ENTITIES,    // renderEntities
    PS2_RPHASE_PASS1,       // sortAndRender pass 1 (water/ice/glass)
    PS2_RPHASE_HAND,        // renderHand -> held item + overlays
    PS2_RPHASE_HUD,         // GuiIngame::renderGameOverlay
    PS2_RPHASE_COUNT
};

extern "C" void ps2_perf_add_render_phase(int phase, unsigned int cycles);

static inline unsigned int ps2_rphase_cycles(void)
{
    unsigned int c;
    __asm__ __volatile__("mfc0 %0, $9" : "=r"(c));
    return c;
}

// Bracket a phase. Declares a scoped start value, so two brackets in the same
// scope need different names.
#define PS2_RPHASE_BEGIN(v)        const unsigned int v = ps2_rphase_cycles()
#define PS2_RPHASE_END(v, phase)   ps2_perf_add_render_phase((phase), ps2_rphase_cycles() - (v))

#endif // PS2_PLATFORM
