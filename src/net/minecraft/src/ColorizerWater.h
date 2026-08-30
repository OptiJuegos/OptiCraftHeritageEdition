#pragma once

#include <vector>
#include "java/Type.h"

// net.minecraft.src.ColorizerWater
class ColorizerWater
{
public:
	static void setWaterBiomeColorizer(const std::vector<int_t> &ai);

private:
	static std::vector<int_t> waterBuffer;
};
