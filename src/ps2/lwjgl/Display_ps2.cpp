// Display_ps2.cpp — PS2 implementation of lwjgl::Display.
// No SDL. Buffer swaps go through gsKit; messages are polled from the pad.
#ifdef PS2_PLATFORM

#include "platform/Log.h"
#include "lwjgl/Display.h"

#include "client/Minecraft.h"
#include "ps2/render/Ps2Tuning.h"
#include "ps2/render/Ps2Graphics.h"
#include "ps2/render/Ps2RenderBackend.h"
#include "ps2/input/Ps2Input.h"
#include "ps2/render/Ps2RenderPhase.h"
#include "ps2/system/Ps2Perf.h"
#include "java/System.h"

#include <gsKit.h>
#include <tamtypes.h>
#include <stdio.h>
#include <string.h>


namespace
{
    // WorldRenderer's greedy/step build alternates opaque (0) and translucent
    // (1) passes per section — see ps2BuildPass in WorldRenderer.cpp.
    static const int kMeshPassCount = 2;

    struct Ps2PerfCounters
    {
        long long frameStartNs = 0;
        long long frameSumNs = 0;
        long long frameMaxNs = 0;
        int frameCount = 0;

        long long gsSwapSumNs = 0;
        long long gsSwapMaxNs = 0;
        int gsSwapCount = 0;

        long long chunkBuildSumNs = 0;
        long long chunkBuildMaxNs = 0;
        int chunkBuildCount = 0;
        int chunkBuildVerts = 0;

        long long tickSumNs = 0;
        long long tickMaxNs = 0;
        int tickFrameCount = 0;
        int tickTotalCount = 0;
        int tickMaxCount = 0;

        long long renderSumNs = 0;
        long long renderMaxNs = 0;
        int renderCount = 0;

        long long displayUpdateSumNs = 0;
        long long displayUpdateMaxNs = 0;
        int displayUpdateCount = 0;

        long long lightingSumNs = 0;
        long long lightingMaxNs = 0;
        int lightingCount = 0;

        long long slowTickPhaseNs = 0;
        char slowTickPhase[32] = {};

		long long populateSumNs[PS2_POP_PHASE_COUNT] = {};
		long long populateMaxNs[PS2_POP_PHASE_COUNT] = {};
		int populateCount = 0;

        long long meshPassSumNs[kMeshPassCount] = {};
        long long meshPassMaxNs[kMeshPassCount] = {};
        int meshPassCount[kMeshPassCount] = {};
        int meshPassVerts[kMeshPassCount] = {};

        int snowColumnCalls = 0;
        int snowIcePlaced = 0;
        int snowPlacedCount = 0;
        long long snowNotifySum = 0;
        int snowNotifyMax = 0;

        long long chunkLoadSumNs = 0;
        long long chunkLoadMaxNs = 0;
        int chunkLoadCount = 0;

        // Distinct from populateSumNs[PS2_POP_TOTAL] above: that one times a
        // single PS2 populate task (ChunkProviderGeneratePopulateIncremental.cpp), this
        // times the per-tick drainPendingPopulate() call that can run several
        // of those tasks (ChunkProvider::unload100OldestChunks).
        long long populateDrainSumNs = 0;
        long long populateDrainMaxNs = 0;
        int populateDrainCount = 0;

        long long generateSumNs = 0;
        long long generateMaxNs = 0;
        int generateCount = 0;

        long long unloadSaveSumNs = 0;
        long long unloadSaveMaxNs = 0;
        int unloadSaveCount = 0;

        long long chunkEvictSumNs = 0;
        long long chunkEvictMaxNs = 0;
        int chunkEvictCount = 0;
    };

    static Ps2PerfCounters s_perf;

    // Render-phase attribution, in EE cycles. See Ps2RenderPhase.h.
    static unsigned long s_renderPhase[PS2_RPHASE_COUNT] = {};
    // Sum alone averages a single-frame spike away across the 120-frame
    // reporting window: an 8-way-split render can hide a multi-second outlier
    // inside one phase's per-frame average of a few tenths of a millisecond.
    // Tracked separately from the sum, per phase, so a spike like that is
    // attributable instead of merely visible in the coarser whole-render
    // avg/max pair.
    static unsigned long s_renderPhaseMax[PS2_RPHASE_COUNT] = {};
    static const char* const s_renderPhaseName[PS2_RPHASE_COUNT] =
        { "sky", "frustum", "build", "pass0", "ents", "pass1", "hand", "hud" };
}

