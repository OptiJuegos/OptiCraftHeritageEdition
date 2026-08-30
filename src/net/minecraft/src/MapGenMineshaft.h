#pragma once

#include "MapGenStructure.h"

// net.minecraft.src.MapGenMineshaft
class MapGenMineshaft : public MapGenStructure
{
protected:
	bool canSpawnStructureAtCoords(int_t chunkX, int_t chunkZ) override;
	StructureStart *getStructureStart(int_t chunkX, int_t chunkZ) override;
};
