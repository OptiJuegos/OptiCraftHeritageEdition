#include "MapGenStructure.h"

#include <cmath>
#include <limits>

#include "ChunkCoordIntPair.h"
#include "ChunkPosition.h"
#include "StructureBoundingBox.h"
#include "StructureComponent.h"
#include "StructureStart.h"
#include "World.h"
#include "java/Arithmetic.h"

namespace
{
	double getJavaStructureDistance(const ChunkPosition &position, int_t x, int_t y, int_t z)
	{
		const int_t dx = JavaArithmetic::intSub(position.x, x);
		const int_t dy = JavaArithmetic::intSub(position.y, y);
		const int_t dz = JavaArithmetic::intSub(position.z, z);
		const int_t mixed = JavaArithmetic::intMul(JavaArithmetic::intMul(dx, dy), dy);
		const int_t horizontal = JavaArithmetic::intMul(dz, dz);
		return static_cast<double>(JavaArithmetic::intAdd(JavaArithmetic::intAdd(dx, mixed), horizontal));
	}
}

MapGenStructure::MapGenStructure()
{
	// Structures use the vanilla 8-chunk source sweep even when cave generation
	// uses a reduced platform radius.
	sourceRange = 8;
}

MapGenStructure::~MapGenStructure()
{
	for (auto &entry : coordMap)
		delete entry.second;
}

void MapGenStructure::generateChunk(World *world, int_t sourceChunkX, int_t sourceChunkZ,
                                    int_t, int_t, byte_t *)
{
	const ulong_t key = ChunkCoordIntPair::chunkXZ2Long(sourceChunkX, sourceChunkZ);
	if (coordMap.find(key) != coordMap.end())
		return;

	rand.nextInt();
	if (canSpawnStructureAtCoords(sourceChunkX, sourceChunkZ))
	{
		coordMap.emplace(key, getStructureStart(sourceChunkX, sourceChunkZ));
		coordOrder.add(key);
	}
}

bool MapGenStructure::generateStructuresInChunk(World *world, Random &random,
                                                int_t chunkX, int_t chunkZ)
{
	const int_t minX = JavaArithmetic::intAdd(JavaArithmetic::intShl(chunkX, 4), 8);
	const int_t minZ = JavaArithmetic::intAdd(JavaArithmetic::intShl(chunkZ, 4), 8);
	const int_t maxX = JavaArithmetic::intAdd(minX, 15);
	const int_t maxZ = JavaArithmetic::intAdd(minZ, 15);
	const StructureBoundingBox chunkBounds(minX, minZ, maxX, maxZ);
	bool generated = false;

	// generateStructure() places blocks and measures ground level, which loads
	// neighbouring chunks; a chunk that is not resident yet is generated on the spot,
	// and ChunkProviderGenerate::provideChunk runs this generator's own source sweep,
	// re-entering generateChunk() and inserting into coordMap. A rehash then
	// invalidates the iterator held here. Java only risked a
	// ConcurrentModificationException; in C++ it is a use-after-free, so iterate over a
	// snapshot. Entries are only ever added to coordMap, never erased before teardown,
	// so a snapshotted pointer stays valid for the whole loop.
	const std::vector<ulong_t> structureKeys = coordOrder.valuesInIterationOrder();
	for (ulong_t key : structureKeys)
	{
		auto it = coordMap.find(key);
		StructureStart *start = it != coordMap.end() ? it->second : nullptr;
		if (start != nullptr && start->isSizeableStructure() && start->getBoundingBox() != nullptr &&
		    start->getBoundingBox()->intersectsWith(minX, minZ, maxX, maxZ))
		{
			start->generateStructure(world, random, chunkBounds);
			generated = true;
		}
	}
	return generated;
}

bool MapGenStructure::isInsideStructure(int_t x, int_t y, int_t z) const
{
	const std::vector<ulong_t> structureKeys = coordOrder.valuesInIterationOrder();
	for (ulong_t key : structureKeys)
	{
		auto it = coordMap.find(key);
		StructureStart *start = it != coordMap.end() ? it->second : nullptr;
		if (start == nullptr || !start->isSizeableStructure() || start->getBoundingBox() == nullptr ||
		    !start->getBoundingBox()->intersectsWith(x, z, x, z))
			continue;

		for (StructureComponent *component : start->getComponents())
		{
			if (component != nullptr && component->getBoundingBox() != nullptr &&
			    component->getBoundingBox()->isVecInside(x, y, z))
				return true;
		}
	}
	return false;
}

ChunkPosition *MapGenStructure::getNearestInstance(World *world, int_t x, int_t y, int_t z)
{
	worldObj = world;
	rand.setSeed(world->getRandomSeed());
	const long_t xMultiplier = rand.nextLong();
	const long_t zMultiplier = rand.nextLong();
	const long_t xSeed = JavaArithmetic::longMul(static_cast<long_t>(JavaArithmetic::intShr(x, 4)), xMultiplier);
	const long_t zSeed = JavaArithmetic::longMul(static_cast<long_t>(JavaArithmetic::intShr(z, 4)), zMultiplier);
	const ulong_t seedBits = static_cast<ulong_t>(xSeed) ^ static_cast<ulong_t>(zSeed) ^
	                         static_cast<ulong_t>(world->getRandomSeed());
	rand.setSeed(JavaArithmetic::longFromBits(seedBits));
	generateChunk(world, JavaArithmetic::intShr(x, 4), JavaArithmetic::intShr(z, 4), 0, 0, nullptr);

	double bestDistance = std::numeric_limits<double>::max();
	ChunkPosition *best = nullptr;
	const std::vector<ulong_t> structureKeys = coordOrder.valuesInIterationOrder();
	for (ulong_t key : structureKeys)
	{
		auto it = coordMap.find(key);
		StructureStart *start = it != coordMap.end() ? it->second : nullptr;
		if (start == nullptr || !start->isSizeableStructure() || start->getComponents().empty())
			continue;

		ChunkPosition *position = start->getComponents().front()->getCenter();
		if (position == nullptr)
			continue;
		const double distance = getJavaStructureDistance(*position, x, y, z);
		if (distance < bestDistance)
		{
			delete best;
			bestDistance = distance;
			best = position;
		}
		else
		{
			delete position;
		}
	}

	if (best != nullptr)
		return best;

	std::vector<ChunkPosition *> coordinates = getStructureCoordinates();
	for (ChunkPosition *position : coordinates)
	{
		if (position == nullptr)
			continue;
		const double distance = getJavaStructureDistance(*position, x, y, z);
		if (distance < bestDistance)
		{
			delete best;
			bestDistance = distance;
			best = new ChunkPosition(*position);
		}
		delete position;
	}
	return best;
}

std::vector<ChunkPosition *> MapGenStructure::getStructureCoordinates()
{
	return {};
}