extern "C" void ps2_perf_add_render_phase(int phase, unsigned int cycles)
{
    if (phase >= 0 && phase < PS2_RPHASE_COUNT)
    {
        s_renderPhase[phase] += cycles;
        if (cycles > s_renderPhaseMax[phase])
            s_renderPhaseMax[phase] = cycles;
    }
}

namespace
{

    static void ps2_perf_add_sample(long long ns, long long& sum, long long& max, int& count)
    {
        if (ns < 0)
            ns = 0;
        sum += ns;
        if (ns > max)
            max = ns;
        count++;
    }

    static float ps2_perf_ms(long long ns)
    {
        return (float)((double)ns / 1000000.0);
    }

    static float ps2_perf_avg_ms(long long sum, int count)
    {
        return count > 0 ? ps2_perf_ms(sum / count) : 0.0f;
    }

    static int ps2_perf_pct_of_60(float ms)
    {
        int pct = (int)((ms * 100.0f / 16.6667f) + 0.5f);
        if (pct < 0) pct = 0;
        return pct;
    }
}

extern "C" void ps2_perf_frame_begin()
{
    s_perf.frameStartNs = System::nanoTime();
}

extern "C" void ps2_perf_frame_end()
{
    if (s_perf.frameStartNs == 0)
        return;
    long long now = System::nanoTime();
    ps2_perf_add_sample(now - s_perf.frameStartNs,
                         s_perf.frameSumNs, s_perf.frameMaxNs, s_perf.frameCount);
    s_perf.frameStartNs = 0;
}

extern "C" void ps2_perf_add_chunk_build_ns(long long ns, int vertices)
{
    ps2_perf_add_sample(ns, s_perf.chunkBuildSumNs, s_perf.chunkBuildMaxNs, s_perf.chunkBuildCount);
    if (vertices > 0)
        s_perf.chunkBuildVerts += vertices;
}

extern "C" void ps2_perf_add_tick_ns(long long ns, int ticks)
{
    ps2_perf_add_sample(ns, s_perf.tickSumNs, s_perf.tickMaxNs, s_perf.tickFrameCount);
    if (ticks > 0)
        s_perf.tickTotalCount += ticks;
    if (ticks > s_perf.tickMaxCount)
        s_perf.tickMaxCount = ticks;
}

extern "C" void ps2_perf_add_render_ns(long long ns)
{
    ps2_perf_add_sample(ns, s_perf.renderSumNs, s_perf.renderMaxNs, s_perf.renderCount);
}

extern "C" void ps2_perf_add_display_update_ns(long long ns)
{
    ps2_perf_add_sample(ns, s_perf.displayUpdateSumNs, s_perf.displayUpdateMaxNs, s_perf.displayUpdateCount);
}

extern "C" void ps2_perf_add_lighting_ns(long long ns)
{
    ps2_perf_add_sample(ns, s_perf.lightingSumNs, s_perf.lightingMaxNs, s_perf.lightingCount);
}

extern "C" void ps2_perf_add_populate_phase(int phase, long long ns)
{
	if (phase < 0 || phase >= PS2_POP_PHASE_COUNT)
		return;
	if (ns < 0)
		ns = 0;
	s_perf.populateSumNs[phase] += ns;
	if (ns > s_perf.populateMaxNs[phase])
		s_perf.populateMaxNs[phase] = ns;
	if (phase == PS2_POP_TOTAL)
		s_perf.populateCount++;
}

extern "C" void ps2_perf_add_mesh_pass_ns(int pass, long long ns, int vertices)
{
    if (pass < 0 || pass >= kMeshPassCount)
        return;
    ps2_perf_add_sample(ns, s_perf.meshPassSumNs[pass], s_perf.meshPassMaxNs[pass],
                         s_perf.meshPassCount[pass]);
    if (vertices > 0)
        s_perf.meshPassVerts[pass] += vertices;
}

