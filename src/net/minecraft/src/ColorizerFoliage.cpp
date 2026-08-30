#include "ColorizerFoliage.h"

// Fallback foliage tint (matches getFoliageColorBasic) used when
// /misc/foliagecolor.png cannot be decoded. Without it getFoliageColor() would
// index an all-zero buffer and oak leaves would render black.
static const int_t FOLIAGE_FALLBACK_COLOR = 0x48b518;

std::vector<int_t> ColorizerFoliage::foliageBuffer(0x10000, FOLIAGE_FALLBACK_COLOR);

void ColorizerFoliage::setFoliageBiomeColorizer(const std::vector<int_t> &ai)
{
	// Only accept a full 256x256 colormap; see ColorizerGrass for rationale.
	if (ai.size() == 0x10000)
		foliageBuffer = ai;
	else
		foliageBuffer.assign(0x10000, FOLIAGE_FALLBACK_COLOR);
}

int_t ColorizerFoliage::getFoliageColor(double d, double d1)
{
	d1 *= d;
	int_t i = (int_t)((1.0 - d) * 255.0);
	int_t j = (int_t)((1.0 - d1) * 255.0);
	return foliageBuffer[j << 8 | i];
}

int_t ColorizerFoliage::getFoliageColorByIndex(int_t index)
{
	return foliageBuffer[index];
}

int_t ColorizerFoliage::getFoliageColorPine()
{
	return 0x619961;
}

int_t ColorizerFoliage::getFoliageColorBirch()
{
	return 0x80a755;
}

int_t ColorizerFoliage::getFoliageColorBasic()
{
	return 0x48b518;
}

int_t ColorizerFoliage::getSpawnDelay()
{
	return getFoliageColorBasic();
}
