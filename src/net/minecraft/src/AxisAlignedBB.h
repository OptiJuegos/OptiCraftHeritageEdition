#pragma once

#include "java/Type.h"
#include "java/String.h"

class Vec3D;
class MovingObjectPosition;

// net.minecraft.src.AxisAlignedBB
class AxisAlignedBB
{
public:
	static AxisAlignedBB *getBoundingBox(double d, double d1, double d2, double d3, double d4, double d5);
	static void clearBoundingBoxPool();
	static void trimBoundingBoxPool(); // release temporary overflow heap memory
	static void resetBoundingBoxPool() { clearBoundingBoxPool(); } // reset pool
	static AxisAlignedBB *getBoundingBoxFromPool(double d, double d1, double d2, double d3, double d4, double d5);

public:
	// Public so entities can own their bounding box inline (by value) instead of
	// allocating one on the heap per entity. Zero-initialised to match the old
	// getBoundingBox(0,0,0,0,0,0) behaviour.
	AxisAlignedBB() : minX(0.0), minY(0.0), minZ(0.0), maxX(0.0), maxY(0.0), maxZ(0.0) {}

private:
	AxisAlignedBB(double d, double d1, double d2, double d3, double d4, double d5);

public:
	AxisAlignedBB *setBounds(double d, double d1, double d2, double d3, double d4, double d5);
	AxisAlignedBB *addCoord(double d, double d1, double d2);
	AxisAlignedBB *expand(double d, double d1, double d2);
	AxisAlignedBB *contract(double d, double d1, double d2); // Java func_28195_e
	AxisAlignedBB *getOffsetBoundingBox(double d, double d1, double d2);
	double calculateXOffset(AxisAlignedBB *axisalignedbb, double d);
	double calculateYOffset(AxisAlignedBB *axisalignedbb, double d);
	double calculateZOffset(AxisAlignedBB *axisalignedbb, double d);
	bool intersectsWith(AxisAlignedBB *axisalignedbb);
	AxisAlignedBB *offset(double d, double d1, double d2);
	bool isVecInside(Vec3D *vec3d);
	double getAverageEdgeLength();
	AxisAlignedBB *copy();
	MovingObjectPosition *calculateIntercept(Vec3D *vec3d, Vec3D *vec3d1);

private:
	bool isVecInYZ(Vec3D *vec3d);
	bool isVecInXZ(Vec3D *vec3d);
	bool isVecInXY(Vec3D *vec3d);

public:
	void setBB(AxisAlignedBB *axisalignedbb);
	jstring toString();

private:
	static constexpr int_t POOL_SIZE = 1024;
	static AxisAlignedBB pool[POOL_SIZE];
	static int_t numBoundingBoxesInUse;

public:
	double minX;
	double minY;
	double minZ;
	double maxX;
	double maxY;
	double maxZ;
};
