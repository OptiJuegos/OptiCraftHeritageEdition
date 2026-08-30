#include "PathEntity.h"

#include "PathPoint.h"
#include "Entity.h"
#include "Vec3D.h"
#include "platform/PlatformTuning.h"
#include "java/Arithmetic.h"

PathEntity::PathEntity(const std::vector<PathPoint *> &apathpoint)
	: pathIndex(0), pathLength((int_t)apathpoint.size())
{
	points.reserve(apathpoint.size());
	for (PathPoint *point : apathpoint)
		points.emplace_back(point->xCoord, point->yCoord, point->zCoord);
}

void PathEntity::incrementPathIndex()
{
	pathIndex++;
}

bool PathEntity::isFinished()
{
	return pathIndex >= pathLength;
}

PathPoint *PathEntity::getFinalPathPoint()
{
	if (pathLength > 0)
	{
		return &points[pathLength - 1];
	}
	return nullptr;
}


PathPoint *PathEntity::getPathPointFromIndex(int_t index)
{
	if (index < 0 || index >= pathLength)
		return nullptr;
	return &points[(size_t)index];
}

int_t PathEntity::getCurrentPathLength() const
{
	return pathLength;
}

void PathEntity::setCurrentPathLength(int_t length)
{
	if (length < 0)
		length = 0;
	if (length > (int_t)points.size())
		length = (int_t)points.size();
	pathLength = length;
	if (pathIndex > pathLength)
		pathIndex = pathLength;
}

int_t PathEntity::getCurrentPathIndex() const
{
	return pathIndex;
}

void PathEntity::setCurrentPathIndex(int_t index)
{
	if (index < 0)
		index = 0;
	if (index > pathLength)
		index = pathLength;
	pathIndex = index;
}

bool PathEntity::getVectorCoordinatesFromIndex(Entity *entity, int_t index, double &x, double &y, double &z) const
{
	if (entity == nullptr || index < 0 || index >= pathLength)
		return false;

	const PathPoint &point = points[(size_t)index];
	const double centerOffset = (double)JavaArithmetic::floatToInt(entity->width + 1.0f) * 0.5;
	x = (double)point.xCoord + centerOffset;
	y = (double)point.yCoord;
	z = (double)point.zCoord + centerOffset;
	return true;
}

bool PathEntity::getCurrentNodeCoordinates(Entity *entity, double &x, double &y, double &z) const
{
	return getVectorCoordinatesFromIndex(entity, pathIndex, x, y, z);
}

Vec3D *PathEntity::getVectorFromIndex(Entity *entity, int_t index)
{
	double x;
	double y;
	double z;
	if (!getVectorCoordinatesFromIndex(entity, index, x, y, z))
		return nullptr;
	return Vec3D::createVector(x, y, z);
}

Vec3D *PathEntity::getCurrentNodeVec3d(Entity *entity)
{
	return getVectorFromIndex(entity, pathIndex);
}

bool PathEntity::isSamePath(const PathEntity *other) const
{
	if (other == nullptr || other->pathLength != pathLength)
		return false;
	for (int_t i = 0; i < pathLength; ++i)
	{
		const PathPoint &left = points[(size_t)i];
		const PathPoint &right = other->points[(size_t)i];
		if (left.xCoord != right.xCoord || left.yCoord != right.yCoord || left.zCoord != right.zCoord)
			return false;
	}
	return true;
}

bool PathEntity::isDestinationSame(Vec3D *position) const
{
	PathPoint *finalPoint = const_cast<PathEntity *>(this)->getFinalPathPoint();
	return finalPoint != nullptr && position != nullptr &&
		finalPoint->xCoord == JavaArithmetic::doubleToInt(position->xCoord) &&
		finalPoint->zCoord == JavaArithmetic::doubleToInt(position->zCoord);
}

Vec3D *PathEntity::getPosition(Entity *entity)
{
	return getCurrentNodeVec3d(entity);
}
