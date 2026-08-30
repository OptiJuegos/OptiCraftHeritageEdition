#pragma once

#include <vector>
#include "java/Type.h"
#include "PathPoint.h"

class Entity;
class Vec3D;

// net.minecraft.src.PathEntity
class PathEntity
{
public:
	PathEntity(const std::vector<PathPoint *> &apathpoint);

	void incrementPathIndex();
	bool isFinished();
	PathPoint *getFinalPathPoint();
	PathPoint *getPathPointFromIndex(int_t index);
	int_t getCurrentPathLength() const;
	void setCurrentPathLength(int_t length);
	int_t getCurrentPathIndex() const;
	void setCurrentPathIndex(int_t index);
	bool getVectorCoordinatesFromIndex(Entity *entity, int_t index, double &x, double &y, double &z) const;
	bool getCurrentNodeCoordinates(Entity *entity, double &x, double &y, double &z) const;
	Vec3D *getVectorFromIndex(Entity *entity, int_t index);
	Vec3D *getCurrentNodeVec3d(Entity *entity);
	Vec3D *getPosition(Entity *entity);
	bool isSamePath(const PathEntity *other) const;
	bool isDestinationSame(Vec3D *position) const;

private:
	std::vector<PathPoint> points;
	int_t pathIndex;
	int_t pathLength;
};
