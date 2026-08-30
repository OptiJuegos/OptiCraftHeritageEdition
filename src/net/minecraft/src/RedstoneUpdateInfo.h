#pragma once

#include "java/Type.h"

// net.minecraft.src.RedstoneUpdateInfo
class RedstoneUpdateInfo
{
public:
	RedstoneUpdateInfo(int_t i, int_t j, int_t k, long_t l);

	int_t x;
	int_t y;
	int_t z;
	long_t updateTime;
};
