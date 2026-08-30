#pragma once

#include <memory>
#include "ComponentVillage.h"

class Random;
class StructureBoundingBox;

// net.minecraft.src.ComponentVillageHall
class ComponentVillageHall : public ComponentVillage
{
public:
	ComponentVillageHall(int_t type, Random &random,
	                     std::unique_ptr<StructureBoundingBox> bounds, int_t direction);
	void buildComponent(StructureComponent *, std::vector<StructureComponent *> &, Random &) override;
	bool addComponentParts(World *world, Random &random,
	                       const StructureBoundingBox &chunkBounds) override;
	static ComponentVillageHall *findValidPlacement(
		const std::vector<StructureComponent *> &components, Random &random,
		int_t x, int_t y, int_t z, int_t direction, int_t type);

protected:
	int_t getVillagerType(int_t index) const override;

private:
	int_t averageGroundLevel;
};
