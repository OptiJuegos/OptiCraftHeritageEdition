#pragma once

#include <vector>

#include "java/Type.h"

// net.minecraft.src.NibbleArrayReader
class NibbleArrayReader
{
public:
	NibbleArrayReader(const std::vector<byte_t> &data, int_t depthBits);

	int_t get(int_t x, int_t y, int_t z) const;

	std::vector<byte_t> data;

private:
	int_t depthBits;
	int_t depthBitsPlusFour;
};
