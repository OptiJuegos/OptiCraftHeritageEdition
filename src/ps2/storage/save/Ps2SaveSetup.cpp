#ifdef PS2_PLATFORM

#include "platform/Log.h"
#include "ps2/storage/save/Ps2SaveSetup.h"

#include "ps2/boot/Ps2BootScreen.h"
#include "ps2/boot/Ps2BootRenderer.h"
#include "ps2/boot/SavesPromptPS2.h"
#include "ps2/storage/save/Ps2MemoryCard.h"
#include "ps2/storage/save/Ps2SaveStorage.h"

#include <cstdio>

namespace Ps2SaveSetup
{

void selectStorage()
{
    MC_LOG_INFO("save", "[PS2] init memory card...\n");
    const bool mcWritable = Ps2MemoryCard::initialize();
    ps2BootFlash(0, 255, 255);
    MC_LOG_DEBUG("ps2.boot", "[PS2] memory card init done (CYAN flash); save prompt is next\n");

    MC_LOG_INFO("save", "[PS2] save location prompt...\n");
    const SaveLocation saveLocation = ps2_show_saves_prompt();
    MC_LOG_INFO("save", "[PS2] save loc: %d\n", static_cast<int>(saveLocation));

    bool mcUsable = false;
    if (saveLocation == SAVE_LOC_MC)
    {
        bool cardReady = mcWritable;
        if (!cardReady && !Ps2MemoryCard::isFormatted())
        {
            if (ps2_show_format_prompt() && ps2_do_format_mc())
                cardReady = Ps2MemoryCard::probeWritable();
        }
        mcUsable = cardReady;
    }

    if (saveLocation == SAVE_LOC_MC && mcUsable)
        Ps2SaveStorage::setTarget(Ps2SaveStorage::Target::MemoryCard);
    else if (saveLocation == SAVE_LOC_MASS)
        Ps2SaveStorage::setTarget(Ps2SaveStorage::Target::MassStorage);
    else
        Ps2SaveStorage::setTarget(Ps2SaveStorage::Target::Disabled);

    MC_LOG_INFO("save", "[PS2] save path: %s\n", Ps2SaveStorage::displayRoot());

    // The boot save/format UI is drawn with the same double-buffered gsKit
    // context used by the game.  Both framebuffers can therefore still contain
    // the last "SAVES & WORLDS" frame when we hand control to Minecraft.
    // If a later GUI operation changes/rebuilds render state before both buffers
    // have been repainted, gsKit can expose that stale buffer for a field and the
    // old prompt appears to flicker over the menu.
    //
    // Finish the boot UI by explicitly painting/presenting BOTH buffers black.
    // Do this once here, after all possible format prompts, rather than inside
    // each prompt so selecting an unformatted Memory Card does not introduce an
    // unnecessary black flash between the two boot screens.
    if (Ps2BootRenderer::ready())
    {
        Ps2BootRenderer::resetAlpha();
        Ps2BootRenderer::setAlphaBlend(false);
        for (int buffer = 0; buffer < 2; ++buffer)
        {
            Ps2BootRenderer::clear({0, 0, 0, 0x80});
            Ps2BootRenderer::present();
        }
    }
}

} // namespace Ps2SaveSetup

#endif // PS2_PLATFORM
