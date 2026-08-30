#pragma once

#include "java/Type.h"

class EntityCreature;
class Vec3D;

// net.minecraft.src.RandomPositionGenerator
class RandomPositionGenerator
{
public:
	static Vec3D *findRandomTarget(EntityCreature *entity, int_t horizontalRange, int_t verticalRange);
	static Vec3D *findRandomTargetBlockTowards(EntityCreature *entity, int_t horizontalRange, int_t verticalRange, Vec3D *target);
	static Vec3D *findRandomTargetBlockAwayFrom(EntityCreature *entity, int_t horizontalRange, int_t verticalRange, Vec3D *target);

private:
	static Vec3D *findRandomTargetBlock(EntityCreature *entity, int_t horizontalRange, int_t verticalRange,
		double directionX, double directionY, double directionZ, bool hasDirection);
};
