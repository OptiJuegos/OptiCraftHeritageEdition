#include "AxisAlignedBB.h"

#include "Vec3D.h"
#include "MovingObjectPosition.h"

#include <memory>
#include <vector>

AxisAlignedBB AxisAlignedBB::pool[AxisAlignedBB::POOL_SIZE];
int_t AxisAlignedBB::numBoundingBoxesInUse = 0;

// Java relied on GC when the temporary AABB pool overflowed. In C++, returning
// new objects from getBoundingBoxFromPool() leaked them. Keep overflow objects
// owned here and reuse them after clearBoundingBoxPool().
static std::vector<std::unique_ptr<AxisAlignedBB>> g_axisAlignedBBOverflowPool;

AxisAlignedBB *AxisAlignedBB::getBoundingBox(double d, double d1, double d2, double d3, double d4, double d5)
{
	return new AxisAlignedBB(d, d1, d2, d3, d4, d5);
}

void AxisAlignedBB::clearBoundingBoxPool()
{
	numBoundingBoxesInUse = 0;
}

void AxisAlignedBB::trimBoundingBoxPool()
{
	numBoundingBoxesInUse = 0;
	g_axisAlignedBBOverflowPool.clear();
	g_axisAlignedBBOverflowPool.shrink_to_fit();
}

AxisAlignedBB *AxisAlignedBB::getBoundingBoxFromPool(double d, double d1, double d2, double d3, double d4, double d5)
{
	int_t index = numBoundingBoxesInUse++;
	if (index < POOL_SIZE)
		return pool[index].setBounds(d, d1, d2, d3, d4, d5);

	size_t overflowIndex = static_cast<size_t>(index - POOL_SIZE);
	if (overflowIndex >= g_axisAlignedBBOverflowPool.size())
		g_axisAlignedBBOverflowPool.emplace_back(new AxisAlignedBB(d, d1, d2, d3, d4, d5));
	return g_axisAlignedBBOverflowPool[overflowIndex]->setBounds(d, d1, d2, d3, d4, d5);
}

AxisAlignedBB::AxisAlignedBB(double d, double d1, double d2, double d3, double d4, double d5)
{
	minX = d;
	minY = d1;
	minZ = d2;
	maxX = d3;
	maxY = d4;
	maxZ = d5;
}

AxisAlignedBB *AxisAlignedBB::setBounds(double d, double d1, double d2, double d3, double d4, double d5)
{
	minX = d;
	minY = d1;
	minZ = d2;
	maxX = d3;
	maxY = d4;
	maxZ = d5;
	return this;
}

AxisAlignedBB *AxisAlignedBB::addCoord(double d, double d1, double d2)
{
	double d3 = minX;
	double d4 = minY;
	double d5 = minZ;
	double d6 = maxX;
	double d7 = maxY;
	double d8 = maxZ;
	if (d < 0.0)
		d3 += d;
	if (d > 0.0)
		d6 += d;
	if (d1 < 0.0)
		d4 += d1;
	if (d1 > 0.0)
		d7 += d1;
	if (d2 < 0.0)
		d5 += d2;
	if (d2 > 0.0)
		d8 += d2;
	return getBoundingBoxFromPool(d3, d4, d5, d6, d7, d8);
}

AxisAlignedBB *AxisAlignedBB::expand(double d, double d1, double d2)
{
	double d3 = minX - d;
	double d4 = minY - d1;
	double d5 = minZ - d2;
	double d6 = maxX + d;
	double d7 = maxY + d1;
	double d8 = maxZ + d2;
	return getBoundingBoxFromPool(d3, d4, d5, d6, d7, d8);
}

AxisAlignedBB *AxisAlignedBB::contract(double d, double d1, double d2)
{
	double d3 = minX + d;
	double d4 = minY + d1;
	double d5 = minZ + d2;
	double d6 = maxX - d;
	double d7 = maxY - d1;
	double d8 = maxZ - d2;
	return getBoundingBoxFromPool(d3, d4, d5, d6, d7, d8);
}

AxisAlignedBB *AxisAlignedBB::getOffsetBoundingBox(double d, double d1, double d2)
{
	return getBoundingBoxFromPool(minX + d, minY + d1, minZ + d2, maxX + d, maxY + d1, maxZ + d2);
}

