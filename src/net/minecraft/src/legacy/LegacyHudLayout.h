#pragma once

#include "java/Type.h"

inline int_t legacyHudBottomInset(bool splitScreen = false)
{
    return splitScreen ? 16 : 28;
}

inline int_t legacyHudTooltipStripHeight()
{
    return 34;
}
