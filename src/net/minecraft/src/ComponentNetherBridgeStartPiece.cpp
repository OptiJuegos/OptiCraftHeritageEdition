#include "ComponentNetherBridgeStartPiece.h"

#include "StructureNetherBridgePieces.h"
#include "StructureNetherBridgePieceWeight.h"
#include "java/Random.h"

ComponentNetherBridgeStartPiece::ComponentNetherBridgeStartPiece(Random &random, int_t x, int_t z)
	: ComponentNetherBridgeCrossing3(random, x, z), previousPiece(nullptr)
{
	primaryWeightStorage = StructureNetherBridgePieces::createPrimaryWeights();
	primaryWeights.reserve(primaryWeightStorage.size());
	for (const auto &weight : primaryWeightStorage)
		primaryWeights.push_back(weight.get());

	secondaryWeightStorage = StructureNetherBridgePieces::createSecondaryWeights();
	secondaryWeights.reserve(secondaryWeightStorage.size());
	for (const auto &weight : secondaryWeightStorage)
		secondaryWeights.push_back(weight.get());
}
