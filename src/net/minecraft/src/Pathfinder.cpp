#include "PathFinder.h"

#include "platform/PlatformTuning.h"

#include "AxisAlignedBB.h"
#include "Block.h"
#include "BlockDoor.h"
#include "Entity.h"
#include "IBlockAccess.h"
#include "MCHash.h"
#include "MathHelper.h"
#include "Material.h"
#include "Path.h"
#include "PathEntity.h"
#include "PathPoint.h"
#include "java/Arithmetic.h"

#include <new>

Pathfinder::Pathfinder(IBlockAccess *iblockaccess)
	: Pathfinder(iblockaccess, true, false, false, false)
{
}

Pathfinder::Pathfinder(IBlockAccess *iblockaccess, bool woodenDoorAllowed, bool movementBlockAllowed, bool pathingInWater, bool entityCanDrown)
	: worldMap(iblockaccess),
	  isWoodenDoorAllowed(woodenDoorAllowed),
	  isMovementBlockAllowed(movementBlockAllowed),
	  isPathingInWater(pathingInWater),
	  canEntityDrown(entityCanDrown)
{
	path = new Path();
#if PLATFORM_BOUNDED_PATHFIND
	const int_t expectedPointCount = PLATFORM_PATHFIND_MAX_NODES * 4 + 2;
	const int_t minimumSlotCount = expectedPointCount * 2;
	int_t slotCount = 16;
	while (slotCount < minimumSlotCount)
		slotCount <<= 1;
	allocatedPoints.reserve(static_cast<std::size_t>(expectedPointCount));
	pointTable.resize(static_cast<std::size_t>(slotCount));
	pointTableMask = slotCount - 1;
#else
	pointMap = new MCHash();
#endif
}

Pathfinder::~Pathfinder()
{
	delete path;
	delete pointMap;
	for (PathPoint *point : allocatedPoints)
		delete point;
}

void Pathfinder::reset(IBlockAccess *blockAccess, bool woodenDoorAllowed, bool movementBlockAllowed,
                       bool pathingInWater, bool entityCanDrown)
{
	worldMap = blockAccess;
	isWoodenDoorAllowed = woodenDoorAllowed;
	isMovementBlockAllowed = movementBlockAllowed;
	isPathingInWater = pathingInWater;
	canEntityDrown = entityCanDrown;
}

PathEntity *Pathfinder::createEntityPathTo(Entity *entity, Entity *entity1, float f)
{
	return createEntityPathTo(entity, entity1->posX, entity1->boundingBox->minY, entity1->posZ, f);
}

PathEntity *Pathfinder::createEntityPathTo(Entity *entity, int_t i, int_t j, int_t k, float f)
{
	return createEntityPathTo(entity, (double)((float)i + 0.5f), (double)((float)j + 0.5f), (double)((float)k + 0.5f), f);
}

PathEntity *Pathfinder::createEntityPathTo(Entity *entity, double d, double d1, double d2, float f)
{
#if PLATFORM_BOUNDED_PATHFIND
	beginPointTableSearch();
#else
	pointMap->clearMap();
#endif
	activePointCount = 0;
	path->clearPath();
	bool oldPathingInWater = isPathingInWater;
	int_t startY = MathHelper::floor_double(entity->boundingBox->minY + 0.5);
	if (canEntityDrown && entity->isInWater())
	{
		startY = JavaArithmetic::doubleToInt(entity->boundingBox->minY);
		int_t blockId = worldMap->getBlockId(MathHelper::floor_double(entity->posX), startY, MathHelper::floor_double(entity->posZ));
		while (blockId == Block::waterMoving->blockID || blockId == Block::waterStill->blockID)
		{
			startY = JavaArithmetic::intAdd(startY, 1);
			blockId = worldMap->getBlockId(MathHelper::floor_double(entity->posX), startY, MathHelper::floor_double(entity->posZ));
		}
		isPathingInWater = false;
	}

	PathPoint *pathpoint = openPoint(MathHelper::floor_double(entity->boundingBox->minX), startY, MathHelper::floor_double(entity->boundingBox->minZ));
	PathPoint *pathpoint1 = openPoint(MathHelper::floor_double(d - (double)(entity->width / 2.0f)), MathHelper::floor_double(d1), MathHelper::floor_double(d2 - (double)(entity->width / 2.0f)));
	PathPoint pathpoint2(MathHelper::floor_float(entity->width + 1.0f), MathHelper::floor_float(entity->height + 1.0f), MathHelper::floor_float(entity->width + 1.0f));
	PathEntity *pathentity = addToPath(entity, pathpoint, pathpoint1, &pathpoint2, f);
	isPathingInWater = oldPathingInWater;
	return pathentity;
}

