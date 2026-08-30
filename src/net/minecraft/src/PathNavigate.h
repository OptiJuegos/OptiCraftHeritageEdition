#pragma once

#include <memory>
#include "java/Type.h"
#include "platform/PlatformTuning.h"

class EntityLiving;
class PathEntity;
class World;
class Vec3D;

// net.minecraft.src.PathNavigate
class PathNavigate
{
public:
	PathNavigate(EntityLiving *entity, World *world, float pathSearchRange);
	~PathNavigate();

	void func_48664_a(bool value);
	void setAvoidsWater(bool value);
	bool func_48658_a() const;
	bool getAvoidsWater() const;
	void setBreakDoors(bool value);
	void func_48663_c(bool value);
	void setEnterDoors(bool value);
	bool func_48665_b() const;
	bool getCanBreakDoors() const;
	void func_48680_d(bool value);
	void setAvoidSun(bool value);
	void setSpeed(float value);
	void func_48669_e(bool value);
	void setCanSwim(bool value);

	PathEntity *getPathToXYZ(double x, double y, double z);
	bool func_48666_a(double x, double y, double z, float speed);
	bool tryMoveToXYZ(double x, double y, double z, float speed);
	PathEntity *func_48679_a(EntityLiving *target);
	PathEntity *getPathToEntityLiving(EntityLiving *target);
	bool func_48667_a(EntityLiving *target, float speed);
	bool tryMoveToEntityLiving(EntityLiving *target, float speed);
	bool setPath(PathEntity *path, float speed);
	PathEntity *getPath() const;
	void onUpdateNavigation();
	bool noPath() const;
	void clearPathEntity();
	Vec3D *getEntityPosition() const;

private:
#if PLATFORM_FLOAT_ENTITY_AI_MATH
	using path_math_t = float;
#else
	using path_math_t = double;
#endif

	void pathFollow();
	int_t getPathableYPos() const;
	bool canNavigate() const;
	bool func_48657_k() const;
	bool isInLiquid() const;
	void removeSunnyPath();
	bool isDirectPathBetweenPoints(double startX, double startY, double startZ,
		double endX, double endY, double endZ, int_t sizeX, int_t sizeY, int_t sizeZ) const;
	bool isSafeToStandAt(int_t x, int_t y, int_t z, int_t sizeX, int_t sizeY, int_t sizeZ,
		int_t startBlockX, int_t startBlockZ, path_math_t startFractionX, path_math_t startFractionZ,
		path_math_t dirX, path_math_t dirZ) const;
	bool isPositionClear(int_t x, int_t y, int_t z, int_t sizeX, int_t sizeY, int_t sizeZ,
		int_t startBlockX, int_t startBlockZ, path_math_t startFractionX, path_math_t startFractionZ,
		path_math_t dirX, path_math_t dirZ) const;

	EntityLiving *theEntity;
	World *worldObj;
	std::unique_ptr<PathEntity> currentPath;
	float speed;
	float pathSearchRange;
	bool noSunPathfind;
	int_t totalTicks;
	int_t ticksAtLastPos;
	path_math_t lastPosX;
	path_math_t lastPosY;
	path_math_t lastPosZ;
	bool canPassOpenWoodenDoors;
	bool canPassClosedWoodenDoors;
	bool avoidsWater;
	bool canSwim;
};
