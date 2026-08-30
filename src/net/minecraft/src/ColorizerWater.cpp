#include "ColorizerWater.h"

// Fallback water tint used when /misc/watercolor.png cannot be decoded. White
// means "no tint", which is safe: water keeps its texture color instead of
// going black from an all-zero buffer.
static const int_t WATER_FALLBACK_COLOR = 0xffffff;

std::vector<int_t> ColorizerWater::waterBuffer(0x10000, WATER_FALLBACK_COLOR);

void ColorizerWater::setWaterBiomeColorizer(const std::vector<int_t> &ai)
{
	// Only accept a full 256x256 colormap; see ColorizerGrass for rationale.
	if (ai.size() == 0x10000)
		waterBuffer = ai;
	else
		waterBuffer.assign(0x10000, WATER_FALLBACK_COLOR);
}
