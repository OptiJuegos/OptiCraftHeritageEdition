#include "RandomPositionGenerator.h"

#include "ChunkCoordinates.h"
#include "EntityCreature.h"
#include "MathHelper.h"
#include "Vec3D.h"

Vec3D *RandomPositionGenerator::findRandomTarget(EntityCreature *entity, int_t horizontalRange, int_t verticalRange)
{
	return findRandomTargetBlock(entity, horizontalRange, verticalRange, 0.0, 0.0, 0.0, false);
}

Vec3D *RandomPositionGenerator::findRandomTargetBlockTowards(EntityCreature *entity, int_t horizontalRange, int_t verticalRange, Vec3D *target)
{
	if (entity == nullptr || target == nullptr)
		return nullptr;
	return findRandomTargetBlock(entity, horizontalRange, verticalRange,
		target->xCoord - entity->posX, target->yCoord - entity->posY, target->zCoord - entity->posZ, true);
}

Vec3D *RandomPositionGenerator::findRandomTargetBlockAwayFrom(EntityCreature *entity, int_t horizontalRange, int_t verticalRange, Vec3D *target)
{
	if (entity == nullptr || target == nullptr)
		return nullptr;
	return findRandomTargetBlock(entity, horizontalRange, verticalRange,
		entity->posX - target->xCoord, entity->posY - target->yCoord, entity->posZ - target->zCoord, true);
}

Vec3D *RandomPositionGenerator::findRandomTargetBlock(EntityCreature *entity, int_t horizontalRange, int_t verticalRange,
	double directionX, double directionY, double directionZ, bool hasDirection)
{
	if (entity == nullptr)
		return nullptr;
	Random &random = entity->getRNG();
	bool found = false;
	int_t bestX = 0;
	int_t bestY = 0;
	int_t bestZ = 0;
	float bestWeight = -99999.0f;
	bool enforceHome = false;
	if (entity->hasHome())
	{
		ChunkCoordinates home = entity->getHomePosition();
		double distance = home.getEuclideanDistanceTo(MathHelper::floor_double(entity->posX),
			MathHelper::floor_double(entity->posY), MathHelper::floor_double(entity->posZ)) + 4.0;
		enforceHome = distance < (double)(entity->getMaximumHomeDistance() + (float)horizontalRange);
	}

	for (int_t attempt = 0; attempt < 10; ++attempt)
	{
		int_t dx = random.nextInt(horizontalRange * 2) - horizontalRange;
		int_t dy = random.nextInt(verticalRange * 2) - verticalRange;
		int_t dz = random.nextInt(horizontalRange * 2) - horizontalRange;
		if (hasDirection && (double)dx * directionX + (double)dz * directionZ < 0.0)
			continue;
		int_t x = dx + MathHelper::floor_double(entity->posX);
		int_t y = dy + MathHelper::floor_double(entity->posY);
		int_t z = dz + MathHelper::floor_double(entity->posZ);
		if (enforceHome && !entity->isWithinHomeDistance(x, y, z))
			continue;
		float weight = entity->getBlockPathWeight(x, y, z);
		if (weight > bestWeight)
		{
			bestWeight = weight;
			bestX = x;
			bestY = y;
			bestZ = z;
			found = true;
		}
	}
	return found ? Vec3D::createVector((double)bestX, (double)bestY, (double)bestZ) : nullptr;
}
