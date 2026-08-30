#pragma once

#include <memory>
#include <vector>

#include "ComponentNetherBridgeCrossing3.h"

class StructureNetherBridgePieceWeight;

// net.minecraft.src.ComponentNetherBridgeStartPiece
class ComponentNetherBridgeStartPiece : public ComponentNetherBridgeCrossing3
{
public:
	ComponentNetherBridgeStartPiece(Random &random, int_t x, int_t z);

	StructureNetherBridgePieceWeight *previousPiece;
	std::vector<StructureNetherBridgePieceWeight *> primaryWeights;
	std::vector<StructureNetherBridgePieceWeight *> secondaryWeights;
	std::vector<StructureComponent *> pendingChildren;

private:
	std::vector<std::unique_ptr<StructureNetherBridgePieceWeight>> primaryWeightStorage;
	std::vector<std::unique_ptr<StructureNetherBridgePieceWeight>> secondaryWeightStorage;
};
