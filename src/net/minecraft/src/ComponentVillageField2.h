#pragma once

#include <memory>
#include "ComponentVillage.h"

class Random;
class StructureBoundingBox;

// net.minecraft.src.ComponentVillageField2
class ComponentVillageField2 : public ComponentVillage
{
public:
	ComponentVillageField2(int_t type, Random &random,
	                       std::unique_ptr<StructureBoundingBox> bounds, int_t direction);
	void buildComponent(StructureComponent *, std::vector<StructureComponent *> &, Random &) override;
	bool addComponentParts(World *world, Random &random,
	                       const StructureBoundingBox &chunkBounds) override;
	static ComponentVillageField2 *findValidPlacement(
		const std::vector<StructureComponent *> &components, Random &random,
		int_t x, int_t y, int_t z, int_t direction, int_t type);

private:
	int_t averageGroundLevel;
};
