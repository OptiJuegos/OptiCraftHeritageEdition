#pragma once

#include "java/Type.h"

class Minecraft;

// Legacy keeps its prompts on a single row along the bottom edge. The menus and
// the in-game HUD share these so both sit on the same line.
constexpr int_t LEGACY_HINT_MARGIN = 8;
constexpr int_t LEGACY_HINT_GAP = 6;

inline int_t legacyHintRowY(int_t screenHeight, bool splitScreen = false)
{
    return splitScreen ? (screenHeight - 12) : (screenHeight - 15);
}

void drawLegacyMenuHints(Minecraft *mc, int_t screenWidth, int_t screenHeight, bool showBack);
