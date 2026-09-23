#include "net/minecraft/src/UiStrings.h"
#include "LegacyOptionState.h"

#include <algorithm>

bool legacyCloudsChecked(int_t optifineCloudMode)
{
    return optifineCloudMode != 3;
}

int_t legacyCloudsToggledValue(int_t optifineCloudMode)
{
    return legacyCloudsChecked(optifineCloudMode) ? 3 : 0;
}

bool legacyFogChecked(bool fogOff)
{
    return !fogOff;
}

bool legacyFogToggledOff(bool fogOff)
{
    return !fogOff;
}

bool legacySmoothLightingChecked(float_t aoLevel)
{
    return aoLevel > 0.0f;
}

float_t legacySmoothLightingToggleValue(float_t aoLevel)
{
    return legacySmoothLightingChecked(aoLevel) ? 0.0f : 1.0f;
}

std::string legacyRenderDistanceLabel(int_t fineDistanceBlocks)
{
    const int_t chunks = std::max<int_t>(1, fineDistanceBlocks / 16);
    return uiText("Render Distance: ") + std::to_string(chunks);
}

std::string legacyFovLabel(float_t normalizedFov)
{
    const float_t value = std::max<float_t>(0.0f, std::min<float_t>(1.0f, normalizedFov));
    return uiText("FOV: ") + std::to_string(static_cast<int_t>(70.0f + value * 40.0f));
}

std::string legacySensitivityLabel(float_t normalizedSensitivity)
{
    const float_t value = std::max<float_t>(0.0f, std::min<float_t>(1.0f, normalizedSensitivity));
    return uiText("Sensitivity: ") + std::to_string(static_cast<int_t>(value * 200.0f)) + "%";
}
