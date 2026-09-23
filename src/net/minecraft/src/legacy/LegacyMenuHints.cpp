#include "LegacyMenuHints.h"
#include "net/minecraft/src/UiStrings.h"

#include <string>
#include <algorithm>

#include "net/minecraft/src/FontRenderer.h"
#include "platform/PlatformConfig.h"

void drawLegacyMenuHints(FontRenderer *font, int_t screenWidth, int_t screenHeight, bool showBack)
{
    if (font == nullptr)
        return;

#if PLATFORM_PS2
    const std::string navigate = "[D-Pad] " + uiText("Navigate");
    const std::string select = "[X] " + uiText("Select");
    const std::string back = "[O] " + uiText("Back");
#elif PLATFORM_WII
    const std::string navigate = "[D-Pad] " + uiText("Navigate");
    const std::string select = "[A] " + uiText("Select");
    const std::string back = "[B] " + uiText("Back");
#else
    const std::string navigate = "[Up/Down] " + uiText("Navigate");
    const std::string select = "[Enter] " + uiText("Select");
    const std::string back = "[Esc] " + uiText("Back");
#endif

    const int_t y = legacyHintRowY(screenHeight);
    const int_t columns = showBack ? 3 : 2;
    const int_t cell = std::max<int_t>(1, (screenWidth - LEGACY_HINT_MARGIN * 2) / columns);
    const std::string labels[] = {navigate, select, back};
    for (int_t i = 0; i < columns; ++i)
        font->drawStringWithShadow(font->trimStringToWidth(labels[i], cell - 2),
            LEGACY_HINT_MARGIN + i * cell, y, 0xf0f0f0);
}
