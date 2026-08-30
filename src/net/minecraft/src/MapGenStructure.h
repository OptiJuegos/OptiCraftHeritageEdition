#pragma once

#include <unordered_map>
#include <vector>

#include "MapGenBase.h"
#include "java/Type.h"
#include "java/HashSet.h"

class ChunkPosition;
class StructureStart;
class World;

// net.minecraft.src.MapGenStructure
class MapGenStructure : public MapGenBase
{
public:
	MapGenStructure();
	~MapGenStructure() override;

	bool generateStructuresInChunk(World *world, Random &random, int_t chunkX, int_t chunkZ);
	bool isInsideStructure(int_t x, int_t y, int_t z) const;
	ChunkPosition *getNearestInstance(World *world, int_t x, int_t y, int_t z);

protected:
	void generateChunk(World *world, int_t sourceChunkX, int_t sourceChunkZ,
	                   int_t targetChunkX, int_t targetChunkZ, byte_t blocks[]) override;
	virtual std::vector<ChunkPosition *> getStructureCoordinates();
	virtual bool canSpawnStructureAtCoords(int_t chunkX, int_t chunkZ) = 0;
	virtual StructureStart *getStructureStart(int_t chunkX, int_t chunkZ) = 0;

	struct StructureKeyHash
	{
		std::uint32_t operator()(ulong_t value) const
		{
			// java.lang.Long.hashCode(): (int)(value ^ (value >>> 32)).
			return static_cast<std::uint32_t>(value ^ (value >> 32));
		}
	};

	struct StructureKeyEqual
	{
		bool operator()(ulong_t lhs, ulong_t rhs) const { return lhs == rhs; }
	};

	std::unordered_map<ulong_t, StructureStart *> coordMap;
	JavaHashSet<ulong_t, StructureKeyHash, StructureKeyEqual> coordOrder;
};
