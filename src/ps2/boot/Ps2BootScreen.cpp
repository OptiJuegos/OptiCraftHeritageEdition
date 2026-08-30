#ifdef PS2_PLATFORM

#include "ps2/boot/Ps2BootScreen.h"
#include "ps2/boot/Ps2BootRenderer.h"
#include "platform/Log.h"

#include <delaythread.h>

void ps2BootFlash(unsigned char r, unsigned char g, unsigned char b)
{
#if MC_LOG_LEVEL >= 1
    if (!Ps2BootRenderer::ready()) return;
    for (int i = 0; i < 50; ++i) {
        Ps2BootRenderer::clear({r, g, b, 0x80});
        Ps2BootRenderer::present();
    }
#else
    (void)r; (void)g; (void)b;
#endif
}

[[noreturn]] void ps2HaltNoData()
{
    for (;;) {
        for (int i = 0; i < 30; ++i) {
            const int on = i < 15;
            Ps2BootRenderer::clear({static_cast<unsigned char>(on ? 0xC0 : 0x20), 0, 0, 0x80});
            Ps2BootRenderer::present();
        }
    }
}

[[noreturn]] void ps2HaltBlack()
{
    for (;;) {
        Ps2BootRenderer::clear({0, 0, 0, 0x80});
        Ps2BootRenderer::present();
        DelayThread(1000000);
    }
}

#endif // PS2_PLATFORM
