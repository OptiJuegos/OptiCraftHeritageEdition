#pragma once

#include <memory>
#include <vector>

#include "java/Type.h"

class ComponentNetherBridgePiece;
class Random;
class StructureComponent;
class StructureNetherBridgePieceWeight;

// net.minecraft.src.StructureNetherBridgePieces
class StructureNetherBridgePieces
{
public:
	static std::vector<std::unique_ptr<StructureNetherBridgePieceWeight>> createPrimaryWeights();
	static std::vector<std::unique_ptr<StructureNetherBridgePieceWeight>> createSecondaryWeights();

	static ComponentNetherBridgePiece *createNextComponent(
		const StructureNetherBridgePieceWeight &weight,
		std::vector<StructureComponent *> &components, Random &random,
		int_t x, int_t y, int_t z, int_t direction, int_t depth);
};
