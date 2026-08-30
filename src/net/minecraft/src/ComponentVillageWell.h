#pragma once

#include "ComponentVillage.h"

class Random;
class StructureBoundingBox;

// net.minecraft.src.ComponentVillageWell
class ComponentVillageWell : public ComponentVillage
{
public:
	ComponentVillageWell(int_t type, Random &random, int_t x, int_t z);
	void buildComponent(StructureComponent *parent, std::vector<StructureComponent *> &components,
	                    Random &random) override;
	bool addComponentParts(World *world, Random &random,
	                       const StructureBoundingBox &chunkBounds) override;

private:
	const bool field_35104_a;
	int_t averageGroundLevel;
};
