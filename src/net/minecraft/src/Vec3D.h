#pragma once

#include "java/Type.h"
#include "java/String.h"

// net.minecraft.src.Vec3D
class Vec3D
{
public:
	static Vec3D *createVectorHelper(double d, double d1, double d2);
	static void initialize();                                       // reset per-frame pool index
	static void trimVectorPool(); // release temporary overflow heap memory
	static void resetVectorPool() { initialize(); } // reset pool
	static Vec3D *createVector(double d, double d1, double d2);

private:
	Vec3D() = default;              // pool array construction
	Vec3D(double d, double d1, double d2);
	Vec3D *setComponents(double d, double d1, double d2);

public:
	Vec3D *subtract(Vec3D *vec3d);
	Vec3D *normalize();
	Vec3D *crossProduct(Vec3D *vec3d);
	double dotProduct(Vec3D *vec3d);
	Vec3D *addVector(double d, double d1, double d2);
	double distanceTo(Vec3D *vec3d);
	double squareDistanceTo(Vec3D *vec3d);
	double squareDistanceTo(double d, double d1, double d2);
	double lengthVector();
	Vec3D *getIntermediateWithXValue(Vec3D *vec3d, double d);
	Vec3D *getIntermediateWithYValue(Vec3D *vec3d, double d);
	Vec3D *getIntermediateWithZValue(Vec3D *vec3d, double d);
	jstring toString();
	void rotateAroundX(float f);
	void rotateAroundY(float f);

private:
	static constexpr int_t POOL_SIZE = 1024;
	static Vec3D pool[POOL_SIZE];
	static int_t nextVector;

public:
	double xCoord;
	double yCoord;
	double zCoord;
};
