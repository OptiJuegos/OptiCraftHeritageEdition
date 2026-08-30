#pragma once

#include <vector>

#include "MapGenStructure.h"
#include "SpawnListEntry.h"

// net.minecraft.src.MapGenNetherBridge
class MapGenNetherBridge : public MapGenStructure
{
public:
	MapGenNetherBridge();

	std::vector<SpawnListEntry> *getSpawnList();

protected:
	bool canSpawnStructureAtCoords(int_t chunkX, int_t chunkZ) override;
	StructureStart *getStructureStart(int_t chunkX, int_t chunkZ) override;

private:
	std::vector<SpawnListEntry> spawnList;
};
