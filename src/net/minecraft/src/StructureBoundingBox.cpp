#include "StructureBoundingBox.h"
#include "java/Arithmetic.h"

#include <algorithm>
#include <limits>
#include <sstream>

StructureBoundingBox::StructureBoundingBox()
	: minX(0), minY(0), minZ(0), maxX(0), maxY(0), maxZ(0)
{
}

StructureBoundingBox::StructureBoundingBox(int_t minXValue, int_t minYValue, int_t minZValue,
                                           int_t maxXValue, int_t maxYValue, int_t maxZValue)
	: minX(minXValue), minY(minYValue), minZ(minZValue),
	  maxX(maxXValue), maxY(maxYValue), maxZ(maxZValue)
{
}

StructureBoundingBox::StructureBoundingBox(int_t minXValue, int_t minZValue,
                                           int_t maxXValue, int_t maxZValue)
	: minX(minXValue), minY(1), minZ(minZValue),
	  maxX(maxXValue), maxY(512), maxZ(maxZValue)
{
}

StructureBoundingBox StructureBoundingBox::getNewBoundingBox()
{
	return StructureBoundingBox(
		std::numeric_limits<int_t>::max(),
		std::numeric_limits<int_t>::max(),
		std::numeric_limits<int_t>::max(),
		std::numeric_limits<int_t>::min(),
		std::numeric_limits<int_t>::min(),
		std::numeric_limits<int_t>::min());
}

StructureBoundingBox StructureBoundingBox::getComponentToAddBoundingBox(
	int_t x, int_t y, int_t z,
	int_t offsetX, int_t offsetY, int_t offsetZ,
	int_t sizeX, int_t sizeY, int_t sizeZ,
	int_t coordBaseMode)
{
	switch (coordBaseMode)
	{
	case 0:
		return StructureBoundingBox(JavaArithmetic::intAdd(x, offsetX), JavaArithmetic::intAdd(y, offsetY), JavaArithmetic::intAdd(z, offsetZ),
		                            JavaArithmetic::intAdd(JavaArithmetic::intSub(JavaArithmetic::intAdd(x, sizeX), 1), offsetX),
		                            JavaArithmetic::intAdd(JavaArithmetic::intSub(JavaArithmetic::intAdd(y, sizeY), 1), offsetY),
		                            JavaArithmetic::intAdd(JavaArithmetic::intSub(JavaArithmetic::intAdd(z, sizeZ), 1), offsetZ));
	case 1:
		return StructureBoundingBox(JavaArithmetic::intAdd(JavaArithmetic::intAdd(JavaArithmetic::intSub(x, sizeZ), 1), offsetZ),
		                            JavaArithmetic::intAdd(y, offsetY), JavaArithmetic::intAdd(z, offsetX),
		                            JavaArithmetic::intAdd(x, offsetZ),
		                            JavaArithmetic::intAdd(JavaArithmetic::intSub(JavaArithmetic::intAdd(y, sizeY), 1), offsetY),
		                            JavaArithmetic::intAdd(JavaArithmetic::intSub(JavaArithmetic::intAdd(z, sizeX), 1), offsetX));
	case 2:
		return StructureBoundingBox(JavaArithmetic::intAdd(x, offsetX), JavaArithmetic::intAdd(y, offsetY),
		                            JavaArithmetic::intAdd(JavaArithmetic::intAdd(JavaArithmetic::intSub(z, sizeZ), 1), offsetZ),
		                            JavaArithmetic::intAdd(JavaArithmetic::intSub(JavaArithmetic::intAdd(x, sizeX), 1), offsetX),
		                            JavaArithmetic::intAdd(JavaArithmetic::intSub(JavaArithmetic::intAdd(y, sizeY), 1), offsetY),
		                            JavaArithmetic::intAdd(z, offsetZ));
	case 3:
		return StructureBoundingBox(JavaArithmetic::intAdd(x, offsetZ), JavaArithmetic::intAdd(y, offsetY), JavaArithmetic::intAdd(z, offsetX),
		                            JavaArithmetic::intAdd(JavaArithmetic::intSub(JavaArithmetic::intAdd(x, sizeZ), 1), offsetZ),
		                            JavaArithmetic::intAdd(JavaArithmetic::intSub(JavaArithmetic::intAdd(y, sizeY), 1), offsetY),
		                            JavaArithmetic::intAdd(JavaArithmetic::intSub(JavaArithmetic::intAdd(z, sizeX), 1), offsetX));
	default:
		return StructureBoundingBox(JavaArithmetic::intAdd(x, offsetX), JavaArithmetic::intAdd(y, offsetY), JavaArithmetic::intAdd(z, offsetZ),
		                            JavaArithmetic::intAdd(JavaArithmetic::intSub(JavaArithmetic::intAdd(x, sizeX), 1), offsetX),
		                            JavaArithmetic::intAdd(JavaArithmetic::intSub(JavaArithmetic::intAdd(y, sizeY), 1), offsetY),
		                            JavaArithmetic::intAdd(JavaArithmetic::intSub(JavaArithmetic::intAdd(z, sizeZ), 1), offsetZ));
	}
}

