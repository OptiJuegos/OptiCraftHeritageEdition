#include "MovingObjectPosition.h"

#include "Vec3D.h"
#include "Entity.h"

MovingObjectPosition::MovingObjectPosition(int_t i, int_t j, int_t k, int_t l, Vec3D *vec3d)
{
	typeOfHit = EnumMovingObjectType::TILE;
	blockX = i;
	blockY = j;
	blockZ = k;
	sideHit = l;
	hitVec = Vec3D::createVector(vec3d->xCoord, vec3d->yCoord, vec3d->zCoord);
}

MovingObjectPosition::MovingObjectPosition(Entity *entity)
{
	typeOfHit = EnumMovingObjectType::ENTITY;
	entityHit = entity;
	hitVec = Vec3D::createVector(entity->posX, entity->posY, entity->posZ);
}
