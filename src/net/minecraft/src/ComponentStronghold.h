#pragma once

#include "EnumDoor.h"
#include "StructureComponent.h"

class ComponentStrongholdStairs2;
class Random;
class StructureBoundingBox;
class World;

// net.minecraft.src.ComponentStronghold
class ComponentStronghold : public StructureComponent
{
public:
	explicit ComponentStronghold(int_t componentType);

protected:
	void placeDoor(World *world, Random &random, const StructureBoundingBox &chunkBounds,
	               EnumDoor doorType, int_t x, int_t y, int_t z) const;
	EnumDoor getRandomDoor(Random &random) const;
	StructureComponent *getNextComponentNormal(ComponentStrongholdStairs2 *start,
	                                           std::vector<StructureComponent *> &components,
	                                           Random &random, int_t xOffset, int_t yOffset);
	StructureComponent *getNextComponentX(ComponentStrongholdStairs2 *start,
	                                      std::vector<StructureComponent *> &components,
	                                      Random &random, int_t yOffset, int_t zOffset);
	StructureComponent *getNextComponentZ(ComponentStrongholdStairs2 *start,
	                                      std::vector<StructureComponent *> &components,
	                                      Random &random, int_t yOffset, int_t xOffset);
	static bool canStrongholdGoDeeper(const StructureBoundingBox *bounds);
};
