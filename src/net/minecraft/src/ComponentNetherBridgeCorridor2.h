#pragma once

#include "ComponentNetherBridgePiece.h"

// net.minecraft.src.ComponentNetherBridgeCorridor2
class ComponentNetherBridgeCorridor2 : public ComponentNetherBridgePiece
{
public:
	ComponentNetherBridgeCorridor2(int_t componentType, Random &random, const StructureBoundingBox &bounds, int_t direction);

	void buildComponent(StructureComponent *parent, std::vector<StructureComponent *> &components, Random &random) override;
	bool addComponentParts(World *world, Random &random, const StructureBoundingBox &chunkBounds) override;

	static ComponentNetherBridgeCorridor2 *createValidComponent(std::vector<StructureComponent *> &components, Random &random,
	                                      int_t x, int_t y, int_t z, int_t direction, int_t depth);
};
