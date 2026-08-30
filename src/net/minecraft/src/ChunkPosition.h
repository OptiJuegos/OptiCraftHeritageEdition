#pragma once

#include "java/Type.h"
#include "java/Arithmetic.h"
#include <cstdint>

// net.minecraft.src.ChunkPosition
class ChunkPosition
{
public:
	ChunkPosition(int_t i, int_t j, int_t k) : x(i), y(j), z(k) {}

	bool equals(const ChunkPosition &other) const
	{
		return other.x == x && other.y == y && other.z == z;
	}

	int_t hashCode() const
	{
		const std::uint32_t value = static_cast<std::uint32_t>(x) * UINT32_C(0x88f9fa)
		                          + static_cast<std::uint32_t>(y) * UINT32_C(0xef88b)
		                          + static_cast<std::uint32_t>(z);
		return JavaArithmetic::intFromBits(value);
	}

	int_t x;
	int_t y;
	int_t z;
};
