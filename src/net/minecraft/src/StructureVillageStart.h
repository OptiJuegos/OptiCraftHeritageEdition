#pragma once

#include "StructureStart.h"

class Random;
class World;

// net.minecraft.src.StructureVillageStart
class StructureVillageStart : public StructureStart
{
public:
	StructureVillageStart(World *world, Random &random,
	                      int_t chunkX, int_t chunkZ, int_t terrainType);
	bool isSizeableStructure() const override;

private:
	bool hasMoreThanTwoComponents;
};
