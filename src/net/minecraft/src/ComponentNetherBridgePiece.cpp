#include "ComponentNetherBridgePiece.h"

#include <algorithm>
#include <cmath>

#include "ComponentNetherBridgeEnd.h"
#include "ComponentNetherBridgeStartPiece.h"
#include "StructureBoundingBox.h"
#include "StructureNetherBridgePieceWeight.h"
#include "StructureNetherBridgePieces.h"
#include "java/Arithmetic.h"
#include "java/Random.h"

ComponentNetherBridgePiece::ComponentNetherBridgePiece(int_t type)
	: StructureComponent(type)
{
}

int_t ComponentNetherBridgePiece::getTotalWeight(
	const std::vector<StructureNetherBridgePieceWeight *> &weights)
{
	bool hasLimitedPiece = false;
	int_t totalWeight = 0;
	for (const StructureNetherBridgePieceWeight *weight : weights)
	{
		if (weight == nullptr)
			continue;
		if (weight->maxCount > 0 && weight->spawnedCount < weight->maxCount)
			hasLimitedPiece = true;
		totalWeight += weight->weight;
	}
	return hasLimitedPiece ? totalWeight : -1;
}

ComponentNetherBridgePiece *ComponentNetherBridgePiece::getNextWeightedComponent(
	ComponentNetherBridgeStartPiece *start,
	std::vector<StructureNetherBridgePieceWeight *> &weights,
	std::vector<StructureComponent *> &components,
	Random &random, int_t x, int_t y, int_t z, int_t direction, int_t depth)
{
	const int_t totalWeight = getTotalWeight(weights);
	const bool canContinue = totalWeight > 0 && depth <= 30;
	for (int_t attempt = 0; attempt < 5 && canContinue; ++attempt)
	{
		int_t choice = random.nextInt(totalWeight);
		for (auto it = weights.begin(); it != weights.end(); ++it)
		{
			StructureNetherBridgePieceWeight *weight = *it;
			if (weight == nullptr)
				continue;
			choice -= weight->weight;
			if (choice >= 0)
				continue;

			if (!weight->canSpawnMoreStructuresOfType(depth) ||
			    (weight == start->previousPiece && !weight->allowInRow))
				break;

			ComponentNetherBridgePiece *piece = StructureNetherBridgePieces::createNextComponent(
				*weight, components, random, x, y, z, direction, depth);
			if (piece == nullptr)
				break;

			++weight->spawnedCount;
			start->previousPiece = weight;
			if (!weight->canSpawnMoreStructures())
				weights.erase(it);
			return piece;
		}
	}

	return ComponentNetherBridgeEnd::createValidComponent(components, random, x, y, z, direction, depth);
}

StructureComponent *ComponentNetherBridgePiece::getNextComponent(
	ComponentNetherBridgeStartPiece *start,
	std::vector<StructureComponent *> &components,
	Random &random, int_t x, int_t y, int_t z, int_t direction, int_t depth, bool secondary)
{
	if (start == nullptr || start->getBoundingBox() == nullptr)
		return nullptr;

	if (JavaArithmetic::intAbs(JavaArithmetic::intSub(x, start->getBoundingBox()->minX)) <= 112 &&
	    JavaArithmetic::intAbs(JavaArithmetic::intSub(z, start->getBoundingBox()->minZ)) <= 112)
	{
		std::vector<StructureNetherBridgePieceWeight *> &weights = secondary
			? start->secondaryWeights
			: start->primaryWeights;
		ComponentNetherBridgePiece *piece = getNextWeightedComponent(
			start, weights, components, random, x, y, z, direction, depth + 1);
		if (piece != nullptr)
		{
			components.push_back(piece);
			start->pendingChildren.push_back(piece);
		}
		return piece;
	}

	return ComponentNetherBridgeEnd::createValidComponent(
		components, random, x, y, z, direction, depth);
}

StructureComponent *ComponentNetherBridgePiece::getNextComponentNormal(
	ComponentNetherBridgeStartPiece *start, std::vector<StructureComponent *> &components,
	Random &random, int_t xOffset, int_t yOffset, bool secondary)
{
	if (!boundingBox)
		return nullptr;
	switch (coordBaseMode)
	{
	case 0:
		return getNextComponent(start, components, random, boundingBox->minX + xOffset,
		                        boundingBox->minY + yOffset, boundingBox->maxZ + 1,
		                        coordBaseMode, getComponentType(), secondary);
	case 1:
		return getNextComponent(start, components, random, boundingBox->minX - 1,
		                        boundingBox->minY + yOffset, boundingBox->minZ + xOffset,
		                        coordBaseMode, getComponentType(), secondary);
	case 2:
		return getNextComponent(start, components, random, boundingBox->minX + xOffset,
		                        boundingBox->minY + yOffset, boundingBox->minZ - 1,
		                        coordBaseMode, getComponentType(), secondary);
	case 3:
		return getNextComponent(start, components, random, boundingBox->maxX + 1,
		                        boundingBox->minY + yOffset, boundingBox->minZ + xOffset,
		                        coordBaseMode, getComponentType(), secondary);
	default:
		return nullptr;
	}
}

StructureComponent *ComponentNetherBridgePiece::getNextComponentX(
	ComponentNetherBridgeStartPiece *start, std::vector<StructureComponent *> &components,
	Random &random, int_t yOffset, int_t zOffset, bool secondary)
{
	if (!boundingBox)
		return nullptr;
	switch (coordBaseMode)
	{
	case 0:
	case 2:
		return getNextComponent(start, components, random, boundingBox->minX - 1,
		                        boundingBox->minY + yOffset, boundingBox->minZ + zOffset,
		                        1, getComponentType(), secondary);
	case 1:
	case 3:
		return getNextComponent(start, components, random, boundingBox->minX + zOffset,
		                        boundingBox->minY + yOffset, boundingBox->minZ - 1,
		                        2, getComponentType(), secondary);
	default:
		return nullptr;
	}
}

StructureComponent *ComponentNetherBridgePiece::getNextComponentZ(
	ComponentNetherBridgeStartPiece *start, std::vector<StructureComponent *> &components,
	Random &random, int_t yOffset, int_t xOffset, bool secondary)
{
	if (!boundingBox)
		return nullptr;
	switch (coordBaseMode)
	{
	case 0:
	case 2:
		return getNextComponent(start, components, random, boundingBox->maxX + 1,
		                        boundingBox->minY + yOffset, boundingBox->minZ + xOffset,
		                        3, getComponentType(), secondary);
	case 1:
	case 3:
		return getNextComponent(start, components, random, boundingBox->minX + xOffset,
		                        boundingBox->minY + yOffset, boundingBox->maxZ + 1,
		                        0, getComponentType(), secondary);
	default:
		return nullptr;
	}
}

bool ComponentNetherBridgePiece::isAboveGround(const StructureBoundingBox *bounds)
{
	return bounds != nullptr && bounds->minY > 10;
}
