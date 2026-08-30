#pragma once
#include <memory>
#include "ComponentStronghold.h"
class Random; class StructureBoundingBox;
class ComponentStrongholdCrossing : public ComponentStronghold
{
public:
	ComponentStrongholdCrossing(int_t type, Random &random, std::unique_ptr<StructureBoundingBox> bounds, int_t direction);
	void buildComponent(StructureComponent *parent, std::vector<StructureComponent *> &components, Random &random) override;
	bool addComponentParts(World *world, Random &random, const StructureBoundingBox &chunkBounds) override;
	static ComponentStrongholdCrossing *findValidPlacement(const std::vector<StructureComponent *> &components, Random &random, int_t x, int_t y, int_t z, int_t direction, int_t type);
protected:
	EnumDoor doorType;
private:
	bool field_35042_b;
	bool field_35043_c;
	bool field_35040_d;
	bool field_35041_e;
};
