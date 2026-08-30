#include "ChunkCoordinates.h"
#include "java/Math.h"
#include "java/Arithmetic.h"

#include <cmath>
#include <cstdint>

ChunkCoordinates::ChunkCoordinates()
{
}

ChunkCoordinates::ChunkCoordinates(int_t i, int_t j, int_t k)
{
	x = i;
	y = j;
	z = k;
}

ChunkCoordinates::ChunkCoordinates(const ChunkCoordinates *chunkcoordinates)
{
	x = chunkcoordinates->x;
	y = chunkcoordinates->y;
	z = chunkcoordinates->z;
}

bool ChunkCoordinates::equals(const ChunkCoordinates *obj) const
{
	if (obj == nullptr)
		return false;
	else
		return x == obj->x && y == obj->y && z == obj->z;
}

int_t ChunkCoordinates::hashCode() const
{
	// Java masks int shift counts with 0x1f and wraps 32-bit arithmetic.
	// C++ shifting by >= 32 is undefined (normal world Y values hit that here).
	uint32_t value = static_cast<uint32_t>(x) + static_cast<uint32_t>(z);
	value <<= (static_cast<uint_t>(y) + 8u) & 31u;
	value <<= 16;
	return JavaArithmetic::intFromBits(value);
}

int_t ChunkCoordinates::compareChunkCoordinate(const ChunkCoordinates *chunkcoordinates) const
{
	if (y == chunkcoordinates->y)
	{
		if (z == chunkcoordinates->z)
			return JavaArithmetic::intSub(x, chunkcoordinates->x);
		else
			return JavaArithmetic::intSub(z, chunkcoordinates->z);
	}
	else
	{
		return JavaArithmetic::intSub(y, chunkcoordinates->y);
	}
}

void ChunkCoordinates::set(int_t i, int_t j, int_t k)
{
	x = i;
	y = j;
	z = k;
}

double ChunkCoordinates::getSqDistanceTo(int_t i, int_t j, int_t k) const
{
	int_t l = JavaArithmetic::intSub(x, i);
	int_t i1 = JavaArithmetic::intSub(y, j);
	int_t j1 = JavaArithmetic::intSub(z, k);
	int_t distanceSquared = JavaArithmetic::intAdd(
		JavaArithmetic::intAdd(JavaArithmetic::intMul(l, l), JavaArithmetic::intMul(i1, i1)),
		JavaArithmetic::intMul(j1, j1));
	return (double)distanceSquared;
}

double ChunkCoordinates::getEuclideanDistanceTo(int_t i, int_t j, int_t k) const
{
	return JavaMath::sqrt(getSqDistanceTo(i, j, k));
}

float ChunkCoordinates::getDistanceSquared(int_t i, int_t j, int_t k) const
{
	int_t dx = JavaArithmetic::intSub(x, i);
	int_t dy = JavaArithmetic::intSub(y, j);
	int_t dz = JavaArithmetic::intSub(z, k);
	int_t distanceSquared = JavaArithmetic::intAdd(
		JavaArithmetic::intAdd(JavaArithmetic::intMul(dx, dx), JavaArithmetic::intMul(dy, dy)),
		JavaArithmetic::intMul(dz, dz));
	return (float)distanceSquared;
}

int_t ChunkCoordinates::compareTo(const ChunkCoordinates *obj) const
{
	return compareChunkCoordinate(obj);
}
