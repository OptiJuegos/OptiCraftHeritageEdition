#include "PathNavigate.h"

#include <cmath>

#include "AxisAlignedBB.h"
#include "Block.h"
#include "EntityLiving.h"
#include "EntityMoveHelper.h"
#include "Material.h"
#include "MathHelper.h"
#include "java/Math.h"
#include "java/Arithmetic.h"
#include "PathEntity.h"
#include "PathPoint.h"
#include "Vec3D.h"
#include "World.h"

PathNavigate::PathNavigate(EntityLiving *entity, World *world, float range)
	: theEntity(entity), worldObj(world), currentPath(), speed(0.0f), pathSearchRange(range),
	  noSunPathfind(false), totalTicks(0), ticksAtLastPos(0), lastPosX(0.0), lastPosY(0.0), lastPosZ(0.0),
	  canPassOpenWoodenDoors(true), canPassClosedWoodenDoors(false), avoidsWater(false), canSwim(false)
{
}

PathNavigate::~PathNavigate() = default;

void PathNavigate::func_48664_a(bool value)
{
	setAvoidsWater(value);
}

void PathNavigate::setAvoidsWater(bool value)
{
	avoidsWater = value;
}

bool PathNavigate::func_48658_a() const
{
	return getAvoidsWater();
}

bool PathNavigate::getAvoidsWater() const
{
	return avoidsWater;
}

void PathNavigate::setBreakDoors(bool value)
{
	canPassClosedWoodenDoors = value;
}

void PathNavigate::func_48663_c(bool value)
{
	setEnterDoors(value);
}

void PathNavigate::setEnterDoors(bool value)
{
	canPassOpenWoodenDoors = value;
}

bool PathNavigate::func_48665_b() const
{
	return getCanBreakDoors();
}

bool PathNavigate::getCanBreakDoors() const
{
	return canPassClosedWoodenDoors;
}

void PathNavigate::func_48680_d(bool value)
{
	setAvoidSun(value);
}

void PathNavigate::setAvoidSun(bool value)
{
	noSunPathfind = value;
}

void PathNavigate::setSpeed(float value)
{
	speed = value;
}

void PathNavigate::func_48669_e(bool value)
{
	setCanSwim(value);
}

void PathNavigate::setCanSwim(bool value)
{
	canSwim = value;
}

PathEntity *PathNavigate::getPathToXYZ(double x, double y, double z)
{
	if (!canNavigate() || worldObj == nullptr || theEntity == nullptr)
		return nullptr;
	return worldObj->getEntityPathToXYZ(theEntity, MathHelper::floor_double(x), JavaArithmetic::doubleToInt(y), MathHelper::floor_double(z),
		pathSearchRange, canPassOpenWoodenDoors, canPassClosedWoodenDoors, avoidsWater, canSwim);
}

bool PathNavigate::func_48666_a(double x, double y, double z, float speedValue)
{
	return tryMoveToXYZ(x, y, z, speedValue);
}

bool PathNavigate::tryMoveToXYZ(double x, double y, double z, float newSpeed)
{
	return setPath(getPathToXYZ((double)MathHelper::floor_double(x), (double)JavaArithmetic::doubleToInt(y), (double)MathHelper::floor_double(z)), newSpeed);
}

PathEntity *PathNavigate::func_48679_a(EntityLiving *target)
{
	return getPathToEntityLiving(target);
}

PathEntity *PathNavigate::getPathToEntityLiving(EntityLiving *target)
{
	if (!canNavigate() || worldObj == nullptr || theEntity == nullptr || target == nullptr)
		return nullptr;
	return worldObj->getPathToEntity(theEntity, target, pathSearchRange,
		canPassOpenWoodenDoors, canPassClosedWoodenDoors, avoidsWater, canSwim);
}

bool PathNavigate::func_48667_a(EntityLiving *target, float speedValue)
{
	return tryMoveToEntityLiving(target, speedValue);
}

bool PathNavigate::tryMoveToEntityLiving(EntityLiving *target, float newSpeed)
{
	PathEntity *path = getPathToEntityLiving(target);
	return path != nullptr && setPath(path, newSpeed);
}

