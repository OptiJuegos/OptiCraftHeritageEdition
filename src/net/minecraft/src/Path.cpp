#include "Path.h"

#include <limits>
#include <stdexcept>

#include "PathPoint.h"
#include "java/Arithmetic.h"

Path::Path()
	: pathPoints(1024, nullptr), count(0)
{
}

PathPoint *Path::addPoint(PathPoint *pathpoint)
{
	if (pathpoint->index >= 0)
	{
		throw std::runtime_error("OW KNOWS!");
	}
	if (static_cast<std::size_t>(count) == pathPoints.size())
	{
		const int_t newSize = JavaArithmetic::intShl(count, 1);
		if (newSize <= count || newSize <= 0)
			throw std::length_error("Path heap capacity overflow");
		pathPoints.resize(static_cast<std::size_t>(newSize), nullptr);
	}
	pathPoints[count] = pathpoint;
	pathpoint->index = count;
	sortBack(count++);
	return pathpoint;
}

void Path::clearPath()
{
	count = 0;
}

PathPoint *Path::dequeue()
{
	if (count <= 0)
		throw std::runtime_error("Path::dequeue on empty path");

	PathPoint *pathpoint = pathPoints[0];
	pathPoints[0] = pathPoints[--count];
	pathPoints[count] = nullptr;
	if (count > 0)
	{
		sortForward(0);
	}
	pathpoint->index = -1;
	return pathpoint;
}

void Path::changeDistance(PathPoint *pathpoint, float f)
{
	float f1 = pathpoint->distanceToTarget;
	pathpoint->distanceToTarget = f;
	if (f < f1)
	{
		sortBack(pathpoint->index);
	}
	else
	{
		sortForward(pathpoint->index);
	}
}

void Path::sortBack(int_t i)
{
	PathPoint *pathpoint = pathPoints[i];
	float f = pathpoint->distanceToTarget;
	while (true)
	{
		if (i <= 0)
		{
			break;
		}
		int_t j = (i - 1) >> 1;
		PathPoint *pathpoint1 = pathPoints[j];
		if (f >= pathpoint1->distanceToTarget)
		{
			break;
		}
		pathPoints[i] = pathpoint1;
		pathpoint1->index = i;
		i = j;
	}
	pathPoints[i] = pathpoint;
	pathpoint->index = i;
}

void Path::sortForward(int_t i)
{
	PathPoint *pathpoint = pathPoints[i];
	float f = pathpoint->distanceToTarget;
	while (true)
	{
		int_t j = JavaArithmetic::intAdd(1, JavaArithmetic::intShl(i, 1));
		int_t k = JavaArithmetic::intAdd(j, 1);
		if (j >= count)
		{
			break;
		}
		PathPoint *pathpoint1 = pathPoints[j];
		float f1 = pathpoint1->distanceToTarget;
		PathPoint *pathpoint2;
		float f2;
		if (k >= count)
		{
			pathpoint2 = nullptr;
			f2 = std::numeric_limits<float>::infinity();
		}
		else
		{
			pathpoint2 = pathPoints[k];
			f2 = pathpoint2->distanceToTarget;
		}
		if (f1 < f2)
		{
			if (f1 >= f)
			{
				break;
			}
			pathPoints[i] = pathpoint1;
			pathpoint1->index = i;
			i = j;
			continue;
		}
		if (f2 >= f)
		{
			break;
		}
		pathPoints[i] = pathpoint2;
		pathpoint2->index = i;
		i = k;
	}
	pathPoints[i] = pathpoint;
	pathpoint->index = i;
}

bool Path::isPathEmpty()
{
	return count == 0;
}
