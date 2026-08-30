#pragma once

#include "java/Type.h"

// net.minecraft.src.EnumSkyBlock
// Java value-enum with a field -> class with static instances.
class EnumSkyBlock
{
private:
	EnumSkyBlock(int_t j);

public:
	static EnumSkyBlock *const Sky;
	static EnumSkyBlock *const Block;

	const int_t defaultLightValue;

private:
	static EnumSkyBlock *rotationPitch[2];
};
