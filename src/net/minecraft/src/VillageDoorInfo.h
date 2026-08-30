#pragma once

#include "java/Type.h"

// net.minecraft.src.VillageDoorInfo
class VillageDoorInfo
{
public:
	VillageDoorInfo(int_t x, int_t y, int_t z, int_t insideX, int_t insideZ, int_t timestamp);

	int_t getDistanceSquared(int_t x, int_t y, int_t z) const;
	int_t getInsideDistanceSquare(int_t x, int_t y, int_t z) const;
	int_t getInsidePosX() const;
	int_t getInsidePosY() const;
	int_t getInsidePosZ() const;
	bool isInside(int_t x, int_t z) const;
	void resetDoorOpeningRestrictionCounter();
	void incrementDoorOpeningRestrictionCounter();
	int_t getDoorOpeningRestrictionCounter() const;

	const int_t posX;
	const int_t posY;
	const int_t posZ;
	const int_t insideDirectionX;
	const int_t insideDirectionZ;
	int_t lastActivityTimestamp;
	bool isDetachedFromVillageFlag;

private:
	int_t doorOpeningRestrictionCounter;
};