bool PathNavigate::setPath(PathEntity *path, float newSpeed)
{
	if (path == nullptr)
	{
		currentPath.reset();
		return false;
	}

	if (currentPath == nullptr || !path->isSamePath(currentPath.get()))
		currentPath.reset(path);
	else if (path != currentPath.get())
		delete path;

	if (noSunPathfind)
		removeSunnyPath();
	if (currentPath == nullptr || currentPath->getCurrentPathLength() == 0)
		return false;

	speed = newSpeed;
	ticksAtLastPos = totalTicks;
	lastPosX = theEntity->posX;
	lastPosY = static_cast<path_math_t>(getPathableYPos());
	lastPosZ = theEntity->posZ;
	return true;
}

PathEntity *PathNavigate::getPath() const
{
	return currentPath.get();
}

void PathNavigate::onUpdateNavigation()
{
	totalTicks = JavaArithmetic::intAdd(totalTicks, 1);
	if (noPath())
		return;
	if (canNavigate())
		pathFollow();
	if (noPath())
		return;
	double nodeX;
	double nodeY;
	double nodeZ;
	if (currentPath->getCurrentNodeCoordinates(theEntity, nodeX, nodeY, nodeZ))
	{
		EntityMoveHelper *moveHelper = theEntity->getMoveHelper();
		if (moveHelper != nullptr)
			moveHelper->setMoveTo(nodeX, nodeY, nodeZ, speed);
	}
}

void PathNavigate::pathFollow()
{
	const double positionX = theEntity->posX;
	const double positionY = (double)getPathableYPos();
	const double positionZ = theEntity->posZ;
	int_t sameLevelEnd = currentPath->getCurrentPathLength();
	for (int_t i = currentPath->getCurrentPathIndex(); i < currentPath->getCurrentPathLength(); ++i)
	{
		PathPoint *point = currentPath->getPathPointFromIndex(i);
		if (point != nullptr && point->yCoord != JavaArithmetic::doubleToInt(positionY))
		{
			sameLevelEnd = i;
			break;
		}
	}

	const float reachDistanceSq = theEntity->width * theEntity->width;
	for (int_t i = currentPath->getCurrentPathIndex(); i < sameLevelEnd; ++i)
	{
		double nodeX;
		double nodeY;
		double nodeZ;
		if (!currentPath->getVectorCoordinatesFromIndex(theEntity, i, nodeX, nodeY, nodeZ))
			continue;
#if PLATFORM_FLOAT_ENTITY_AI_MATH
		const float dx = (float)(nodeX - positionX);
		const float dy = (float)(nodeY - positionY);
		const float dz = (float)(nodeZ - positionZ);
		if (dx * dx + dy * dy + dz * dz < reachDistanceSq)
#else
		const double dx = nodeX - positionX;
		const double dy = nodeY - positionY;
		const double dz = nodeZ - positionZ;
		if (dx * dx + dy * dy + dz * dz < (double)reachDistanceSq)
#endif
			currentPath->setCurrentPathIndex(i + 1);
	}

#if PLATFORM_FLOAT_ENTITY_AI_MATH
	int_t sizeX = JavaArithmetic::floatToInt(std::ceil(theEntity->width));
#else
	int_t sizeX = JavaArithmetic::doubleToInt(std::ceil(static_cast<double>(theEntity->width)));
#endif
	int_t sizeY = JavaArithmetic::intAdd(JavaArithmetic::floatToInt(theEntity->height), 1);
	int_t sizeZ = sizeX;
	for (int_t i = sameLevelEnd - 1; i >= currentPath->getCurrentPathIndex(); --i)
	{
		double nodeX;
		double nodeY;
		double nodeZ;
		if (currentPath->getVectorCoordinatesFromIndex(theEntity, i, nodeX, nodeY, nodeZ) &&
			isDirectPathBetweenPoints(positionX, positionY, positionZ, nodeX, nodeY, nodeZ, sizeX, sizeY, sizeZ))
		{
			currentPath->setCurrentPathIndex(i);
			break;
		}
	}

	if (JavaArithmetic::intSub(totalTicks, ticksAtLastPos) > 100)
	{
		const path_math_t dx = static_cast<path_math_t>(positionX) - lastPosX;
		const path_math_t dy = static_cast<path_math_t>(positionY) - lastPosY;
		const path_math_t dz = static_cast<path_math_t>(positionZ) - lastPosZ;
		if (dx * dx + dy * dy + dz * dz < static_cast<path_math_t>(2.25))
			clearPathEntity();
		ticksAtLastPos = totalTicks;
		lastPosX = static_cast<path_math_t>(positionX);
		lastPosY = static_cast<path_math_t>(positionY);
		lastPosZ = static_cast<path_math_t>(positionZ);
	}
}