double AxisAlignedBB::calculateXOffset(AxisAlignedBB *axisalignedbb, double d)
{
	if (axisalignedbb->maxY <= minY || axisalignedbb->minY >= maxY)
		return d;
	if (axisalignedbb->maxZ <= minZ || axisalignedbb->minZ >= maxZ)
		return d;
	if (d > 0.0 && axisalignedbb->maxX <= minX)
	{
		double d1 = minX - axisalignedbb->maxX;
		if (d1 < d)
			d = d1;
	}
	if (d < 0.0 && axisalignedbb->minX >= maxX)
	{
		double d2 = maxX - axisalignedbb->minX;
		if (d2 > d)
			d = d2;
	}
	return d;
}

double AxisAlignedBB::calculateYOffset(AxisAlignedBB *axisalignedbb, double d)
{
	if (axisalignedbb->maxX <= minX || axisalignedbb->minX >= maxX)
		return d;
	if (axisalignedbb->maxZ <= minZ || axisalignedbb->minZ >= maxZ)
		return d;
	if (d > 0.0 && axisalignedbb->maxY <= minY)
	{
		double d1 = minY - axisalignedbb->maxY;
		if (d1 < d)
			d = d1;
	}
	if (d < 0.0 && axisalignedbb->minY >= maxY)
	{
		double d2 = maxY - axisalignedbb->minY;
		if (d2 > d)
			d = d2;
	}
	return d;
}

double AxisAlignedBB::calculateZOffset(AxisAlignedBB *axisalignedbb, double d)
{
	if (axisalignedbb->maxX <= minX || axisalignedbb->minX >= maxX)
		return d;
	if (axisalignedbb->maxY <= minY || axisalignedbb->minY >= maxY)
		return d;
	if (d > 0.0 && axisalignedbb->maxZ <= minZ)
	{
		double d1 = minZ - axisalignedbb->maxZ;
		if (d1 < d)
			d = d1;
	}
	if (d < 0.0 && axisalignedbb->minZ >= maxZ)
	{
		double d2 = maxZ - axisalignedbb->minZ;
		if (d2 > d)
			d = d2;
	}
	return d;
}

bool AxisAlignedBB::intersectsWith(AxisAlignedBB *axisalignedbb)
{
	if (axisalignedbb->maxX <= minX || axisalignedbb->minX >= maxX)
		return false;
	if (axisalignedbb->maxY <= minY || axisalignedbb->minY >= maxY)
		return false;
	return axisalignedbb->maxZ > minZ && axisalignedbb->minZ < maxZ;
}

AxisAlignedBB *AxisAlignedBB::offset(double d, double d1, double d2)
{
	minX += d;
	minY += d1;
	minZ += d2;
	maxX += d;
	maxY += d1;
	maxZ += d2;
	return this;
}

bool AxisAlignedBB::isVecInside(Vec3D *vec3d)
{
	if (vec3d->xCoord <= minX || vec3d->xCoord >= maxX)
		return false;
	if (vec3d->yCoord <= minY || vec3d->yCoord >= maxY)
		return false;
	return vec3d->zCoord > minZ && vec3d->zCoord < maxZ;
}

double AxisAlignedBB::getAverageEdgeLength()
{
	double d = maxX - minX;
	double d1 = maxY - minY;
	double d2 = maxZ - minZ;
	return (d + d1 + d2) / 3.0;
}

AxisAlignedBB *AxisAlignedBB::copy()
{
	return getBoundingBoxFromPool(minX, minY, minZ, maxX, maxY, maxZ);
}

