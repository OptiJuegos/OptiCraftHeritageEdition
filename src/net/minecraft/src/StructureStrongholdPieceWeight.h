#pragma once

#include "java/Type.h"

enum class StrongholdPieceType
{
	Straight,
	Prison,
	LeftTurn,
	RightTurn,
	RoomCrossing,
	StairsStraight,
	Stairs,
	Crossing,
	ChestCorridor,
	Library,
	PortalRoom
};

// net.minecraft.src.StructureStrongholdPieceWeight
class StructureStrongholdPieceWeight
{
public:
	StructureStrongholdPieceWeight(StrongholdPieceType type, int_t weight, int_t limit);
	virtual ~StructureStrongholdPieceWeight() = default;

	virtual bool canSpawnMoreStructuresOfType(int_t depth) const;
	bool canSpawnMoreStructures() const;

	StrongholdPieceType pieceType;
	const int_t pieceWeight;
	int_t instancesSpawned;
	const int_t instancesLimit;
};
