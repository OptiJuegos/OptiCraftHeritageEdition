#include "LegacyUiPolicy.h"

bool legacyUiDefaultEnabled()
{
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
    return true;
#else
    return false;
#endif
}

// hardcoded badd
const char *legacyUiTitleResourcePath()
{
#if defined(PS2_PLATFORM)
    return "/title/mclogo.png";
#else
    return "/legacy/title.png";
#endif
}

std::string legacyUiOptionLabel(bool enabled)
{
    return std::string("Legacy UI: ") + (enabled ? "ON" : "OFF");
}
