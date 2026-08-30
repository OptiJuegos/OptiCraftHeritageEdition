#pragma once

#include <memory>
#include <vector>

#include "StructureComponent.h"

class Random;
class StructureBoundingBox;

// net.minecraft.src.ComponentMineshaftRoom
class ComponentMineshaftRoom : public StructureComponent
{
public:
	ComponentMineshaftRoom(int_t componentType, Random &random, int_t x, int_t z);

	void buildComponent(StructureComponent *parent,
	                    std::vector<StructureComponent *> &components,
	                    Random &random) override;
	bool addComponentParts(World *world, Random &random,
	                       const StructureBoundingBox &chunkBounds) override;

private:
	std::vector<std::unique_ptr<StructureBoundingBox>> childStructures;
};
