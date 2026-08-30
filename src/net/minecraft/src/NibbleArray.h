#pragma once

#include <utility>
#include <vector>

#include "java/Type.h"

// net.minecraft.src.NibbleArray
class NibbleArray
{
public:
	// Beta 1.7.3 storage keeps Y as the least-significant coordinate. Existing
	// Chunk callers use getNibble()/setNibble(), which retain that layout.
	explicit NibbleArray(int_t size)
		: data((size_t)(size >> 1), 0)
		, depthBits(4)
		, depthBitsPlusFour(8)
	{
	}

	explicit NibbleArray(std::vector<byte_t> bytes)
		: data(std::move(bytes))
		, depthBits(4)
		, depthBitsPlusFour(8)
	{
	}

	// Minecraft 1.2.5 section storage uses a configurable depth-bit layout.
	NibbleArray(int_t size, int_t depthBitsIn)
		: data((size_t)(size >> 1), 0)
		, depthBits(depthBitsIn)
		, depthBitsPlusFour(depthBitsIn + 4)
	{
	}

	NibbleArray(std::vector<byte_t> bytes, int_t depthBitsIn)
		: data(std::move(bytes))
		, depthBits(depthBitsIn)
		, depthBitsPlusFour(depthBitsIn + 4)
	{
	}

	int_t getNibble(int_t i, int_t j, int_t k) const
	{
		int_t l = i << 11 | k << 7 | j;
		int_t i1 = l >> 1;
		int_t j1 = l & 1;
		if (j1 == 0)
			return data[(size_t)i1] & 0xf;
		else
			return (data[(size_t)i1] >> 4) & 0xf;
	}

	void setNibble(int_t i, int_t j, int_t k, int_t l)
	{
		int_t i1 = i << 11 | k << 7 | j;
		int_t j1 = i1 >> 1;
		int_t k1 = i1 & 1;
		if (k1 == 0)
			data[(size_t)j1] = (byte_t)(data[(size_t)j1] & 0xf0 | l & 0xf);
		else
			data[(size_t)j1] = (byte_t)(data[(size_t)j1] & 0xf | (l & 0xf) << 4);
	}

	int_t get(int_t x, int_t y, int_t z) const
	{
		int_t index = y << depthBitsPlusFour | z << depthBits | x;
		int_t byteIndex = index >> 1;
		int_t nibbleIndex = index & 1;
		return nibbleIndex == 0
			? data[(size_t)byteIndex] & 0xf
			: (data[(size_t)byteIndex] >> 4) & 0xf;
	}

	void set(int_t x, int_t y, int_t z, int_t value)
	{
		int_t index = y << depthBitsPlusFour | z << depthBits | x;
		int_t byteIndex = index >> 1;
		int_t nibbleIndex = index & 1;
		if (nibbleIndex == 0)
			data[(size_t)byteIndex] = (byte_t)(data[(size_t)byteIndex] & 0xf0 | value & 0xf);
		else
			data[(size_t)byteIndex] = (byte_t)(data[(size_t)byteIndex] & 0xf | (value & 0xf) << 4);
	}

	bool isValid() const { return true; }

	std::vector<byte_t> data;

private:
	int_t depthBits;
	int_t depthBitsPlusFour;
};