extern "C" void ps2_perf_add_snow_column(int icePlaced, int snowPlaced, int notifyCalls)
{
    s_perf.snowColumnCalls++;
    if (icePlaced)
        s_perf.snowIcePlaced++;
    if (snowPlaced)
        s_perf.snowPlacedCount++;
    if (notifyCalls > 0)
        s_perf.snowNotifySum += notifyCalls;
    if (notifyCalls > s_perf.snowNotifyMax)
        s_perf.snowNotifyMax = notifyCalls;
}

extern "C" void ps2_perf_add_chunk_load_ns(long long ns)
{
    ps2_perf_add_sample(ns, s_perf.chunkLoadSumNs, s_perf.chunkLoadMaxNs, s_perf.chunkLoadCount);
}

extern "C" void ps2_perf_add_populate_ns(long long ns)
{
    ps2_perf_add_sample(ns, s_perf.populateDrainSumNs, s_perf.populateDrainMaxNs,
                         s_perf.populateDrainCount);
}

extern "C" void ps2_perf_add_generate_ns(long long ns)
{
    ps2_perf_add_sample(ns, s_perf.generateSumNs, s_perf.generateMaxNs, s_perf.generateCount);
}

extern "C" void ps2_perf_add_unload_save_ns(long long ns)
{
    ps2_perf_add_sample(ns, s_perf.unloadSaveSumNs, s_perf.unloadSaveMaxNs, s_perf.unloadSaveCount);
}

extern "C" void ps2_perf_add_chunk_evict_ns(long long ns)
{
    ps2_perf_add_sample(ns, s_perf.chunkEvictSumNs, s_perf.chunkEvictMaxNs, s_perf.chunkEvictCount);
}

extern "C" void ps2_perf_note_tick_phase(const char* name, long long ns)
{
    if (name == nullptr || ns <= s_perf.slowTickPhaseNs)
        return;
    s_perf.slowTickPhaseNs = ns;
    strncpy(s_perf.slowTickPhase, name, sizeof(s_perf.slowTickPhase) - 1);
    s_perf.slowTickPhase[sizeof(s_perf.slowTickPhase) - 1] = '\0';
}

