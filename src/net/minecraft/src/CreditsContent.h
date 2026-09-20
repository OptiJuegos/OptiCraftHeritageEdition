#pragma once

#include <array>

namespace CreditsContent
{
// Short lines fit the 320x224 PS2 canvas without scrolling or shrinking text.
inline constexpr std::array<const char *, 12> lines = {{
    "Original OptiCraft project",
    "OptiCraft Contributors:",
    "OptiJuegos & Cooper Johnson",
    "",
    "Fork created by WaLL",
    "",
    "Minecraft and its related rights",
    "belong to Mojang Studios.",
    "",
    "Unofficial, fan-made fork.",
    "Not affiliated with or endorsed",
    "by Mojang Studios."
}};
inline constexpr int textTop = 44;
inline constexpr int lineHeight = 11;
inline constexpr int backHeight = 20;
inline int backY(int height) { return height - 42; }
}
