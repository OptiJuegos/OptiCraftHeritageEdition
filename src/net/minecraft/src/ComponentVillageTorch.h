#pragma once

#include <memory>

#include "ComponentVillage.h"

class Random;
class StructureBoundingBox;

// net.minecraft.src.ComponentVillageTorch
class ComponentVillageTorch : public ComponentVillage
{
public:
	ComponentVillageTorch(int_t type, Random &random,
	                      std::unique_ptr<StructureBoundingBox> bounds, int_t direction);
	void buildComponent(StructureComponent *, std::vector<StructureComponent *> &, Random &) override;
	bool addComponentParts(World *world, Random &random,
	                       const StructureBoundingBox &chunkBounds) override;

	static std::unique_ptr<StructureBoundingBox> findValidPlacement(
		const std::vector<StructureComponent *> &components, Random &random,
		int_t x, int_t y, int_t z, int_t direction);

private:
	int_t averageGroundLevel;
};
