#pragma once
#include <memory>
#include "ComponentStronghold.h"
class Random; class StructureBoundingBox;
class ComponentStrongholdStairs : public ComponentStronghold
{
public:
	ComponentStrongholdStairs(int_t type, Random &random, int_t x, int_t z);
	ComponentStrongholdStairs(int_t type, Random &random, std::unique_ptr<StructureBoundingBox> bounds, int_t direction);
	void buildComponent(StructureComponent *parent, std::vector<StructureComponent *> &components, Random &random) override;
	bool addComponentParts(World *world, Random &random, const StructureBoundingBox &chunkBounds) override;
	static ComponentStrongholdStairs *getStrongholdStairsComponent(const std::vector<StructureComponent *> &components, Random &random, int_t x, int_t y, int_t z, int_t direction, int_t type);
protected:
	bool field_35036_a;
	EnumDoor doorType;
};
