#pragma once
#include <memory>
#include "ComponentStronghold.h"
class Random; class StructureBoundingBox;
class ComponentStrongholdChestCorridor : public ComponentStronghold
{
public:
	ComponentStrongholdChestCorridor(int_t type, Random &random, std::unique_ptr<StructureBoundingBox> bounds, int_t direction);
	void buildComponent(StructureComponent *parent, std::vector<StructureComponent *> &components, Random &random) override;
	bool addComponentParts(World *world, Random &random, const StructureBoundingBox &chunkBounds) override;
	static ComponentStrongholdChestCorridor *findValidPlacement(const std::vector<StructureComponent *> &components, Random &random, int_t x, int_t y, int_t z, int_t direction, int_t type);
private:
	EnumDoor doorType;
	bool hasMadeChest = false;
};
