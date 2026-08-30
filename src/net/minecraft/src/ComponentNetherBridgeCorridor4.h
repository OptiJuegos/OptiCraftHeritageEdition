#pragma once

#include "ComponentNetherBridgePiece.h"

// net.minecraft.src.ComponentNetherBridgeCorridor4
class ComponentNetherBridgeCorridor4 : public ComponentNetherBridgePiece
{
public:
	ComponentNetherBridgeCorridor4(int_t componentType, Random &random,
	                               const StructureBoundingBox &bounds, int_t direction);
	void buildComponent(StructureComponent *parent, std::vector<StructureComponent *> &components,
	                    Random &random) override;
	bool addComponentParts(World *world, Random &random,
	                       const StructureBoundingBox &chunkBounds) override;
	static ComponentNetherBridgeCorridor4 *createValidComponent(
		std::vector<StructureComponent *> &components, Random &random,
		int_t x, int_t y, int_t z, int_t direction, int_t depth);
};
