#pragma once

#include "java/Type.h"
#include "java/Arithmetic.h"
#include "MathHelper.h"
#include <cstdint>

// net.minecraft.src.PathPoint
class PathPoint
{
public:
	PathPoint(int_t i, int_t j, int_t k)
		: xCoord(i), yCoord(j), zCoord(k),
		  hash(hashOf(i, j, k)), index(-1), isFirst(false),
		  totalPathDistance(0), distanceToNext(0), distanceToTarget(0),
		  previous(nullptr) {}

	static int_t hashOf(int_t i, int_t j, int_t k)
	{
		// Java int shifts/overflow wrap in two's complement. Build the same
		// 32-bit bit pattern with unsigned arithmetic to avoid C++ UB.
		uint32_t value = (uint32_t(j) & 0xffu)
			| ((uint32_t(i) & 0x7fffu) << 8)
			| ((uint32_t(k) & 0x7fffu) << 24)
			| (i >= 0 ? 0u : 0x80000000u)
			| (k >= 0 ? 0u : 0x8000u);
		return JavaArithmetic::intFromBits(value);
	}

	float squareDistanceTo(PathPoint *pathpoint) const
	{
		float f  = (float)JavaArithmetic::intSub(pathpoint->xCoord, xCoord);
		float f1 = (float)JavaArithmetic::intSub(pathpoint->yCoord, yCoord);
		float f2 = (float)JavaArithmetic::intSub(pathpoint->zCoord, zCoord);
		return f * f + f1 * f1 + f2 * f2;
	}

	float distanceTo(PathPoint *pathpoint) const
	{
		const float distanceSq = squareDistanceTo(pathpoint);
		if (distanceSq == 1.0f)
			return 1.0f;
		return MathHelper::sqrt_float(distanceSq);
	}

	bool equals(const PathPoint &other) const
	{
		return hash == other.hash && xCoord == other.xCoord
		       && yCoord == other.yCoord && zCoord == other.zCoord;
	}

	int_t hashCode() const { return hash; }
	bool isAssigned() const { return index >= 0; }

	const int_t xCoord;
	const int_t yCoord;
	const int_t zCoord;

	int_t index;
	float totalPathDistance;
	float distanceToNext;
	float distanceToTarget;
	PathPoint *previous;
	bool isFirst;

private:
	const int_t hash;
};
