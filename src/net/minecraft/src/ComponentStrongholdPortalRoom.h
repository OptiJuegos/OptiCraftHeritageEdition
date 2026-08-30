#pragma once

#include <memory>

#include "ComponentStronghold.h"

class Random;
class StructureBoundingBox;

// net.minecraft.src.ComponentStrongholdPortalRoom
class ComponentStrongholdPortalRoom : public ComponentStronghold
{
public:
	ComponentStrongholdPortalRoom(int_t type, Random &random,
	                              std::unique_ptr<StructureBoundingBox> bounds,
	                              int_t direction);
	void buildComponent(StructureComponent *parent,
	                    std::vector<StructureComponent *> &components,
	                    Random &random) override;
	bool addComponentParts(World *world, Random &random,
	                       const StructureBoundingBox &chunkBounds) override;
	static ComponentStrongholdPortalRoom *findValidPlacement(
		const std::vector<StructureComponent *> &components, Random &random,
		int_t x, int_t y, int_t z, int_t direction, int_t type);

private:
	bool hasSpawner = false;
};
