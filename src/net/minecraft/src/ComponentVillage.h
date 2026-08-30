#pragma once

#include <vector>

#include "StructureComponent.h"

class ComponentVillageStartPiece;
class Random;
class StructureBoundingBox;
class World;

// net.minecraft.src.ComponentVillage
class ComponentVillage : public StructureComponent
{
public:
	explicit ComponentVillage(int_t componentType);

protected:
	StructureComponent *getNextComponentNN(ComponentVillageStartPiece *start,
	                                       std::vector<StructureComponent *> &components,
	                                       Random &random, int_t yOffset, int_t sideOffset);
	StructureComponent *getNextComponentPP(ComponentVillageStartPiece *start,
	                                       std::vector<StructureComponent *> &components,
	                                       Random &random, int_t yOffset, int_t sideOffset);
	int_t getAverageGroundLevel(World *world, const StructureBoundingBox &chunkBounds) const;
	static bool canVillageGoDeeper(const StructureBoundingBox *bounds);
	void spawnVillagers(World *world, const StructureBoundingBox &chunkBounds,
	                    int_t x, int_t y, int_t z, int_t count);
	virtual int_t getVillagerType(int_t index) const;

private:
	int_t villagersSpawned;
};
