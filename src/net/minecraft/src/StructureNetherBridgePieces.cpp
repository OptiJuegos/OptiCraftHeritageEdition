#include "StructureNetherBridgePieces.h"

#include "ComponentNetherBridgeCorridor.h"
#include "ComponentNetherBridgeCorridor2.h"
#include "ComponentNetherBridgeCorridor3.h"
#include "ComponentNetherBridgeCorridor4.h"
#include "ComponentNetherBridgeCorridor5.h"
#include "ComponentNetherBridgeCrossing.h"
#include "ComponentNetherBridgeCrossing2.h"
#include "ComponentNetherBridgeCrossing3.h"
#include "ComponentNetherBridgeEntrance.h"
#include "ComponentNetherBridgeNetherStalkRoom.h"
#include "ComponentNetherBridgeStairs.h"
#include "ComponentNetherBridgeStraight.h"
#include "ComponentNetherBridgeThrone.h"
#include "StructureNetherBridgePieceWeight.h"

std::vector<std::unique_ptr<StructureNetherBridgePieceWeight>>
StructureNetherBridgePieces::createPrimaryWeights()
{
	std::vector<std::unique_ptr<StructureNetherBridgePieceWeight>> result;
	result.emplace_back(new StructureNetherBridgePieceWeight(NetherBridgePieceType::Straight, 30, 0, true));
	result.emplace_back(new StructureNetherBridgePieceWeight(NetherBridgePieceType::Crossing3, 10, 4));
	result.emplace_back(new StructureNetherBridgePieceWeight(NetherBridgePieceType::Crossing, 10, 4));
	result.emplace_back(new StructureNetherBridgePieceWeight(NetherBridgePieceType::Stairs, 10, 3));
	result.emplace_back(new StructureNetherBridgePieceWeight(NetherBridgePieceType::Throne, 5, 2));
	result.emplace_back(new StructureNetherBridgePieceWeight(NetherBridgePieceType::Entrance, 5, 1));
	return result;
}

std::vector<std::unique_ptr<StructureNetherBridgePieceWeight>>
StructureNetherBridgePieces::createSecondaryWeights()
{
	std::vector<std::unique_ptr<StructureNetherBridgePieceWeight>> result;
	result.emplace_back(new StructureNetherBridgePieceWeight(NetherBridgePieceType::Corridor5, 25, 0, true));
	result.emplace_back(new StructureNetherBridgePieceWeight(NetherBridgePieceType::Crossing2, 15, 5));
	result.emplace_back(new StructureNetherBridgePieceWeight(NetherBridgePieceType::Corridor2, 5, 10));
	result.emplace_back(new StructureNetherBridgePieceWeight(NetherBridgePieceType::Corridor, 5, 10));
	result.emplace_back(new StructureNetherBridgePieceWeight(NetherBridgePieceType::Corridor3, 10, 3, true));
	result.emplace_back(new StructureNetherBridgePieceWeight(NetherBridgePieceType::Corridor4, 7, 2));
	result.emplace_back(new StructureNetherBridgePieceWeight(NetherBridgePieceType::NetherStalkRoom, 5, 2));
	return result;
}

ComponentNetherBridgePiece *StructureNetherBridgePieces::createNextComponent(
	const StructureNetherBridgePieceWeight &weight,
	std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t depth)
{
	switch (weight.pieceType)
	{
	case NetherBridgePieceType::Straight:
		return ComponentNetherBridgeStraight::createValidComponent(components, random, x, y, z, direction, depth);
	case NetherBridgePieceType::Crossing3:
		return ComponentNetherBridgeCrossing3::createValidComponent(components, random, x, y, z, direction, depth);
	case NetherBridgePieceType::Crossing:
		return ComponentNetherBridgeCrossing::createValidComponent(components, random, x, y, z, direction, depth);
	case NetherBridgePieceType::Stairs:
		return ComponentNetherBridgeStairs::createValidComponent(components, random, x, y, z, direction, depth);
	case NetherBridgePieceType::Throne:
		return ComponentNetherBridgeThrone::createValidComponent(components, random, x, y, z, direction, depth);
	case NetherBridgePieceType::Entrance:
		return ComponentNetherBridgeEntrance::createValidComponent(components, random, x, y, z, direction, depth);
	case NetherBridgePieceType::Corridor5:
		return ComponentNetherBridgeCorridor5::createValidComponent(components, random, x, y, z, direction, depth);
	case NetherBridgePieceType::Crossing2:
		return ComponentNetherBridgeCrossing2::createValidComponent(components, random, x, y, z, direction, depth);
	case NetherBridgePieceType::Corridor2:
		return ComponentNetherBridgeCorridor2::createValidComponent(components, random, x, y, z, direction, depth);
	case NetherBridgePieceType::Corridor:
		return ComponentNetherBridgeCorridor::createValidComponent(components, random, x, y, z, direction, depth);
	case NetherBridgePieceType::Corridor3:
		return ComponentNetherBridgeCorridor3::createValidComponent(components, random, x, y, z, direction, depth);
	case NetherBridgePieceType::Corridor4:
		return ComponentNetherBridgeCorridor4::createValidComponent(components, random, x, y, z, direction, depth);
	case NetherBridgePieceType::NetherStalkRoom:
		return ComponentNetherBridgeNetherStalkRoom::createValidComponent(components, random, x, y, z, direction, depth);
	default:
		return nullptr;
	}
}
