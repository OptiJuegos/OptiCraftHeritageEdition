#pragma once

#include <vector>

#include "StructureComponent.h"

class ComponentNetherBridgeStartPiece;
class Random;
class StructureNetherBridgePieceWeight;

// net.minecraft.src.ComponentNetherBridgePiece
class ComponentNetherBridgePiece : public StructureComponent
{
public:
	explicit ComponentNetherBridgePiece(int_t componentType);

protected:
	StructureComponent *getNextComponentNormal(ComponentNetherBridgeStartPiece *start,
	                                           std::vector<StructureComponent *> &components,
	                                           Random &random, int_t xOffset, int_t yOffset,
	                                           bool secondary);
	StructureComponent *getNextComponentX(ComponentNetherBridgeStartPiece *start,
	                                      std::vector<StructureComponent *> &components,
	                                      Random &random, int_t yOffset, int_t zOffset,
	                                      bool secondary);
	StructureComponent *getNextComponentZ(ComponentNetherBridgeStartPiece *start,
	                                      std::vector<StructureComponent *> &components,
	                                      Random &random, int_t yOffset, int_t xOffset,
	                                      bool secondary);

	static bool isAboveGround(const StructureBoundingBox *bounds);

private:
	static int_t getTotalWeight(const std::vector<StructureNetherBridgePieceWeight *> &weights);
	ComponentNetherBridgePiece *getNextWeightedComponent(
		ComponentNetherBridgeStartPiece *start,
		std::vector<StructureNetherBridgePieceWeight *> &weights,
		std::vector<StructureComponent *> &components,
		Random &random, int_t x, int_t y, int_t z, int_t direction, int_t depth);
	StructureComponent *getNextComponent(ComponentNetherBridgeStartPiece *start,
	                                     std::vector<StructureComponent *> &components,
	                                     Random &random, int_t x, int_t y, int_t z,
	                                     int_t direction, int_t depth, bool secondary);
};