MovingObjectPosition *AxisAlignedBB::calculateIntercept(Vec3D *vec3d, Vec3D *vec3d1)
{
	Vec3D *vec3d2 = vec3d->getIntermediateWithXValue(vec3d1, minX);
	Vec3D *vec3d3 = vec3d->getIntermediateWithXValue(vec3d1, maxX);
	Vec3D *vec3d4 = vec3d->getIntermediateWithYValue(vec3d1, minY);
	Vec3D *vec3d5 = vec3d->getIntermediateWithYValue(vec3d1, maxY);
	Vec3D *vec3d6 = vec3d->getIntermediateWithZValue(vec3d1, minZ);
	Vec3D *vec3d7 = vec3d->getIntermediateWithZValue(vec3d1, maxZ);
	if (!isVecInYZ(vec3d2))
		vec3d2 = nullptr;
	if (!isVecInYZ(vec3d3))
		vec3d3 = nullptr;
	if (!isVecInXZ(vec3d4))
		vec3d4 = nullptr;
	if (!isVecInXZ(vec3d5))
		vec3d5 = nullptr;
	if (!isVecInXY(vec3d6))
		vec3d6 = nullptr;
	if (!isVecInXY(vec3d7))
		vec3d7 = nullptr;
	Vec3D *vec3d8 = nullptr;
	if (vec3d2 != nullptr && (vec3d8 == nullptr || vec3d->squareDistanceTo(vec3d2) < vec3d->squareDistanceTo(vec3d8)))
		vec3d8 = vec3d2;
	if (vec3d3 != nullptr && (vec3d8 == nullptr || vec3d->squareDistanceTo(vec3d3) < vec3d->squareDistanceTo(vec3d8)))
		vec3d8 = vec3d3;
	if (vec3d4 != nullptr && (vec3d8 == nullptr || vec3d->squareDistanceTo(vec3d4) < vec3d->squareDistanceTo(vec3d8)))
		vec3d8 = vec3d4;
	if (vec3d5 != nullptr && (vec3d8 == nullptr || vec3d->squareDistanceTo(vec3d5) < vec3d->squareDistanceTo(vec3d8)))
		vec3d8 = vec3d5;
	if (vec3d6 != nullptr && (vec3d8 == nullptr || vec3d->squareDistanceTo(vec3d6) < vec3d->squareDistanceTo(vec3d8)))
		vec3d8 = vec3d6;
	if (vec3d7 != nullptr && (vec3d8 == nullptr || vec3d->squareDistanceTo(vec3d7) < vec3d->squareDistanceTo(vec3d8)))
		vec3d8 = vec3d7;
	if (vec3d8 == nullptr)
		return nullptr;
	byte_t byte0 = -1;
	if (vec3d8 == vec3d2)
		byte0 = 4;
	if (vec3d8 == vec3d3)
		byte0 = 5;
	if (vec3d8 == vec3d4)
		byte0 = 0;
	if (vec3d8 == vec3d5)
		byte0 = 1;
	if (vec3d8 == vec3d6)
		byte0 = 2;
	if (vec3d8 == vec3d7)
		byte0 = 3;
	return new MovingObjectPosition(0, 0, 0, byte0, vec3d8);
}

bool AxisAlignedBB::isVecInYZ(Vec3D *vec3d)
{
	if (vec3d == nullptr)
		return false;
	else
		return vec3d->yCoord >= minY && vec3d->yCoord <= maxY && vec3d->zCoord >= minZ && vec3d->zCoord <= maxZ;
}

bool AxisAlignedBB::isVecInXZ(Vec3D *vec3d)
{
	if (vec3d == nullptr)
		return false;
	else
		return vec3d->xCoord >= minX && vec3d->xCoord <= maxX && vec3d->zCoord >= minZ && vec3d->zCoord <= maxZ;
}

bool AxisAlignedBB::isVecInXY(Vec3D *vec3d)
{
	if (vec3d == nullptr)
		return false;
	else
		return vec3d->xCoord >= minX && vec3d->xCoord <= maxX && vec3d->yCoord >= minY && vec3d->yCoord <= maxY;
}

void AxisAlignedBB::setBB(AxisAlignedBB *axisalignedbb)
{
	minX = axisalignedbb->minX;
	minY = axisalignedbb->minY;
	minZ = axisalignedbb->minZ;
	maxX = axisalignedbb->maxX;
	maxY = axisalignedbb->maxY;
	maxZ = axisalignedbb->maxZ;
}

jstring AxisAlignedBB::toString()
{
	return "box[" + String::toString(minX) + ", " + String::toString(minY) + ", " + String::toString(minZ) + " -> " + String::toString(maxX) + ", " + String::toString(maxY) + ", " + String::toString(maxZ) + "]";
}
