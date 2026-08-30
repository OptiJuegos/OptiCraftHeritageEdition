#pragma once

#include <vector>
#include "java/Type.h"

class PathPoint;

// net.minecraft.src.Path
// Binary heap priority queue keyed by PathPoint::distanceToTarget.
class Path
{
public:
	Path();

	PathPoint *addPoint(PathPoint *pathpoint);
	void clearPath();
	PathPoint *dequeue();
	void changeDistance(PathPoint *pathpoint, float f);
	bool isPathEmpty();

private:
	void sortBack(int_t i);
	void sortForward(int_t i);

	std::vector<PathPoint *> pathPoints;
	int_t count;
};
