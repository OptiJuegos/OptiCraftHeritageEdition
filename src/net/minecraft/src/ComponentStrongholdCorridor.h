#pragma once
#include <memory>
#include "ComponentStronghold.h"
class Random; class StructureBoundingBox;
class ComponentStrongholdCorridor : public ComponentStronghold
{
public:
	ComponentStrongholdCorridor(int_t type, Random &random, std::unique_ptr<StructureBoundingBox> bounds, int_t direction);
	void buildComponent(StructureComponent *, std::vector<StructureComponent *> &, Random &) override;
	bool addComponentParts(World *world, Random &random, const StructureBoundingBox &chunkBounds) override;
	static std::unique_ptr<StructureBoundingBox> func_35051_a(const std::vector<StructureComponent *> &components, Random &random, int_t x, int_t y, int_t z, int_t direction);
private:
	int_t field_35052_a;
};
