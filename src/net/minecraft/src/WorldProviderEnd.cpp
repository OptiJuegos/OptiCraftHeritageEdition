#include "WorldProviderEnd.h"

#include "BiomeGenBase.h"
#include "Block.h"
#include "ChunkCoordinates.h"
#include "ChunkProviderEnd.h"
#include "World.h"
#include "WorldChunkManagerHell.h"

void WorldProviderEnd::registerWorldChunkManager()
{
	worldChunkMgr = new WorldChunkManagerHell(BiomeGenBase::sky, 0.5f, 0.0f);
	worldType = 1;
	hasNoSky = true;
}

IChunkProvider *WorldProviderEnd::getChunkProvider()
{
	return new ChunkProviderEnd(worldObj, worldObj->getRandomSeed());
}

float WorldProviderEnd::calculateCelestialAngle(long_t, float)
{
	return 0.0f;
}

float *WorldProviderEnd::calcSunriseSunsetColors(float, float)
{
	return nullptr;
}

Vec3D *WorldProviderEnd::getFogColor(float celestialAngle, float)
{
	const int_t color = 8421536;
	float brightness = MathHelper::cos(celestialAngle * 3.14159265358979323846f * 2.0f) * 2.0f + 0.5f;
	if (brightness < 0.0f)
		brightness = 0.0f;
	if (brightness > 1.0f)
		brightness = 1.0f;

	float red = (float)(color >> 16 & 255) / 255.0f;
	float green = (float)(color >> 8 & 255) / 255.0f;
	float blue = (float)(color & 255) / 255.0f;
	red *= brightness * 0.0f + 0.15f;
	green *= brightness * 0.0f + 0.15f;
	blue *= brightness * 0.0f + 0.15f;
	return Vec3D::createVector(red, green, blue);
}

bool WorldProviderEnd::isSkyColored()
{
	return false;
}

bool WorldProviderEnd::canRespawnHere()
{
	return false;
}

bool WorldProviderEnd::func_48217_e()
{
	return false;
}

float WorldProviderEnd::getCloudHeight()
{
	return 8.0f;
}

bool WorldProviderEnd::canCoordinateBeSpawn(int_t x, int_t z)
{
	const int_t blockId = worldObj->getFirstUncoveredBlock(x, z);
	return blockId != 0 && blockId < Block::BLOCK_REGISTRY_SIZE &&
	       Block::blocksList[blockId] != nullptr &&
	       Block::blocksList[blockId]->blockMaterial->blocksMovement();
}

ChunkCoordinates *WorldProviderEnd::getEntrancePortalLocation()
{
	return new ChunkCoordinates(100, 50, 0);
}

int_t WorldProviderEnd::getAverageGroundLevel()
{
	return 50;
}

bool WorldProviderEnd::func_48218_b(int_t, int_t)
{
	return true;
}
