#pragma once

#include <vector>

#include "StructureVillagePieceWeight.h"
#include "java/Type.h"

class ComponentVillage;
class ComponentVillageStartPiece;
class Random;
class StructureComponent;

// net.minecraft.src.StructureVillagePieces
class StructureVillagePieces
{
public:
	static std::vector<StructureVillagePieceWeight *> getStructureVillageWeightedPieceList(
		Random &random, int_t terrainType);
	static StructureComponent *getNextStructureComponent(
		ComponentVillageStartPiece *start, std::vector<StructureComponent *> &components,
		Random &random, int_t x, int_t y, int_t z, int_t direction, int_t depth);
	static StructureComponent *getNextStructureComponentVillagePath(
		ComponentVillageStartPiece *start, std::vector<StructureComponent *> &components,
		Random &random, int_t x, int_t y, int_t z, int_t direction, int_t depth);

private:
	static int_t getAvailablePieceWeight(const std::vector<StructureVillagePieceWeight *> &weights);
	static ComponentVillage *getVillageComponentFromWeightedPiece(
		StructureVillagePieceWeight *weight, std::vector<StructureComponent *> &components,
		Random &random, int_t x, int_t y, int_t z, int_t direction, int_t depth);
	static ComponentVillage *getNextVillageComponent(
		ComponentVillageStartPiece *start, std::vector<StructureComponent *> &components,
		Random &random, int_t x, int_t y, int_t z, int_t direction, int_t depth);
	static StructureComponent *getNextVillageStructureComponent(
		ComponentVillageStartPiece *start, std::vector<StructureComponent *> &components,
		Random &random, int_t x, int_t y, int_t z, int_t direction, int_t depth);
	static StructureComponent *getNextComponentVillagePath(
		ComponentVillageStartPiece *start, std::vector<StructureComponent *> &components,
		Random &random, int_t x, int_t y, int_t z, int_t direction, int_t depth);
};
