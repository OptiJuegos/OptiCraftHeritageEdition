#ifdef PS2_PLATFORM

#include "ps2/boot/Ps2BootScreen.h"
#include "ps2/boot/Ps2BootRenderer.h"
#include "ps2/boot/Ps2BootText.h"
#include <delaythread.h>

#include "ps2/storage/assets/Ps2AssetLocator.h"

namespace
{

constexpr int kTextZ = 0xFFFF;
constexpr Ps2BootRenderer::Color kBlack = {0x00, 0x00, 0x00, 0x80};
constexpr Ps2BootRenderer::Color kWhite = {0xE8, 0xE8, 0xE8, 0x80};
constexpr Ps2BootRenderer::Color kMuted = {0xA8, 0xA8, 0xA8, 0x80};
constexpr Ps2BootRenderer::Color kYellow = {0xE8, 0xCC, 0x20, 0x80};

void drawMissingAssetsScreen()
{
    const float centerX = static_cast<float>(Ps2BootRenderer::width()) * 0.5f;
    const float centerY = static_cast<float>(Ps2BootRenderer::height()) * 0.5f;

    Ps2BootRenderer::setAlphaBlend(false);
    Ps2BootRenderer::clear(kBlack);

    Ps2BootText::drawCentered(centerX, centerY - 95.0f, kTextZ,
                              "Assets couldn't be loaded.", 2.5f, kWhite);
    Ps2BootText::drawCentered(centerX, centerY - 55.0f, kTextZ,
                              "Make sure the game files are located in:", 1.5f, kMuted);
    Ps2BootText::drawCentered(centerX, centerY - 25.0f, kTextZ,
                              "PS2 USB: mass:/OptiCraftHeritage", 1.5f, kWhite);
    Ps2BootText::drawCentered(centerX, centerY - 5.0f, kTextZ,
                              "PCSX2 Host: enable 'Host Filesystem' in settings", 1.5f, kYellow);

    const auto &diag = Ps2AssetLocator::diagnosticLogs();
    if (!diag.empty())
    {
        float logY = centerY + 20.0f;
        Ps2BootText::draw(15.0f, logY, kTextZ, "Diagnostics (Search Log):", 1.2f, kYellow);
        logY += 14.0f;
        for (size_t i = 0; i < diag.size() && i < 10; ++i)
        {
            Ps2BootText::draw(15.0f, logY, kTextZ, diag[i].c_str(), 1.0f, kMuted);
            logY += 12.0f;
        }
    }

    Ps2BootRenderer::present();
}

} // namespace

[[noreturn]] void ps2HaltNoData()
{
    for (;;)
        drawMissingAssetsScreen();
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
