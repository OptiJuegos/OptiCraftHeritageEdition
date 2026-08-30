#pragma once

#include <vector>
#include "java/Type.h"

// net.minecraft.src.ColorizerFoliage
class ColorizerFoliage
{
public:
	static void setFoliageBiomeColorizer(const std::vector<int_t> &ai);
	static int_t getFoliageColor(double d, double d1);
	static int_t getFoliageColorByIndex(int_t index);
	static int_t getFoliageColorPine();
	static int_t getFoliageColorBirch();
	static int_t getFoliageColorBasic();
	static int_t getSpawnDelay();

private:
	static std::vector<int_t> foliageBuffer;
};