bool PathNavigate::noPath() const
{
	return currentPath == nullptr || currentPath->isFinished();
}

Vec3D *PathNavigate::getEntityPosition() const
{
	if (theEntity == nullptr)
		return nullptr;
	return Vec3D::createVector(theEntity->posX, (double)getPathableYPos(), theEntity->posZ);
}

void PathNavigate::clearPathEntity()
{
	currentPath.reset();
}

int_t PathNavigate::getPathableYPos() const
{
	if (theEntity->isInWater() && canSwim)
	{
		int_t y = JavaArithmetic::doubleToInt(theEntity->boundingBox->minY);
		int_t blockId = worldObj->getBlockId(MathHelper::floor_double(theEntity->posX), y, MathHelper::floor_double(theEntity->posZ));
		int_t count = 0;
		while (blockId == Block::waterMoving->blockID || blockId == Block::waterStill->blockID)
		{
			++y;
			blockId = worldObj->getBlockId(MathHelper::floor_double(theEntity->posX), y, MathHelper::floor_double(theEntity->posZ));
			if (++count > 16)
				return JavaArithmetic::doubleToInt(theEntity->boundingBox->minY);
		}
		return y;
	}
	return JavaArithmetic::doubleToInt(theEntity->boundingBox->minY + 0.5);
}

bool PathNavigate::canNavigate() const
{
	return theEntity != nullptr && (theEntity->onGround || (canSwim && isInLiquid()));
}

bool PathNavigate::func_48657_k() const
{
	return isInLiquid();
}

bool PathNavigate::isInLiquid() const
{
	return theEntity->isInWater() || theEntity->handleLavaMovement();
}

void PathNavigate::removeSunnyPath()
{
	if (currentPath == nullptr || worldObj == nullptr || theEntity == nullptr)
		return;
	if (worldObj->canBlockSeeTheSky(MathHelper::floor_double(theEntity->posX),
		JavaArithmetic::doubleToInt(theEntity->boundingBox->minY + 0.5), MathHelper::floor_double(theEntity->posZ)))
		return;
	for (int_t i = 0; i < currentPath->getCurrentPathLength(); ++i)
	{
		PathPoint *point = currentPath->getPathPointFromIndex(i);
		if (point != nullptr && worldObj->canBlockSeeTheSky(point->xCoord, point->yCoord, point->zCoord))
		{
			currentPath->setCurrentPathLength(i - 1);
			return;
		}
	}
}

bool PathNavigate::isDirectPathBetweenPoints(double startX, double startY, double startZ,
	double endX, double endY, double endZ, int_t sizeX, int_t sizeY, int_t sizeZ) const
{
	int_t x = MathHelper::floor_double(startX);
	int_t z = MathHelper::floor_double(startZ);
	const int_t startBlockX = x;
	const int_t startBlockZ = z;
	path_math_t dx = static_cast<path_math_t>(endX - startX);
	path_math_t dz = static_cast<path_math_t>(endZ - startZ);
	path_math_t lengthSq = dx * dx + dz * dz;
	if (lengthSq < static_cast<path_math_t>(1.0e-8))
		return false;
#if PLATFORM_FLOAT_ENTITY_AI_MATH
	path_math_t invLength = 1.0f / std::sqrt(lengthSq);
#else
	path_math_t invLength = 1.0 / JavaMath::sqrt(lengthSq);
#endif
	dx *= invLength;
	dz *= invLength;
	const path_math_t startFractionX = static_cast<path_math_t>(startX - static_cast<double>(startBlockX));
	const path_math_t startFractionZ = static_cast<path_math_t>(startZ - static_cast<double>(startBlockZ));
	sizeX += 2;
	sizeZ += 2;
	if (!isSafeToStandAt(x, JavaArithmetic::doubleToInt(startY), z, sizeX, sizeY, sizeZ,
		startBlockX, startBlockZ, startFractionX, startFractionZ, dx, dz))
		return false;
	sizeX -= 2;
	sizeZ -= 2;
	path_math_t stepX = static_cast<path_math_t>(1.0f) / std::fabs(dx);
	path_math_t stepZ = static_cast<path_math_t>(1.0f) / std::fabs(dz);
	path_math_t nextX = -startFractionX;
	path_math_t nextZ = -startFractionZ;
	if (dx >= static_cast<path_math_t>(0.0f)) ++nextX;
	if (dz >= static_cast<path_math_t>(0.0f)) ++nextZ;
	nextX /= dx;
	nextZ /= dz;
	int_t dirX = dx < static_cast<path_math_t>(0.0f) ? -1 : 1;
	int_t dirZ = dz < static_cast<path_math_t>(0.0f) ? -1 : 1;
	int_t endBlockX = MathHelper::floor_double(endX);
	int_t endBlockZ = MathHelper::floor_double(endZ);
	int_t remainingX = endBlockX - x;
	int_t remainingZ = endBlockZ - z;
	while (remainingX * dirX > 0 || remainingZ * dirZ > 0)
	{
		if (nextX < nextZ)
		{
			nextX += stepX;
			x += dirX;
			remainingX = endBlockX - x;
		}
		else
		{
			nextZ += stepZ;
			z += dirZ;
			remainingZ = endBlockZ - z;
		}
		if (!isSafeToStandAt(x, JavaArithmetic::doubleToInt(startY), z, sizeX, sizeY, sizeZ,
			startBlockX, startBlockZ, startFractionX, startFractionZ, dx, dz))
			return false;
	}
	return true;
}

