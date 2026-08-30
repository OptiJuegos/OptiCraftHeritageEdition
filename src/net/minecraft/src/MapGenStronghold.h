#pragma once

#include <array>
#include <vector>

#include "MapGenStructure.h"

class BiomeGenBase;
class ChunkCoordIntPair;
class ChunkPosition;
class StructureStart;

// net.minecraft.src.MapGenStronghold
class MapGenStronghold : public MapGenStructure
{
public:
	MapGenStronghold();
	~MapGenStronghold() override;

protected:
	bool canSpawnStructureAtCoords(int_t chunkX, int_t chunkZ) override;
	StructureStart *getStructureStart(int_t chunkX, int_t chunkZ) override;
	std::vector<ChunkPosition *> getStructureCoordinates() override;

private:
	void ensureStructureCoords();

	std::vector<BiomeGenBase *> allowedBiomeGenBases;
	bool ranBiomeCheck;
	std::array<ChunkCoordIntPair *, 3> structureCoords;
};
