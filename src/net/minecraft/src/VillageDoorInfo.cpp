#include "VillageDoorInfo.h"

#include "java/Arithmetic.h"

VillageDoorInfo::VillageDoorInfo(int_t x, int_t y, int_t z, int_t insideX, int_t insideZ, int_t timestamp)
	: posX(x), posY(y), posZ(z), insideDirectionX(insideX), insideDirectionZ(insideZ),
	  lastActivityTimestamp(timestamp), isDetachedFromVillageFlag(false), doorOpeningRestrictionCounter(0)
{
}

int_t VillageDoorInfo::getDistanceSquared(int_t x, int_t y, int_t z) const
{
	int_t dx = JavaArithmetic::intSub(x, posX);
	int_t dy = JavaArithmetic::intSub(y, posY);
	int_t dz = JavaArithmetic::intSub(z, posZ);
	return JavaArithmetic::intAdd(JavaArithmetic::intAdd(JavaArithmetic::intMul(dx, dx), JavaArithmetic::intMul(dy, dy)), JavaArithmetic::intMul(dz, dz));
}

int_t VillageDoorInfo::getInsideDistanceSquare(int_t x, int_t y, int_t z) const
{
	int_t dx = JavaArithmetic::intSub(JavaArithmetic::intSub(x, posX), insideDirectionX);
	int_t dy = JavaArithmetic::intSub(y, posY);
	int_t dz = JavaArithmetic::intSub(JavaArithmetic::intSub(z, posZ), insideDirectionZ);
	return JavaArithmetic::intAdd(JavaArithmetic::intAdd(JavaArithmetic::intMul(dx, dx), JavaArithmetic::intMul(dy, dy)), JavaArithmetic::intMul(dz, dz));
}

int_t VillageDoorInfo::getInsidePosX() const { return JavaArithmetic::intAdd(posX, insideDirectionX); }
int_t VillageDoorInfo::getInsidePosY() const { return posY; }
int_t VillageDoorInfo::getInsidePosZ() const { return JavaArithmetic::intAdd(posZ, insideDirectionZ); }

bool VillageDoorInfo::isInside(int_t x, int_t z) const
{
	int_t dx = JavaArithmetic::intSub(x, posX);
	int_t dz = JavaArithmetic::intSub(z, posZ);
	return JavaArithmetic::intAdd(JavaArithmetic::intMul(dx, insideDirectionX), JavaArithmetic::intMul(dz, insideDirectionZ)) >= 0;
}

void VillageDoorInfo::resetDoorOpeningRestrictionCounter() { doorOpeningRestrictionCounter = 0; }
void VillageDoorInfo::incrementDoorOpeningRestrictionCounter() { doorOpeningRestrictionCounter = JavaArithmetic::intAdd(doorOpeningRestrictionCounter, 1); }
int_t VillageDoorInfo::getDoorOpeningRestrictionCounter() const { return doorOpeningRestrictionCounter; }