extern "C" void ps2_perf_format_and_reset(char* out, int outSize)
{
    if (out == nullptr || outSize <= 0)
        return;

    const float frameAvg = ps2_perf_avg_ms(s_perf.frameSumNs, s_perf.frameCount);
    const float frameMax = ps2_perf_ms(s_perf.frameMaxNs);
    const float gsAvg = ps2_perf_avg_ms(s_perf.gsSwapSumNs, s_perf.gsSwapCount);
    const float gsMax = ps2_perf_ms(s_perf.gsSwapMaxNs);
    const float chunkAvg = ps2_perf_avg_ms(s_perf.chunkBuildSumNs, s_perf.chunkBuildCount);
    const float chunkMax = ps2_perf_ms(s_perf.chunkBuildMaxNs);
    const float tickAvg = ps2_perf_avg_ms(s_perf.tickSumNs, s_perf.tickFrameCount);
    const float tickMax = ps2_perf_ms(s_perf.tickMaxNs);
    const float renderAvg = ps2_perf_avg_ms(s_perf.renderSumNs, s_perf.renderCount);
    const float renderMax = ps2_perf_ms(s_perf.renderMaxNs);
    const float displayAvg = ps2_perf_avg_ms(s_perf.displayUpdateSumNs, s_perf.displayUpdateCount);
    const float displayMax = ps2_perf_ms(s_perf.displayUpdateMaxNs);
    const float lightAvg = ps2_perf_avg_ms(s_perf.lightingSumNs, s_perf.lightingCount);
    const float lightMax = ps2_perf_ms(s_perf.lightingMaxNs);
    const float slowPhaseMs = ps2_perf_ms(s_perf.slowTickPhaseNs);
    const int framePct = ps2_perf_pct_of_60(frameAvg);
    const int gsPct = ps2_perf_pct_of_60(gsAvg);

    snprintf(out, outSize,
             "EE frame avg=%.1fms max=%.1fms ~%d%%60 | tick avg=%.1fms max=%.1fms ticks=%d maxTicks=%d slowTick=%s %.1fms | render avg=%.1fms max=%.1fms | display avg=%.1fms max=%.1fms | light avg=%.1fms max=%.1fms | GS wait avg=%.1fms max=%.1fms ~%d%%60 | chunk build n=%d avg=%.1fms max=%.1fms verts=%d",
             frameAvg, frameMax, framePct,
             tickAvg, tickMax, s_perf.tickTotalCount, s_perf.tickMaxCount,
             s_perf.slowTickPhase[0] ? s_perf.slowTickPhase : "none", slowPhaseMs,
             renderAvg, renderMax,
             displayAvg, displayMax,
             lightAvg, lightMax,
             gsAvg, gsMax, gsPct,
             s_perf.chunkBuildCount, chunkAvg, chunkMax, s_perf.chunkBuildVerts);

    // Per-phase breakdown of that render figure, appended to the same line.
    // These are per-frame averages so they read directly against "render avg";
    // what they do not account for is the rest of updateCameraAndRender (camera
    // setup, fog, particles, the held item and the HUD).
    {
        const int frames = s_perf.frameCount > 0 ? s_perf.frameCount : 1;
        size_t len = strlen(out);
        if (len < (size_t)outSize)
        {
            int n = snprintf(out + len, (size_t)outSize - len, " | render phases");
            len += (n > 0) ? (size_t)n : 0;
        }
        for (int p = 0; p < PS2_RPHASE_COUNT && len < (size_t)outSize; p++)
        {
            const double ms = (double)s_renderPhase[p] / 294000.0 / (double)frames;
            const double maxMs = (double)s_renderPhaseMax[p] / 294000.0;
            int n = snprintf(out + len, (size_t)outSize - len, " %s=%.1f/max%.1f",
                              s_renderPhaseName[p], ms, maxMs);
            len += (n > 0) ? (size_t)n : 0;
        }
        for (int p = 0; p < PS2_RPHASE_COUNT; p++)
        {
            s_renderPhase[p] = 0;
            s_renderPhaseMax[p] = 0;
        }
    }

	// Population runs synchronously inside the phase historically named
	// "chunkUnload". Report total average/max plus each feature group's maximum;
	// maxima are what explain the visible one-frame hitch.
	if (s_perf.populateCount > 0)
	{
		const float totalAvg = ps2_perf_avg_ms(
			s_perf.populateSumNs[PS2_POP_TOTAL], s_perf.populateCount);
		const float totalMax = ps2_perf_ms(s_perf.populateMaxNs[PS2_POP_TOTAL]);
		size_t len = strlen(out);
		if (len < (size_t)outSize)
		{
			snprintf(out + len, (size_t)outSize - len,
				" | pop n=%d avg/max=%.1f/%.1f max struct=%.1f lake=%.1f dung=%.1f fill=%.1f ore=%.1f deco=%.1f spring=%.1f snow=%.1f",
				s_perf.populateCount, totalAvg, totalMax,
				ps2_perf_ms(s_perf.populateMaxNs[PS2_POP_STRUCTURES]),
				ps2_perf_ms(s_perf.populateMaxNs[PS2_POP_LAKES]),
				ps2_perf_ms(s_perf.populateMaxNs[PS2_POP_DUNGEONS]),
				ps2_perf_ms(s_perf.populateMaxNs[PS2_POP_FILLERS]),
				ps2_perf_ms(s_perf.populateMaxNs[PS2_POP_ORES]),
				ps2_perf_ms(s_perf.populateMaxNs[PS2_POP_DECORATION]),
				ps2_perf_ms(s_perf.populateMaxNs[PS2_POP_SPRINGS]),
				ps2_perf_ms(s_perf.populateMaxNs[PS2_POP_SNOW]));
		}
	}

    // Streaming chunk lifecycle: load/generate/populate-drain/unload-save/evict
    // are each a single named span (unlike populate's per-feature breakdown
    // above), so one compact "n/avg/max" triplet per metric is enough.
    {
        size_t len = strlen(out);
        if (s_perf.chunkLoadCount > 0 && len < (size_t)outSize)
        {
            int n = snprintf(out + len, (size_t)outSize - len,
                " | chunkLoad n=%d avg=%.1fms max=%.1fms",
                s_perf.chunkLoadCount,
                ps2_perf_avg_ms(s_perf.chunkLoadSumNs, s_perf.chunkLoadCount),
                ps2_perf_ms(s_perf.chunkLoadMaxNs));
            len += (n > 0) ? (size_t)n : 0;
        }
        if (s_perf.generateCount > 0 && len < (size_t)outSize)
        {
            int n = snprintf(out + len, (size_t)outSize - len,
                " | generate n=%d avg=%.1fms max=%.1fms",
                s_perf.generateCount,
                ps2_perf_avg_ms(s_perf.generateSumNs, s_perf.generateCount),
                ps2_perf_ms(s_perf.generateMaxNs));
            len += (n > 0) ? (size_t)n : 0;
        }
        if (s_perf.populateDrainCount > 0 && len < (size_t)outSize)
        {
            int n = snprintf(out + len, (size_t)outSize - len,
                " | populateDrain n=%d avg=%.1fms max=%.1fms",
                s_perf.populateDrainCount,
                ps2_perf_avg_ms(s_perf.populateDrainSumNs, s_perf.populateDrainCount),
                ps2_perf_ms(s_perf.populateDrainMaxNs));
            len += (n > 0) ? (size_t)n : 0;
        }
        if (s_perf.unloadSaveCount > 0 && len < (size_t)outSize)
        {
            int n = snprintf(out + len, (size_t)outSize - len,
                " | unloadSave n=%d avg=%.1fms max=%.1fms",
                s_perf.unloadSaveCount,
                ps2_perf_avg_ms(s_perf.unloadSaveSumNs, s_perf.unloadSaveCount),
                ps2_perf_ms(s_perf.unloadSaveMaxNs));
            len += (n > 0) ? (size_t)n : 0;
        }
        if (s_perf.chunkEvictCount > 0 && len < (size_t)outSize)
        {
            int n = snprintf(out + len, (size_t)outSize - len,
                " | chunkEvict n=%d avg=%.1fms max=%.1fms",
                s_perf.chunkEvictCount,
                ps2_perf_avg_ms(s_perf.chunkEvictSumNs, s_perf.chunkEvictCount),
                ps2_perf_ms(s_perf.chunkEvictMaxNs));
            len += (n > 0) ? (size_t)n : 0;
        }
    }

    // Per-pass world mesh build cost (PS2_RENDER_STATS builds only feed this;
    // see the platformProfileChunkMeshPass call sites in WorldRenderer.cpp).
    {
        bool any = false;
        for (int p = 0; p < kMeshPassCount; p++)
            any = any || s_perf.meshPassCount[p] > 0;
        if (any)
        {
            size_t len = strlen(out);
            if (len < (size_t)outSize)
            {
                int n = snprintf(out + len, (size_t)outSize - len, " | mesh pass");
                len += (n > 0) ? (size_t)n : 0;
            }
            for (int p = 0; p < kMeshPassCount && len < (size_t)outSize; p++)
            {
                if (s_perf.meshPassCount[p] <= 0)
                    continue;
                int n = snprintf(out + len, (size_t)outSize - len,
                    " %d=avg%.1f/max%.1f/v%d", p,
                    ps2_perf_avg_ms(s_perf.meshPassSumNs[p], s_perf.meshPassCount[p]),
                    ps2_perf_ms(s_perf.meshPassMaxNs[p]), s_perf.meshPassVerts[p]);
                len += (n > 0) ? (size_t)n : 0;
            }
        }
    }

    if (s_perf.snowColumnCalls > 0)
    {
        const float notifyAvg = (float)s_perf.snowNotifySum / (float)s_perf.snowColumnCalls;
        size_t len = strlen(out);
        if (len < (size_t)outSize)
        {
            snprintf(out + len, (size_t)outSize - len,
                " | snow n=%d ice=%d snow=%d notify avg=%.1f max=%d",
                s_perf.snowColumnCalls, s_perf.snowIcePlaced, s_perf.snowPlacedCount,
                notifyAvg, s_perf.snowNotifyMax);
        }
    }

    s_perf = Ps2PerfCounters();
}

