#pragma once

#include <vector>

#include "StructureStrongholdPieceWeight.h"
#include "StructureStrongholdStones.h"

class ComponentStronghold;
class ComponentStrongholdStairs2;
class Random;
class StructureComponent;

// net.minecraft.src.StructureStrongholdPieces
class StructureStrongholdPieces
{
public:
	static void prepareStructurePieces();
	static StructureComponent *getNextValidComponentAccess(
		ComponentStrongholdStairs2 *start,
		std::vector<StructureComponent *> &components,
		Random &random, int_t x, int_t y, int_t z,
		int_t direction, int_t depth);
	static StrongholdPieceType setComponentType(StrongholdPieceType type);
	static StructureStrongholdStones &getStrongholdStones();

private:
	static bool canAddStructurePieces();
	static ComponentStronghold *getStrongholdComponentFromWeightedPiece(
		StrongholdPieceType type, std::vector<StructureComponent *> &components,
		Random &random, int_t x, int_t y, int_t z, int_t direction, int_t depth);
	static ComponentStronghold *getNextComponent(
		ComponentStrongholdStairs2 *start,
		std::vector<StructureComponent *> &components,
		Random &random, int_t x, int_t y, int_t z,
		int_t direction, int_t depth);
	static StructureComponent *getNextValidComponent(
		ComponentStrongholdStairs2 *start,
		std::vector<StructureComponent *> &components,
		Random &random, int_t x, int_t y, int_t z,
		int_t direction, int_t depth);
};
