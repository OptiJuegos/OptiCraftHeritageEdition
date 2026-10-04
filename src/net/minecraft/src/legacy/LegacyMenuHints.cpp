#include "LegacyMenuHints.h"
#include "net/minecraft/src/ControlIcon.h"
#include "net/minecraft/src/UiStrings.h"
#include "platform/PlatformConfig.h"

void drawLegacyMenuHints(Minecraft *mc, int_t screenWidth, int_t screenHeight, bool showBack)
{
#if PLATFORM_PS2
    const std::string buttons[] = {"D-Pad", "Cross", "Circle"};
#elif PLATFORM_WII
    const std::string buttons[] = {"D-Pad", "A", "B"};
#elif PLATFORM_3DS
    // Touch owns the bottom panel, so the shared Navigate/Select pair is
    // noise under the buttons there -- the title's top panel already
    // carries the "A Select" prompt. Only the Back hint says something
    // the touch UI cannot; a screen without a back action gets no row at
    // all (count 0 makes drawControlHintRow a no-op).
    const std::string buttons[] = {"B"};
    const std::string actions[] = {uiText("Back")};
    drawControlHintRow(mc, screenWidth, legacyHintRowY(screenHeight), buttons, actions, showBack ? 1 : 0);
#else
    const std::string buttons[] = {"Up/Down", "Enter", "Esc"};
#endif
#if !PLATFORM_3DS
    const std::string actions[] = {uiText("Navigate"), uiText("Select"), uiText("Back")};
    drawControlHintRow(mc, screenWidth, legacyHintRowY(screenHeight), buttons, actions, showBack ? 3 : 2);
#endif
}
