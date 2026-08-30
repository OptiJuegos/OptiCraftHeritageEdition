#ifdef PS2_PLATFORM

#include "platform/Log.h"
#include "ps2/render/Ps2Graphics.h"

#include <cstdio>

#include <dma.h>
#include <dmaKit.h>
#include <graph.h>
#include <libvux.h>

GSGLOBAL* gsGlobal = nullptr;


namespace
{

static const int PS2_GS_VRAM_BYTES = 4 * 1024 * 1024;

// GSQUEUE keeps pool[2] — the pool is double-buffered, so this constant costs
// twice its value in EE RAM. The guard in the PS2 renderer guarantees that a
// draw flushes before it can exceed the remaining packet space.
// Telemetry across terrain streaming and the F3 overlay measured qpeak at
// 53-86KB, qdraw at 23-45KB and qover=0. Keep roughly 3x the observed peak for
// menus/weather spikes. This releases 1.5MB from the double-buffered queue.
static const int PS2_GSKIT_OS_QUEUE_BYTES = 256 * 1024;
static const int PS2_GSKIT_PER_QUEUE_BYTES = 64 * 1024;

bool detectPalRegion()
{
    const int region = graph_get_region();
    const bool isPal = region == GRAPH_MODE_PAL;
    MC_LOG_INFO("render", "[PS2] video region: %s\n", isPal ? "PAL" : "NTSC");
    return isPal;
}

} // namespace

extern "C" long ps2_dbg_gs_queue_ram_bytes()
{
    return static_cast<long>(PS2_GSKIT_OS_QUEUE_BYTES + PS2_GSKIT_PER_QUEUE_BYTES) * 2;
}

namespace Ps2Graphics
{

void initialize()
{
    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
                D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF);
#ifdef PS2_ENABLE_VU1_TERRAIN
    // VIF1 uses PS2SDK libdma rather than dmaKit. dmaKit's normal-send helper
    // does not program the VIF1 QWC register, while libdma does. Keep VIF1
    // outside dmaKit's fast-wait mask so GIF submissions stay independent.
    dma_channel_initialize(DMA_CHANNEL_VIF1, nullptr, 0);
#endif

    gsGlobal = gsKit_init_global_custom(PS2_GSKIT_OS_QUEUE_BYTES,
                                        PS2_GSKIT_PER_QUEUE_BYTES);

    const bool palRegion = detectPalRegion();
    gsGlobal->Mode = palRegion ? GS_MODE_PAL : GS_MODE_NTSC;
    gsGlobal->Interlace = GS_INTERLACED;
    gsGlobal->Field = GS_FIELD;
    gsGlobal->Width = 640;
    gsGlobal->Height = palRegion ? 512 : 448;
    gsGlobal->PSM = GS_PSM_CT16;
    gsGlobal->PSMZ = GS_PSMZ_16S;
    gsGlobal->DoubleBuffering = GS_SETTING_ON;
    gsGlobal->ZBuffering = GS_SETTING_ON;

    gsKit_init_screen(gsGlobal);
    gsKit_mode_switch(gsGlobal, GS_ONESHOT);
    gsKit_set_test(gsGlobal, GS_ZTEST_ON);
    gsKit_set_test(gsGlobal, GS_ATEST_ON);
    gsGlobal->Test->AFAIL = 0;
    gsGlobal->PrimAlpha = GS_SETREG_ALPHA(0, 1, 0, 1, 0);
    gsKit_set_primalpha(gsGlobal, gsGlobal->PrimAlpha, 0);

    VuInit();
    (void)PS2_GS_VRAM_BYTES;
}

GSGLOBAL* context()
{
    return gsGlobal;
}

int width()
{
    return gsGlobal ? static_cast<int>(gsGlobal->Width) : 640;
}

int height()
{
    return gsGlobal ? static_cast<int>(gsGlobal->Height) : 448;
}

bool isPal()
{
    return gsGlobal && gsGlobal->Mode == GS_MODE_PAL;
}

} // namespace Ps2Graphics

#endif // PS2_PLATFORM
