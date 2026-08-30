#include "StructureStrongholdPieces.h"

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <vector>

#include "ComponentStronghold.h"
#include "ComponentStrongholdChestCorridor.h"
#include "ComponentStrongholdCorridor.h"
#include "ComponentStrongholdCrossing.h"
#include "ComponentStrongholdLeftTurn.h"
#include "ComponentStrongholdLibrary.h"
#include "ComponentStrongholdPortalRoom.h"
#include "ComponentStrongholdPrison.h"
#include "ComponentStrongholdRightTurn.h"
#include "ComponentStrongholdRoomCrossing.h"
#include "ComponentStrongholdStairs.h"
#include "ComponentStrongholdStairs2.h"
#include "ComponentStrongholdStairsStraight.h"
#include "ComponentStrongholdStraight.h"
#include "StructureBoundingBox.h"
#include "StructureComponent.h"
#include "StructureStrongholdPieceWeight2.h"
#include "StructureStrongholdPieceWeight3.h"
#include "StructureStrongholdStones.h"
#include "java/Arithmetic.h"
#include "java/Random.h"

namespace
{
std::vector<std::unique_ptr<StructureStrongholdPieceWeight>> pieceWeights;
std::vector<StructureStrongholdPieceWeight *> structurePieceList;
StrongholdPieceType strongComponentType = StrongholdPieceType::Straight;
bool hasStrongComponentType = false;
int_t totalWeight = 0;
StructureStrongholdStones strongholdStones;

void ensurePieceWeights()
{
	if (!pieceWeights.empty())
		return;

	pieceWeights.emplace_back(new StructureStrongholdPieceWeight(StrongholdPieceType::Straight, 40, 0));
	pieceWeights.emplace_back(new StructureStrongholdPieceWeight(StrongholdPieceType::Prison, 5, 5));
	pieceWeights.emplace_back(new StructureStrongholdPieceWeight(StrongholdPieceType::LeftTurn, 20, 0));
	pieceWeights.emplace_back(new StructureStrongholdPieceWeight(StrongholdPieceType::RightTurn, 20, 0));
	pieceWeights.emplace_back(new StructureStrongholdPieceWeight(StrongholdPieceType::RoomCrossing, 10, 6));
	pieceWeights.emplace_back(new StructureStrongholdPieceWeight(StrongholdPieceType::StairsStraight, 5, 5));
	pieceWeights.emplace_back(new StructureStrongholdPieceWeight(StrongholdPieceType::Stairs, 5, 5));
	pieceWeights.emplace_back(new StructureStrongholdPieceWeight(StrongholdPieceType::Crossing, 5, 4));
	pieceWeights.emplace_back(new StructureStrongholdPieceWeight(StrongholdPieceType::ChestCorridor, 5, 4));
	pieceWeights.emplace_back(new StructureStrongholdPieceWeight2(StrongholdPieceType::Library, 10, 2));
	pieceWeights.emplace_back(new StructureStrongholdPieceWeight3(StrongholdPieceType::PortalRoom, 20, 1));
}
} // namespace

void StructureStrongholdPieces::prepareStructurePieces()
{
	ensurePieceWeights();
	structurePieceList.clear();
	structurePieceList.reserve(pieceWeights.size());
	for (const std::unique_ptr<StructureStrongholdPieceWeight> &weight : pieceWeights)
	{
		weight->instancesSpawned = 0;
		structurePieceList.push_back(weight.get());
	}
	hasStrongComponentType = false;
}

bool StructureStrongholdPieces::canAddStructurePieces()
{
	bool hasLimitedPieceRemaining = false;
	totalWeight = 0;
	for (StructureStrongholdPieceWeight *weight : structurePieceList)
	{
		if (weight == nullptr)
			continue;
		if (weight->instancesLimit > 0 && weight->instancesSpawned < weight->instancesLimit)
			hasLimitedPieceRemaining = true;
		totalWeight += weight->pieceWeight;
	}
	return hasLimitedPieceRemaining;
}

