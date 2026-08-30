#pragma once

#include <memory>

#include "ComponentStronghold.h"

class Random;
class StructureBoundingBox;

// net.minecraft.src.ComponentStrongholdLibrary
class ComponentStrongholdLibrary : public ComponentStronghold
{
public:
	ComponentStrongholdLibrary(int_t type, Random &random,
	                           std::unique_ptr<StructureBoundingBox> bounds,
	                           int_t direction);
	void buildComponent(StructureComponent *, std::vector<StructureComponent *> &, Random &) override;
	bool addComponentParts(World *world, Random &random,
	                       const StructureBoundingBox &chunkBounds) override;
	static ComponentStrongholdLibrary *findValidPlacement(
		const std::vector<StructureComponent *> &components, Random &random,
		int_t x, int_t y, int_t z, int_t direction, int_t type);

protected:
	EnumDoor doorType;

private:
	bool isLargeRoom;
};
