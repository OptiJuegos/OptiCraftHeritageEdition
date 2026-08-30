#pragma once

#include "WorldProvider.h"

// net.minecraft.src.WorldProviderEnd
class WorldProviderEnd : public WorldProvider
{
public:
	void registerWorldChunkManager() override;
	IChunkProvider *getChunkProvider() override;
	float calculateCelestialAngle(long_t worldTime, float partialTicks) override;
	float *calcSunriseSunsetColors(float celestialAngle, float partialTicks) override;
	Vec3D *getFogColor(float celestialAngle, float partialTicks) override;
	bool isSkyColored() override;
	bool hasSkyColorBlend() override { return false; }
	bool canRespawnHere() override;
	bool func_48217_e() override;
	float getCloudHeight() override;
	bool canCoordinateBeSpawn(int_t x, int_t z) override;
	ChunkCoordinates *getEntrancePortalLocation() override;
	int_t getAverageGroundLevel() override;
	bool func_48218_b(int_t x, int_t z) override;
};