bool PathNavigate::isSafeToStandAt(int_t x, int_t y, int_t z, int_t sizeX, int_t sizeY, int_t sizeZ,
	int_t startBlockX, int_t startBlockZ, path_math_t startFractionX, path_math_t startFractionZ,
	path_math_t dirX, path_math_t dirZ) const
{
	int_t minX = x - sizeX / 2;
	int_t minZ = z - sizeZ / 2;
	if (!isPositionClear(minX, y, minZ, sizeX, sizeY, sizeZ, startBlockX, startBlockZ,
		startFractionX, startFractionZ, dirX, dirZ))
		return false;
	for (int_t bx = minX; bx < minX + sizeX; ++bx)
	{
		for (int_t bz = minZ; bz < minZ + sizeZ; ++bz)
		{
			path_math_t relX = static_cast<path_math_t>(bx - startBlockX) + static_cast<path_math_t>(0.5f) - startFractionX;
			path_math_t relZ = static_cast<path_math_t>(bz - startBlockZ) + static_cast<path_math_t>(0.5f) - startFractionZ;
			if (relX * dirX + relZ * dirZ < static_cast<path_math_t>(0.0f))
				continue;
			int_t blockId = worldObj->getBlockId(bx, y - 1, bz);
			if (blockId <= 0 || blockId >= Block::BLOCK_REGISTRY_SIZE)
				return false;
			Block *block = Block::blocksList[blockId];
			if (block == nullptr)
				return false;
			Material *material = block->blockMaterial;
			if (material == Material::water && !theEntity->isInWater())
				return false;
			if (material == Material::lava)
				return false;
		}
	}
	return true;
}

bool PathNavigate::isPositionClear(int_t x, int_t y, int_t z, int_t sizeX, int_t sizeY, int_t sizeZ,
	int_t startBlockX, int_t startBlockZ, path_math_t startFractionX, path_math_t startFractionZ,
	path_math_t dirX, path_math_t dirZ) const
{
	for (int_t bx = x; bx < x + sizeX; ++bx)
	{
		for (int_t by = y; by < y + sizeY; ++by)
		{
			for (int_t bz = z; bz < z + sizeZ; ++bz)
			{
				path_math_t relX = static_cast<path_math_t>(bx - startBlockX) + static_cast<path_math_t>(0.5f) - startFractionX;
				path_math_t relZ = static_cast<path_math_t>(bz - startBlockZ) + static_cast<path_math_t>(0.5f) - startFractionZ;
				if (relX * dirX + relZ * dirZ < static_cast<path_math_t>(0.0f))
					continue;
				int_t blockId = worldObj->getBlockId(bx, by, bz);
				if (blockId > 0 && blockId < Block::BLOCK_REGISTRY_SIZE)
				{
					Block *block = Block::blocksList[blockId];
					if (block != nullptr && !block->getBlocksMovement(worldObj, bx, by, bz))
						return false;
				}
			}
		}
	}
	return true;
}