PathEntity *Pathfinder::addToPath(Entity *entity, PathPoint *pathpoint, PathPoint *pathpoint1, PathPoint *pathpoint2, float f)
{
	pathpoint->totalPathDistance = 0.0f;
	pathpoint->distanceToNext    = pathpoint->distanceTo(pathpoint1);
	pathpoint->distanceToTarget  = pathpoint->distanceToNext;
	path->clearPath();
	path->addPoint(pathpoint);
	PathPoint *pathpoint3 = pathpoint;
#if PLATFORM_CONSOLE_LOW || PLATFORM_WII
	// Bound A* cost on weak consoles. Vanilla expands the open set until it is
	// empty, which can scan thousands of nodes when the target is unreachable.
	// After the cap, bail and return the best-so-far path (pathpoint3 below).
	int_t nodesExpanded = 0;
#endif
	while (!path->isPathEmpty())
	{
#if PLATFORM_CONSOLE_LOW || PLATFORM_WII
		if (++nodesExpanded > PLATFORM_PATHFIND_MAX_NODES)
			break;
#endif
		PathPoint *pathpoint4 = path->dequeue();
		if (pathpoint4->equals(*pathpoint1))
		{
			return createEntityPath(pathpoint, pathpoint1);
		}
		if (pathpoint4->distanceToNext < pathpoint3->distanceToNext)
		{
			pathpoint3 = pathpoint4;
		}
		pathpoint4->isFirst = true;
		int_t i = findPathOptions(entity, pathpoint4, pathpoint2, pathpoint1, f);
		int_t j = 0;
		while (j < i)
		{
			PathPoint *pathpoint5 = pathOptions[j];
			float f1 = pathpoint4->totalPathDistance + pathpoint4->distanceTo(pathpoint5);
			if (!pathpoint5->isAssigned() || f1 < pathpoint5->totalPathDistance)
			{
				pathpoint5->previous = pathpoint4;
				pathpoint5->totalPathDistance = f1;
				pathpoint5->distanceToNext = pathpoint5->distanceTo(pathpoint1);
				if (pathpoint5->isAssigned())
				{
					path->changeDistance(pathpoint5, pathpoint5->totalPathDistance + pathpoint5->distanceToNext);
				}
				else
				{
					pathpoint5->distanceToTarget = pathpoint5->totalPathDistance + pathpoint5->distanceToNext;
					path->addPoint(pathpoint5);
				}
			}
			j++;
		}
	}
	if (pathpoint3 == pathpoint)
	{
		return nullptr;
	}
	return createEntityPath(pathpoint, pathpoint3);
}

int_t Pathfinder::findPathOptions(Entity *entity, PathPoint *pathpoint, PathPoint *pathpoint1, PathPoint *pathpoint2, float f)
{
	int_t i = 0;
	int_t j = 0;
	if (getVerticalOffset(entity, pathpoint->xCoord, JavaArithmetic::intAdd(pathpoint->yCoord, 1), pathpoint->zCoord, pathpoint1) == 1)
	{
		j = 1;
	}
	PathPoint *pathpoint3 = getSafePoint(entity, pathpoint->xCoord, pathpoint->yCoord, JavaArithmetic::intAdd(pathpoint->zCoord, 1), pathpoint1, j);
	PathPoint *pathpoint4 = getSafePoint(entity, JavaArithmetic::intSub(pathpoint->xCoord, 1), pathpoint->yCoord, pathpoint->zCoord, pathpoint1, j);
	PathPoint *pathpoint5 = getSafePoint(entity, JavaArithmetic::intAdd(pathpoint->xCoord, 1), pathpoint->yCoord, pathpoint->zCoord, pathpoint1, j);
	PathPoint *pathpoint6 = getSafePoint(entity, pathpoint->xCoord, pathpoint->yCoord, JavaArithmetic::intSub(pathpoint->zCoord, 1), pathpoint1, j);
	const float maxDistanceSq = f * f;
	if (pathpoint3 != nullptr && !pathpoint3->isFirst && pathpoint3->squareDistanceTo(pathpoint2) < maxDistanceSq) { pathOptions[i++] = pathpoint3; }
	if (pathpoint4 != nullptr && !pathpoint4->isFirst && pathpoint4->squareDistanceTo(pathpoint2) < maxDistanceSq) { pathOptions[i++] = pathpoint4; }
	if (pathpoint5 != nullptr && !pathpoint5->isFirst && pathpoint5->squareDistanceTo(pathpoint2) < maxDistanceSq) { pathOptions[i++] = pathpoint5; }
	if (pathpoint6 != nullptr && !pathpoint6->isFirst && pathpoint6->squareDistanceTo(pathpoint2) < maxDistanceSq) { pathOptions[i++] = pathpoint6; }
	return i;
}