bool StructureBoundingBox::intersectsWith(const StructureBoundingBox &other) const
{
	return maxX >= other.minX && minX <= other.maxX &&
	       maxZ >= other.minZ && minZ <= other.maxZ &&
	       maxY >= other.minY && minY <= other.maxY;
}

bool StructureBoundingBox::intersectsWith(int_t otherMinX, int_t otherMinZ,
                                          int_t otherMaxX, int_t otherMaxZ) const
{
	return maxX >= otherMinX && minX <= otherMaxX &&
	       maxZ >= otherMinZ && minZ <= otherMaxZ;
}

void StructureBoundingBox::expandTo(const StructureBoundingBox &other)
{
	minX = std::min(minX, other.minX);
	minY = std::min(minY, other.minY);
	minZ = std::min(minZ, other.minZ);
	maxX = std::max(maxX, other.maxX);
	maxY = std::max(maxY, other.maxY);
	maxZ = std::max(maxZ, other.maxZ);
}

void StructureBoundingBox::offset(int_t x, int_t y, int_t z)
{
	minX = JavaArithmetic::intAdd(minX, x);
	minY = JavaArithmetic::intAdd(minY, y);
	minZ = JavaArithmetic::intAdd(minZ, z);
	maxX = JavaArithmetic::intAdd(maxX, x);
	maxY = JavaArithmetic::intAdd(maxY, y);
	maxZ = JavaArithmetic::intAdd(maxZ, z);
}

bool StructureBoundingBox::isVecInside(int_t x, int_t y, int_t z) const
{
	return x >= minX && x <= maxX &&
	       z >= minZ && z <= maxZ &&
	       y >= minY && y <= maxY;
}

int_t StructureBoundingBox::getXSize() const { return JavaArithmetic::intAdd(JavaArithmetic::intSub(maxX, minX), 1); }
int_t StructureBoundingBox::getYSize() const { return JavaArithmetic::intAdd(JavaArithmetic::intSub(maxY, minY), 1); }
int_t StructureBoundingBox::getZSize() const { return JavaArithmetic::intAdd(JavaArithmetic::intSub(maxZ, minZ), 1); }
int_t StructureBoundingBox::getCenterX() const { return JavaArithmetic::intAdd(minX, getXSize() / 2); }
int_t StructureBoundingBox::getCenterY() const { return JavaArithmetic::intAdd(minY, getYSize() / 2); }
int_t StructureBoundingBox::getCenterZ() const { return JavaArithmetic::intAdd(minZ, getZSize() / 2); }

std::string StructureBoundingBox::toString() const
{
	std::ostringstream stream;
	stream << '(' << minX << ", " << minY << ", " << minZ
	       << "; " << maxX << ", " << maxY << ", " << maxZ << ')';
	return stream.str();
}
