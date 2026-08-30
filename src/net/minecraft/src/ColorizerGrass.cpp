#include "ColorizerGrass.h"

// Fallback grass tint used when /misc/grasscolor.png cannot be decoded (e.g. on
// PS2 where the colormap PNG may not load). Without it getGrassColor() would
// index an all-zero buffer and every grass block would render black.
static const int_t GRASS_FALLBACK_COLOR = 0x79c05a;

std::vector<int_t> ColorizerGrass::grassBuffer(0x10000, GRASS_FALLBACK_COLOR);

void ColorizerGrass::setGrassBiomeColorizer(const std::vector<int_t> &ai)
{
	// Only accept a full 256x256 colormap. readTextureImageData() silently
	// returns the (small) missing-texture buffer on failure, which would make
	// getGrassColor() read out of bounds and produce black/garbage colors.
	if (ai.size() == 0x10000)
		grassBuffer = ai;
	else
		grassBuffer.assign(0x10000, GRASS_FALLBACK_COLOR);
}

int_t ColorizerGrass::getGrassColor(double d, double d1)
{
	d1 *= d;
	int_t i = (int_t)((1.0 - d) * 255.0);
	int_t j = (int_t)((1.0 - d1) * 255.0);
	return grassBuffer[j << 8 | i];
}

int_t ColorizerGrass::getGrassColorByIndex(int_t index)
{
	return grassBuffer[index];
}