namespace lwjgl
{
namespace Display
{

static DisplayMode s_mode(640, 448);
static bool s_closeRequested = false;

void setDisplayMode(const DisplayMode& dm)
{
    // On PS2 the GS resolution is fixed at init; ignore what Minecraft requests
    // and report back the actual screen size so resize() converges immediately.
    (void)dm;
}

DisplayMode getDisplayMode() { return s_mode; }

void setTitle(const jstring&) {}
void setFullscreen(bool) {}
bool isCloseRequested() { return s_closeRequested; }
bool isVisible()        { return true; }
bool isActive()         { return true; }
void create()
{
    // GS is already up from main_ps2.cpp. Just sync the display mode.
    if (Ps2Graphics::context())
        s_mode = DisplayMode(Ps2Graphics::width(), Ps2Graphics::height());
}

int_t getX()      { return 0; }
int_t getY()      { return 0; }
int_t getWidth()  { return s_mode.getWidth(); }
int_t getHeight() { return s_mode.getHeight(); }

void processMessages()
{
    Minecraft* mc = Minecraft::getMinecraft();
    bool inMenu   = (mc && mc->currentScreen != nullptr);

    // Ensure the game captures mouse (camera) whenever no menu is open.
    if (!inMenu && mc && !mc->inGameHasFocus)
        mc->setIngameFocus();

    Ps2Input::poll(inMenu);
}

void swapBuffers()
{
    if (!gsGlobal) return;
    long long startNs = System::nanoTime();

    // Breadcrumbs, not statistics. Both calls below can block forever on a GIF
    // that another engine still owns -- gsKit_queue_exec drives Path3 and
    // gsKit_vsync_wait/sync_flip waits on the video field -- and neither prints
    // anything of its own. With sync writes on (McLog::setSyncWrites) the last
    // line in debug.log then names which one the console stopped inside, which
    // is not otherwise recoverable from a frozen picture.
    static long s_frame = 0;
    ++s_frame;

    // Before the exec, while pool_cur still holds what this frame wrote.
    //
    // gsKit does not bounds-check the queue (see ps2_gs_queue_guard in
    // Ps2GsQueue.cpp), and the per-draw guards only cover the sites that
    // call them. An overrun corrupts the C++ heap behind the pool and is felt
    // much later, as an allocation that never returns -- so it has to be caught
    // here, once per frame, whatever draw produced it. This runs in the menu
    // too, which the periodic FRAME line does not: that one is gated on a World
    // existing, and the frames before a world is created are exactly the ones
    // nothing was watching.
    ps2_gs_queue_check_overflow();
    ps2_gs_queue_report(s_frame);

    MC_LOG_TRACE("frame", "[PS2] frame %ld: gs queue exec\n", s_frame);
    gsKit_queue_exec(gsGlobal);
    const int activeBeforeFlip = (int)gsGlobal->ActiveBuffer;
    const int contextBeforeFlip = (int)gsGlobal->PrimContext;
    const int firstBeforeFlip = (int)gsGlobal->FirstFrame;
    MC_LOG_TRACE("frame", "[PS2] frame %ld: present active=%d ctx=%d first=%d\n",
                 s_frame, activeBeforeFlip, contextBeforeFlip, firstBeforeFlip);

#if PS2_TARGET_FPS > 0
    // Frame pacing. gsKit_sync_flip waits exactly one field and presents, so the
    // rate floats up to the full 60. To cap it, hold the finished frame for
    // however many extra fields the target period needs — the flip still lands
    // on a vblank, so there is no tearing.
    //
    // The loop bound is a FIELD COUNT, never a clock reading. A clock-gated wait
    // (`while (nanoTime() - last < period) vsync_wait();`) is unbounded:
    // System::nanoTime() resolves to newlib gettimeofday -> ps2sdk
    // _gettimeofday -> GetTimerSystemTime(), and when that timer is not running
    // it returns a fixed value, so the condition never goes false and the game
    // hangs on its first presented frame. Each gsKit_vsync_wait() blocks for at
    // most one field, so counting fields is bounded by construction.
    static int s_fieldsPerFrame = 0;
    if (s_fieldsPerFrame == 0)
    {
        const int fieldHz = (gsGlobal->Mode == GS_MODE_PAL) ? 50 : 60;
        s_fieldsPerFrame = (fieldHz + PS2_TARGET_FPS / 2) / PS2_TARGET_FPS;
        if (s_fieldsPerFrame < 1) s_fieldsPerFrame = 1;
        if (s_fieldsPerFrame > 4) s_fieldsPerFrame = 4;
    }

    // The clock is only ever an optimisation here: it lets a frame that already
    // overran its budget skip the remaining waits instead of being rounded down
    // to the next divisor of 60. Probe it once, and demand a plausible field
    // time (~16.7ms NTSC / 20ms PAL) rather than merely "it moved", so a dead or
    // garbage clock disables the shortcut instead of corrupting the pacing.
    static int s_clockUsable = -1;
    static long long s_lastPresentNs = 0;
    if (s_clockUsable < 0)
    {
        const long long probeNs = System::nanoTime();
        gsKit_vsync_wait();
        const long long fieldNs = System::nanoTime() - probeNs;
        s_clockUsable = (fieldNs > 1000000LL && fieldNs < 100000000LL) ? 1 : 0;
        // Worth printing once: this same clock is what every "tick=..ms
        // render=..ms" figure in the FRAME log is computed from. If it reads
        // dead here, those numbers are fiction and should not be trusted.
        MC_LOG_INFO("game", "[PS2] frame clock probe: one field = %lld ns -> nanoTime %s\n",
               fieldNs, s_clockUsable ? "USABLE" : "DEAD (perf timings are meaningless)");
        fflush(stdout);
    }

    const long long periodNs = 1000000000LL / (long long)PS2_TARGET_FPS;
    for (int field = 0; field < s_fieldsPerFrame; field++)
    {
        gsKit_vsync_wait();
        if (s_clockUsable && System::nanoTime() - s_lastPresentNs >= periodNs)
            break;
    }

    // gsKit_sync_flip is not "wait then setactive": its own source (ee/gs/src/
    // gsCore.c) does the actual display flip itself, inline, before delegating
    // to setactive:
    //     GS_SET_DISPFB2(ScreenBuffer[ActiveBuffer & 1], ...);
    //     ActiveBuffer ^= 1;
    //     gsKit_setactive(gsGlobal);
    // setactive() alone only reprograms the DRAW-target registers (FRAME_1/2)
    // to whatever ActiveBuffer already is -- it never touches DISPFB2 (which
    // buffer the CRTC scans out) and never toggles ActiveBuffer. Calling only
    // setactive() after the pacing waits, as this branch used to, left the
    // display showing the same buffer forever while the game kept drawing into
    // it: confirmed on PCSX2 as a frame frozen right after the boot screen,
    // with the "gsKit flip invariant failed: active 0->0" warning below firing
    // on the very first check. Vsync/host-refresh-rate settings do not affect
    // this -- it is a missing register write, not a timing issue. Replicate
    // sync_flip's own flip step here, guarded the same way it guards it.
    if (!firstBeforeFlip && gsGlobal->DoubleBuffering == GS_SETTING_ON)
    {
        GS_SET_DISPFB2(gsGlobal->ScreenBuffer[gsGlobal->ActiveBuffer & 1] / 8192,
                       gsGlobal->Width / 64, gsGlobal->PSM, 0, 0);
        gsGlobal->ActiveBuffer ^= 1;
    }
    gsKit_setactive(gsGlobal);
    s_lastPresentNs = System::nanoTime();
#else
    gsKit_sync_flip(gsGlobal);
#endif

    if (!firstBeforeFlip && gsGlobal->DoubleBuffering == GS_SETTING_ON)
    {
        static bool warnedBadFlip = false;
        const bool activeDidNotFlip = (int)gsGlobal->ActiveBuffer == activeBeforeFlip;
        if (!warnedBadFlip && activeDidNotFlip)
        {
            warnedBadFlip = true;
            MC_LOG_WARN("render",
                "[PS2] gsKit flip invariant failed: active %d->%d\n",
                activeBeforeFlip, (int)gsGlobal->ActiveBuffer);
        }
    }

    ps2_render_invalidate_framebuffer_state();
    gsKit_queue_reset(gsGlobal->Os_Queue);
    MC_LOG_TRACE("frame", "[PS2] frame %ld: presented active=%d ctx=%d first=%d\n",
                 s_frame, (int)gsGlobal->ActiveBuffer, (int)gsGlobal->PrimContext,
                 (int)gsGlobal->FirstFrame);
    long long endNs = System::nanoTime();
    ps2_perf_add_sample(endNs - startNs,
                         s_perf.gsSwapSumNs, s_perf.gsSwapMaxNs, s_perf.gsSwapCount);
}

void update(bool doProcessMessages)
{
    swapBuffers();
    if (doProcessMessages)
        processMessages();
}

} // namespace Display
} // namespace lwjgl

#endif // PS2_PLATFORM
