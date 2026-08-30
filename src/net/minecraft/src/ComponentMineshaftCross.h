#pragma once

#include <memory>
#include <vector>

#include "StructureComponent.h"

class Random;
class StructureBoundingBox;

// net.minecraft.src.ComponentMineshaftCross
class ComponentMineshaftCross : public StructureComponent
{
public:
	ComponentMineshaftCross(int_t componentType, Random &random,
	                        std::unique_ptr<StructureBoundingBox> bounds,
	                        int_t direction);

	static std::unique_ptr<StructureBoundingBox> findValidPlacement(
		const std::vector<StructureComponent *> &components, Random &random,
		int_t x, int_t y, int_t z, int_t direction);

	void buildComponent(StructureComponent *parent,
	                    std::vector<StructureComponent *> &components,
	                    Random &random) override;
	bool addComponentParts(World *world, Random &random,
	                       const StructureBoundingBox &chunkBounds) override;

private:
	int_t corridorDirection;
	bool isMultipleFloors;
};
