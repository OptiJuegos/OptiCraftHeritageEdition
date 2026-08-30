#pragma once

#include <memory>

#include "ComponentVillageRoadPiece.h"

class ComponentVillageStartPiece;
class Random;
class StructureBoundingBox;

// net.minecraft.src.ComponentVillagePathGen
class ComponentVillagePathGen : public ComponentVillageRoadPiece
{
public:
	ComponentVillagePathGen(int_t type, Random &random,
	                        std::unique_ptr<StructureBoundingBox> bounds, int_t direction);
	void buildComponent(StructureComponent *parent, std::vector<StructureComponent *> &components,
	                    Random &random) override;
	bool addComponentParts(World *world, Random &random,
	                       const StructureBoundingBox &chunkBounds) override;

	static std::unique_ptr<StructureBoundingBox> func_35087_a(
		ComponentVillageStartPiece *start, const std::vector<StructureComponent *> &components,
		Random &random, int_t x, int_t y, int_t z, int_t direction);

private:
	int_t averageGroundLevel;
};