PathPoint *Pathfinder::getSafePoint(Entity *entity, int_t i, int_t j, int_t k, PathPoint *pathpoint, int_t l)
{
	PathPoint *result = nullptr;
	int_t offset = getVerticalOffset(entity, i, j, k, pathpoint);
	if (offset == 2)
		return openPoint(i, j, k);
	if (offset == 1)
		result = openPoint(i, j, k);

	const int_t raisedY = JavaArithmetic::intAdd(j, l);
	if (result == nullptr && l > 0 && offset != -3 && offset != -4 &&
		getVerticalOffset(entity, i, raisedY, k, pathpoint) == 1)
	{
		result = openPoint(i, raisedY, k);
		j = raisedY;
	}

	if (result != nullptr)
	{
		int_t fallDistance = 0;
		int_t belowOffset = 0;
		while (j > 0)
		{
			belowOffset = getVerticalOffset(entity, i, JavaArithmetic::intSub(j, 1), k, pathpoint);
			if (isPathingInWater && belowOffset == -1)
				return nullptr;
			if (belowOffset != 1)
				break;
			if (++fallDistance >= 4)
				return nullptr;
			j = JavaArithmetic::intSub(j, 1);
			if (j > 0)
				result = openPoint(i, j, k);
		}
		if (belowOffset == -2)
			return nullptr;
	}
	return result;
}

PathPoint *Pathfinder::openPoint(int_t i, int_t j, int_t k)
{
	int_t l = PathPoint::hashOf(i, j, k);
#if PLATFORM_BOUNDED_PATHFIND
	PathPoint *pathpoint = pointTableOverflow ? lookupActivePoint(l) : lookupPointTable(l);
	if (pathpoint == nullptr)
	{
		pathpoint = acquirePoint(i, j, k);
		if (!pointTableOverflow && !insertPointTable(l, pathpoint))
			pointTableOverflow = true;
	}
#else
	PathPoint *pathpoint = (PathPoint *)pointMap->lookup(l);
	if (pathpoint == nullptr)
	{
		pathpoint = acquirePoint(i, j, k);
		pointMap->addKey(l, pathpoint);
	}
#endif
	return pathpoint;
}

PathPoint *Pathfinder::lookupPointTable(int_t hash) const
{
	if (pointTable.empty())
		return nullptr;

	int_t slotIndex = static_cast<int_t>(static_cast<uint32_t>(MCHash::getHash(hash)) &
	                                      static_cast<uint32_t>(pointTableMask));
	for (std::size_t probe = 0; probe < pointTable.size(); ++probe)
	{
		const PointMapSlot &slot = pointTable[static_cast<std::size_t>(slotIndex)];
		if (slot.generation != pointTableGeneration)
			return nullptr;
		if (slot.hash == hash)
			return slot.point;
		slotIndex = (slotIndex + 1) & pointTableMask;
	}
	return nullptr;
}

