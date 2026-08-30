#include "MapGenStronghold.h"

#include <cmath>

#include "BiomeGenBase.h"
#include "ChunkCoordIntPair.h"
#include "ChunkPosition.h"
#include "ComponentStrongholdStairs2.h"
#include "StructureStart.h"
#include "StructureStrongholdStart.h"
#include "World.h"
#include "WorldChunkManager.h"
#include "java/Arithmetic.h"
#include "java/Math.h"
#include "java/Random.h"

namespace
{
int_t javaRoundToInt(double value)
{
	return JavaArithmetic::longToInt(JavaArithmetic::roundDouble(value));
}
} // namespace

MapGenStronghold::MapGenStronghold()
	: ranBiomeCheck(false), structureCoords{nullptr, nullptr, nullptr}
{
	allowedBiomeGenBases = {
		BiomeGenBase::desert,
		BiomeGenBase::forest,
		BiomeGenBase::extremeHills,
		BiomeGenBase::swampland,
		BiomeGenBase::taiga,
		BiomeGenBase::icePlains,
		BiomeGenBase::iceMountains,
		BiomeGenBase::desertHills,
		BiomeGenBase::forestHills,
		BiomeGenBase::extremeHillsEdge,
		BiomeGenBase::jungle,
		BiomeGenBase::jungleHills
	};
}

MapGenStronghold::~MapGenStronghold()
{
	for (ChunkCoordIntPair *coord : structureCoords)
		delete coord;
}

void MapGenStronghold::ensureStructureCoords()
{
	if (ranBiomeCheck || worldObj == nullptr)
		return;

	Random strongholdRandom;
	strongholdRandom.setSeed(worldObj->getRandomSeed());
	double angle = strongholdRandom.nextDouble() * 3.14159265358979323846 * 2.0;

	for (std::size_t index = 0; index < structureCoords.size(); ++index)
	{
		const double distance = (1.25 + strongholdRandom.nextDouble()) * 32.0;
		int_t chunkX = javaRoundToInt(JavaMath::cos(angle) * distance);
		int_t chunkZ = javaRoundToInt(JavaMath::sin(angle) * distance);

		ChunkPosition *position = worldObj->getWorldChunkManager()->findBiomePosition(
			JavaArithmetic::intAdd(JavaArithmetic::intMul(chunkX, 16), 8),
			JavaArithmetic::intAdd(JavaArithmetic::intMul(chunkZ, 16), 8), 112,
			allowedBiomeGenBases, strongholdRandom);
		if (position != nullptr)
		{
			chunkX = JavaArithmetic::intShr(position->x, 4);
			chunkZ = JavaArithmetic::intShr(position->z, 4);
			delete position;
		}

		delete structureCoords[index];
		structureCoords[index] = new ChunkCoordIntPair(chunkX, chunkZ);
		angle += 3.14159265358979323846 * 2.0 / static_cast<double>(structureCoords.size());
	}

	ranBiomeCheck = true;
}

bool MapGenStronghold::canSpawnStructureAtCoords(int_t chunkX, int_t chunkZ)
{
	ensureStructureCoords();
	for (ChunkCoordIntPair *coord : structureCoords)
	{
		if (coord != nullptr && coord->chunkXPos == chunkX && coord->chunkZPos == chunkZ)
			return true;
	}
	return false;
}

std::vector<ChunkPosition *> MapGenStronghold::getStructureCoordinates()
{
	ensureStructureCoords();
	std::vector<ChunkPosition *> result;
	result.reserve(structureCoords.size());
	for (ChunkCoordIntPair *coord : structureCoords)
	{
		if (coord != nullptr)
			result.push_back(new ChunkPosition(JavaArithmetic::intAdd(JavaArithmetic::intShl(coord->chunkXPos, 4), 8), 64, JavaArithmetic::intAdd(JavaArithmetic::intShl(coord->chunkZPos, 4), 8)));
	}
	return result;
}

StructureStart *MapGenStronghold::getStructureStart(int_t chunkX, int_t chunkZ)
{
	for (;;)
	{
		StructureStrongholdStart *start = new StructureStrongholdStart(worldObj, rand, chunkX, chunkZ);
		const std::vector<StructureComponent *> &components = start->getComponents();
		if (!components.empty())
		{
			ComponentStrongholdStairs2 *stairs = static_cast<ComponentStrongholdStairs2 *>(components.front());
			if (stairs != nullptr && stairs->portalRoom != nullptr)
				return start;
		}
		delete start;
	}
}
