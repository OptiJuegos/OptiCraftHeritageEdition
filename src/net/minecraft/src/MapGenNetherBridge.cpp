#include "MapGenNetherBridge.h"

#include <typeinfo>

#include "EntityBlaze.h"
#include "EntityMagmaCube.h"
#include "EntityPigZombie.h"
#include "StructureNetherBridgeStart.h"
#include "World.h"
#include "java/Arithmetic.h"

MapGenNetherBridge::MapGenNetherBridge()
{
	spawnList.emplace_back(typeid(EntityBlaze), 10, 2, 3);
	spawnList.emplace_back(typeid(EntityPigZombie), 10, 4, 4);
	spawnList.emplace_back(typeid(EntityMagmaCube), 3, 4, 4);
}

std::vector<SpawnListEntry> *MapGenNetherBridge::getSpawnList()
{
	return &spawnList;
}

bool MapGenNetherBridge::canSpawnStructureAtCoords(int_t chunkX, int_t chunkZ)
{
	const int_t regionX = JavaArithmetic::intShr(chunkX, 4);
	const int_t regionZ = JavaArithmetic::intShr(chunkZ, 4);
	const int_t mixed = regionX ^ JavaArithmetic::intShl(regionZ, 4);
	const long_t seed = JavaArithmetic::longFromBits(static_cast<ulong_t>(static_cast<long_t>(mixed)) ^
		static_cast<ulong_t>(worldObj->getRandomSeed()));
	rand.setSeed(seed);
	rand.nextInt();
	if (rand.nextInt(3) != 0)
		return false;

	const int_t expectedX = JavaArithmetic::intAdd(JavaArithmetic::intShl(regionX, 4), 4 + rand.nextInt(8));
	if (chunkX != expectedX)
		return false;
	const int_t expectedZ = JavaArithmetic::intAdd(JavaArithmetic::intShl(regionZ, 4), 4 + rand.nextInt(8));
	return chunkZ == expectedZ;
}

StructureStart *MapGenNetherBridge::getStructureStart(int_t chunkX, int_t chunkZ)
{
	return new StructureNetherBridgeStart(worldObj, rand, chunkX, chunkZ);
}
