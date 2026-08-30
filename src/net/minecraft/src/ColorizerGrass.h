#pragma once

#include <vector>
#include "java/Type.h"

// net.minecraft.src.ColorizerGrass
class ColorizerGrass
{
public:
	static void setGrassBiomeColorizer(const std::vector<int_t> &ai);
	static int_t getGrassColor(double d, double d1);
	static int_t getGrassColorByIndex(int_t index);

private:
	static std::vector<int_t> grassBuffer;
};
