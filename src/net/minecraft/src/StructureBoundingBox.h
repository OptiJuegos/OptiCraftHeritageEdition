#pragma once

#include <string>

#include "java/Type.h"

// net.minecraft.src.StructureBoundingBox
class StructureBoundingBox
{
public:
	StructureBoundingBox();
	StructureBoundingBox(const StructureBoundingBox &other) = default;
	StructureBoundingBox(int_t minX, int_t minY, int_t minZ,
	                     int_t maxX, int_t maxY, int_t maxZ);
	StructureBoundingBox(int_t minX, int_t minZ, int_t maxX, int_t maxZ);

	static StructureBoundingBox getNewBoundingBox();
	static StructureBoundingBox getComponentToAddBoundingBox(
		int_t x, int_t y, int_t z,
		int_t offsetX, int_t offsetY, int_t offsetZ,
		int_t sizeX, int_t sizeY, int_t sizeZ,
		int_t coordBaseMode);

	bool intersectsWith(const StructureBoundingBox &other) const;
	bool intersectsWith(int_t minX, int_t minZ, int_t maxX, int_t maxZ) const;
	void expandTo(const StructureBoundingBox &other);
	void offset(int_t x, int_t y, int_t z);
	bool isVecInside(int_t x, int_t y, int_t z) const;

	int_t getXSize() const;
	int_t getYSize() const;
	int_t getZSize() const;
	int_t getCenterX() const;
	int_t getCenterY() const;
	int_t getCenterZ() const;
	std::string toString() const;

	int_t minX;
	int_t minY;
	int_t minZ;
	int_t maxX;
	int_t maxY;
	int_t maxZ;
};
