#pragma once

#include <array>
#include "java/Type.h"
#include "Vec3D.h"
#include "MathHelper.h"
#include "IChunkProvider.h"

class World;
class WorldChunkManager;
class ChunkCoordinates;
class WorldType;

// net.minecraft.src.WorldProvider
class WorldProvider
{
public:
	WorldProvider();
	virtual ~WorldProvider();

	void registerWorld(World *world);
	virtual void registerWorldChunkManager();
	virtual IChunkProvider *getChunkProvider();
	virtual bool canCoordinateBeSpawn(int_t i, int_t j);
	virtual float calculateCelestialAngle(long_t l, float f);
	virtual float *calcSunriseSunsetColors(float f, float f1);
	virtual Vec3D *getFogColor(float celestialAngle, float partialTicks);
	virtual bool canRespawnHere();
	virtual int_t getMoonPhase(long_t worldTime, float partialTicks);
	virtual bool func_48217_e();
	virtual float getCloudHeight();
	virtual bool isSkyColored();
	virtual ChunkCoordinates *getEntrancePortalLocation();
	virtual int_t getAverageGroundLevel();
	virtual bool getWorldHasNoSky();
	virtual double getVoidFogYFactor();
	virtual bool hasVoidParticles();
	virtual bool func_48218_b(int_t x, int_t z);
	// func_28112_c — true if the sky/horizon should have the normal color blend applied
	virtual bool hasSkyColorBlend() { return true; }

	static WorldProvider *getProviderForDimension(int_t i);

protected:
	virtual void generateLightBrightnessTable();

public:
	World *worldObj = nullptr;
	WorldChunkManager *worldChunkMgr = nullptr;
	WorldType *terrainType = nullptr; // Static WorldType registry entry; not owned.
	bool isNether;
	bool isHellWorld;
	bool hasNoSky;
	float lightBrightnessTable[16];
	int_t worldType;

private:
	float colorsSunriseSunset[4];
};