bool Pathfinder::insertPointTable(int_t hash, PathPoint *point)
{
	if (pointTable.empty())
		return false;

	int_t slotIndex = static_cast<int_t>(static_cast<uint32_t>(MCHash::getHash(hash)) &
	                                      static_cast<uint32_t>(pointTableMask));
	for (std::size_t probe = 0; probe < pointTable.size(); ++probe)
	{
		PointMapSlot &slot = pointTable[static_cast<std::size_t>(slotIndex)];
		if (slot.generation != pointTableGeneration)
		{
			slot.hash = hash;
			slot.point = point;
			slot.generation = pointTableGeneration;
			return true;
		}
		if (slot.hash == hash)
		{
			slot.point = point;
			return true;
		}
		slotIndex = (slotIndex + 1) & pointTableMask;
	}
	return false;
}

PathPoint *Pathfinder::lookupActivePoint(int_t hash) const
{
	for (int_t i = 0; i < activePointCount; ++i)
	{
		PathPoint *point = allocatedPoints[static_cast<std::size_t>(i)];
		if (point->hashCode() == hash)
			return point;
	}
	return nullptr;
}

void Pathfinder::beginPointTableSearch()
{
	++pointTableGeneration;
	if (pointTableGeneration == 0)
	{
		for (PointMapSlot &slot : pointTable)
			slot.generation = 0;
		pointTableGeneration = 1;
	}
	pointTableOverflow = false;
}

PathPoint *Pathfinder::acquirePoint(int_t i, int_t j, int_t k)
{
	PathPoint *point = nullptr;
	if (activePointCount < static_cast<int_t>(allocatedPoints.size()))
	{
		point = allocatedPoints[static_cast<std::size_t>(activePointCount)];
		point->~PathPoint();
		new (point) PathPoint(i, j, k);
	}
	else
	{
		point = new PathPoint(i, j, k);
		allocatedPoints.push_back(point);
	}
	activePointCount++;
	return point;
}

int_t Pathfinder::getVerticalOffset(Entity *entity, int_t i, int_t j, int_t k, PathPoint *pathpoint)
{
	bool specialBlock = false;
	const int_t endX = JavaArithmetic::intAdd(i, pathpoint->xCoord);
	const int_t endY = JavaArithmetic::intAdd(j, pathpoint->yCoord);
	const int_t endZ = JavaArithmetic::intAdd(k, pathpoint->zCoord);
	for (int_t x = i; x < endX; ++x)
	{
		for (int_t y = j; y < endY; ++y)
		{
			for (int_t z = k; z < endZ; ++z)
			{
				int_t blockId = worldMap->getBlockId(x, y, z);
				if (blockId <= 0)
					continue;

				if (Block::trapdoor != nullptr && blockId == Block::trapdoor->blockID)
				{
					specialBlock = true;
				}
				else if (blockId != Block::waterMoving->blockID && blockId != Block::waterStill->blockID)
				{
					if (!isWoodenDoorAllowed && blockId == Block::doorWood->blockID)
						return 0;
				}
				else
				{
					if (isPathingInWater)
						return -1;
					specialBlock = true;
				}

				Block *block = Block::blocksList[blockId];
				if (block == nullptr)
					continue;
				if (!block->getBlocksMovement(worldMap, x, y, z) && (!isMovementBlockAllowed || blockId != Block::doorWood->blockID))
				{
					if (blockId == Block::fence->blockID || (Block::fenceGate != nullptr && blockId == Block::fenceGate->blockID))
						return -3;
					if (Block::trapdoor != nullptr && blockId == Block::trapdoor->blockID)
						return -4;
					if (block->blockMaterial != Material::lava)
						return 0;
					if (!entity->handleLavaMovement())
						return -2;
				}
			}
		}
	}
	return specialBlock ? 2 : 1;
}

PathEntity *Pathfinder::createEntityPath(PathPoint *pathpoint, PathPoint *pathpoint1)
{
	int_t i = 1;
	for (PathPoint *pathpoint2 = pathpoint1; pathpoint2->previous != nullptr; pathpoint2 = pathpoint2->previous)
	{
		i++;
	}
	std::vector<PathPoint *> apathpoint(i, nullptr);
	PathPoint *pathpoint3 = pathpoint1;
	apathpoint[--i] = pathpoint3;
	while (pathpoint3->previous != nullptr)
	{
		pathpoint3 = pathpoint3->previous;
		apathpoint[--i] = pathpoint3;
	}
	return new PathEntity(apathpoint);
}