ComponentStronghold *StructureStrongholdPieces::getStrongholdComponentFromWeightedPiece(
	StrongholdPieceType type, std::vector<StructureComponent *> &components,
	Random &random, int_t x, int_t y, int_t z, int_t direction, int_t depth)
{
	switch (type)
	{
	case StrongholdPieceType::Straight:
		return ComponentStrongholdStraight::findValidPlacement(components, random, x, y, z, direction, depth);
	case StrongholdPieceType::Prison:
		return ComponentStrongholdPrison::findValidPlacement(components, random, x, y, z, direction, depth);
	case StrongholdPieceType::LeftTurn:
		return ComponentStrongholdLeftTurn::findValidPlacement(components, random, x, y, z, direction, depth);
	case StrongholdPieceType::RightTurn:
		return ComponentStrongholdRightTurn::findValidPlacement(components, random, x, y, z, direction, depth);
	case StrongholdPieceType::RoomCrossing:
		return ComponentStrongholdRoomCrossing::findValidPlacement(components, random, x, y, z, direction, depth);
	case StrongholdPieceType::StairsStraight:
		return ComponentStrongholdStairsStraight::findValidPlacement(components, random, x, y, z, direction, depth);
	case StrongholdPieceType::Stairs:
		return ComponentStrongholdStairs::getStrongholdStairsComponent(components, random, x, y, z, direction, depth);
	case StrongholdPieceType::Crossing:
		return ComponentStrongholdCrossing::findValidPlacement(components, random, x, y, z, direction, depth);
	case StrongholdPieceType::ChestCorridor:
		return ComponentStrongholdChestCorridor::findValidPlacement(components, random, x, y, z, direction, depth);
	case StrongholdPieceType::Library:
		return ComponentStrongholdLibrary::findValidPlacement(components, random, x, y, z, direction, depth);
	case StrongholdPieceType::PortalRoom:
		return ComponentStrongholdPortalRoom::findValidPlacement(components, random, x, y, z, direction, depth);
	}
	return nullptr;
}

ComponentStronghold *StructureStrongholdPieces::getNextComponent(
	ComponentStrongholdStairs2 *start, std::vector<StructureComponent *> &components,
	Random &random, int_t x, int_t y, int_t z, int_t direction, int_t depth)
{
	if (!canAddStructurePieces())
		return nullptr;

	if (hasStrongComponentType)
	{
		ComponentStronghold *forced = getStrongholdComponentFromWeightedPiece(
			strongComponentType, components, random, x, y, z, direction, depth);
		hasStrongComponentType = false;
		if (forced != nullptr)
			return forced;
	}

	for (int_t attempt = 0; attempt < 5; ++attempt)
	{
		int_t selection = random.nextInt(totalWeight);
		for (auto it = structurePieceList.begin(); it != structurePieceList.end(); ++it)
		{
			StructureStrongholdPieceWeight *weight = *it;
			selection -= weight->pieceWeight;
			if (selection >= 0)
				continue;

			if (!weight->canSpawnMoreStructuresOfType(depth) || weight == start->field_35038_a)
				break;

			ComponentStronghold *component = getStrongholdComponentFromWeightedPiece(
				weight->pieceType, components, random, x, y, z, direction, depth);
			if (component != nullptr)
			{
				++weight->instancesSpawned;
				start->field_35038_a = weight;
				if (!weight->canSpawnMoreStructures())
					structurePieceList.erase(it);
				return component;
			}
		}
	}

	std::unique_ptr<StructureBoundingBox> bounds = ComponentStrongholdCorridor::func_35051_a(
		components, random, x, y, z, direction);
	if (bounds != nullptr && bounds->minY > 1)
		return new ComponentStrongholdCorridor(depth, random, std::move(bounds), direction);
	return nullptr;
}

StructureComponent *StructureStrongholdPieces::getNextValidComponent(
	ComponentStrongholdStairs2 *start, std::vector<StructureComponent *> &components,
	Random &random, int_t x, int_t y, int_t z, int_t direction, int_t depth)
{
	if (start == nullptr || start->getBoundingBox() == nullptr || depth > 50)
		return nullptr;
	if (JavaArithmetic::intAbs(JavaArithmetic::intSub(x, start->getBoundingBox()->minX)) > 112 ||
	    JavaArithmetic::intAbs(JavaArithmetic::intSub(z, start->getBoundingBox()->minZ)) > 112)
		return nullptr;

	ComponentStronghold *component = getNextComponent(
		start, components, random, x, y, z, direction, depth + 1);
	if (component != nullptr)
	{
		components.push_back(component);
		start->field_35037_b.push_back(component);
	}
	return component;
}

StructureComponent *StructureStrongholdPieces::getNextValidComponentAccess(
	ComponentStrongholdStairs2 *start, std::vector<StructureComponent *> &components,
	Random &random, int_t x, int_t y, int_t z, int_t direction, int_t depth)
{
	return getNextValidComponent(start, components, random, x, y, z, direction, depth);
}

StrongholdPieceType StructureStrongholdPieces::setComponentType(StrongholdPieceType type)
{
	strongComponentType = type;
	hasStrongComponentType = true;
	return type;
}

StructureStrongholdStones &StructureStrongholdPieces::getStrongholdStones()
{
	return strongholdStones;
}
