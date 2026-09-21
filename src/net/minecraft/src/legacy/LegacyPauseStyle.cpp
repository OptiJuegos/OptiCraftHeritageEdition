#include "LegacyPauseStyle.h"

namespace
{
constexpr long_t LEGACY_PAUSE_INPUT_DELAY_MILLIS = 150L;
}

int_t legacyPauseOverlayTopColor()
{
    return static_cast<int_t>(0x70000000u);
}

int_t legacyPauseOverlayBottomColor()
{
    return static_cast<int_t>(0x90000000u);
}

float_t legacyPauseButtonOpacity()
{
    return 1.0f;
}

int_t legacyPauseButtonCount()
{
    return 5;
}

bool legacyPauseInputDelayElapsed(long_t openedAtMillis, long_t nowMillis)
{
    return nowMillis - openedAtMillis >= LEGACY_PAUSE_INPUT_DELAY_MILLIS;
}
