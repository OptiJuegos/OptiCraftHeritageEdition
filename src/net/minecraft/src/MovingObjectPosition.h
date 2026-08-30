#pragma once

#include "java/Type.h"
#include "EnumMovingObjectType.h"

class Vec3D;
class Entity;

// net.minecraft.src.MovingObjectPosition
class MovingObjectPosition
{
public:
	MovingObjectPosition() = default;   // for static intercept slot in calculateIntercept
	MovingObjectPosition(int_t i, int_t j, int_t k, int_t l, Vec3D *vec3d);
	MovingObjectPosition(Entity *entity);

	EnumMovingObjectType typeOfHit;
	int_t blockX = 0;
	int_t blockY = 0;
	int_t blockZ = 0;
	int_t sideHit = 0;
	Vec3D *hitVec = nullptr;
	Entity *entityHit = nullptr;
};
