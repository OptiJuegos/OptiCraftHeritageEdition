#pragma once

#include "StructureStart.h"

class Random;
class World;

// net.minecraft.src.StructureMineshaftStart
class StructureMineshaftStart : public StructureStart
{
public:
	StructureMineshaftStart(World *world, Random &random, int_t chunkX, int_t chunkZ);
};
