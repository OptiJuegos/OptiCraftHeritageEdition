#include "MapGenVillage.h"

#include "BiomeGenBase.h"
#include "StructureVillageStart.h"
#include "World.h"
#include "WorldChunkManager.h"
#include "java/Random.h"
#include "java/Arithmetic.h"

std::vector<BiomeGenBase *> MapGenVillage::villageSpawnBiomes;

MapGenVillage::MapGenVillage(int_t terrainTypeValue)
	: terrainType(terrainTypeValue)
{
	if (villageSpawnBiomes.empty())
	{
		villageSpawnBiomes.push_back(BiomeGenBase::plains);
		villageSpawnBiomes.push_back(BiomeGenBase::desert);
	}
}

bool MapGenVillage::canSpawnStructureAtCoords(int_t chunkX, int_t chunkZ)
{
	static constexpr int_t spacing = 32;
	static constexpr int_t separation = 8;
	const int_t originalX = chunkX;
	const int_t originalZ = chunkZ;
	if (chunkX < 0)
		chunkX = JavaArithmetic::intSub(chunkX, spacing - 1);
	if (chunkZ < 0)
		chunkZ = JavaArithmetic::intSub(chunkZ, spacing - 1);

	int_t regionX = chunkX / spacing;
	int_t regionZ = chunkZ / spacing;
	Random &random = worldObj->setRandomSeed(regionX, regionZ, 10387312);
	regionX = JavaArithmetic::intMul(regionX, spacing);
	regionZ = JavaArithmetic::intMul(regionZ, spacing);
	regionX = JavaArithmetic::intAdd(regionX, random.nextInt(spacing - separation));
	regionZ = JavaArithmetic::intAdd(regionZ, random.nextInt(spacing - separation));
	if (originalX != regionX || originalZ != regionZ)
		return false;

	return worldObj->getWorldChunkManager()->areBiomesViable(
		JavaArithmetic::intAdd(JavaArithmetic::intMul(originalX, 16), 8),
		JavaArithmetic::intAdd(JavaArithmetic::intMul(originalZ, 16), 8), 0, villageSpawnBiomes);
}

StructureStart *MapGenVillage::getStructureStart(int_t chunkX, int_t chunkZ)
{
	return new StructureVillageStart(worldObj, rand, chunkX, chunkZ, terrainType);
}
