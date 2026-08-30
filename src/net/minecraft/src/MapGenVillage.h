#pragma once

#include <vector>

#include "MapGenStructure.h"

class BiomeGenBase;

// net.minecraft.src.MapGenVillage
class MapGenVillage : public MapGenStructure
{
public:
	explicit MapGenVillage(int_t terrainType);

	static std::vector<BiomeGenBase *> villageSpawnBiomes;

protected:
	bool canSpawnStructureAtCoords(int_t chunkX, int_t chunkZ) override;
	StructureStart *getStructureStart(int_t chunkX, int_t chunkZ) override;

private:
	const int_t terrainType;
};
