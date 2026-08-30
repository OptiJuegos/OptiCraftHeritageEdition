#pragma once

#include <memory>

#include "ComponentStronghold.h"

class Random;
class StructureBoundingBox;

// net.minecraft.src.ComponentStrongholdRoomCrossing
class ComponentStrongholdRoomCrossing : public ComponentStronghold
{
public:
	ComponentStrongholdRoomCrossing(int_t type, Random &random,
	                                std::unique_ptr<StructureBoundingBox> bounds,
	                                int_t direction);
	void buildComponent(StructureComponent *parent,
	                    std::vector<StructureComponent *> &components,
	                    Random &random) override;
	bool addComponentParts(World *world, Random &random,
	                       const StructureBoundingBox &chunkBounds) override;
	static ComponentStrongholdRoomCrossing *findValidPlacement(
		const std::vector<StructureComponent *> &components, Random &random,
		int_t x, int_t y, int_t z, int_t direction, int_t type);

protected:
	EnumDoor doorType;
	int_t roomType;
};
