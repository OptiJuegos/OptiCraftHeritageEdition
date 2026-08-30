#pragma once

#include "StructureStart.h"
#include "java/Type.h"

class Random;
class World;

// net.minecraft.src.StructureStrongholdStart
class StructureStrongholdStart : public StructureStart
{
public:
	StructureStrongholdStart(World *world, Random &random, int_t chunkX, int_t chunkZ);
};
