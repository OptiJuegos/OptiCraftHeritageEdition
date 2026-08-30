#include "ComponentNetherBridgeCrossing.h"

#include <memory>

#include "Block.h"
#include "ComponentNetherBridgeStartPiece.h"
#include "StructureBoundingBox.h"
#include "World.h"
#include "java/Random.h"

ComponentNetherBridgeCrossing::ComponentNetherBridgeCrossing(
	int_t type, Random &, const StructureBoundingBox &bounds, int_t direction)
	: ComponentNetherBridgePiece(type)
{
	coordBaseMode = direction;
	boundingBox = std::make_unique<StructureBoundingBox>(bounds);
}

void ComponentNetherBridgeCrossing::buildComponent(
	StructureComponent *parent, std::vector<StructureComponent *> &components, Random &random)
{
	auto *start = static_cast<ComponentNetherBridgeStartPiece *>(parent);
	getNextComponentNormal(start, components, random, 2, 0, false);
	getNextComponentX(start, components, random, 0, 2, false);
	getNextComponentZ(start, components, random, 0, 2, false);
}

ComponentNetherBridgeCrossing *ComponentNetherBridgeCrossing::createValidComponent(
	std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t depth)
{
	const StructureBoundingBox bounds = StructureBoundingBox::getComponentToAddBoundingBox(
		x, y, z, -2, 0, 0, 7, 9, 7, direction);
	return isAboveGround(&bounds) && StructureComponent::findIntersecting(components, bounds) == nullptr
		? new ComponentNetherBridgeCrossing(depth, random, bounds, direction)
		: nullptr;
}

bool ComponentNetherBridgeCrossing::addComponentParts(
	World *world, Random &, const StructureBoundingBox &b)
{
	const int_t brick = Block::netherBrick->blockID;
	const int_t fence = Block::netherFence->blockID;
	fillWithBlocks(world, b, 0, 0, 0, 6, 1, 6, brick, brick, false);
	fillWithBlocks(world, b, 0, 2, 0, 6, 7, 6, 0, 0, false);
	fillWithBlocks(world, b, 0, 2, 0, 1, 6, 0, brick, brick, false);
	fillWithBlocks(world, b, 0, 2, 6, 1, 6, 6, brick, brick, false);
	fillWithBlocks(world, b, 5, 2, 0, 6, 6, 0, brick, brick, false);
	fillWithBlocks(world, b, 5, 2, 6, 6, 6, 6, brick, brick, false);
	fillWithBlocks(world, b, 0, 2, 0, 0, 6, 1, brick, brick, false);
	fillWithBlocks(world, b, 0, 2, 5, 0, 6, 6, brick, brick, false);
	fillWithBlocks(world, b, 6, 2, 0, 6, 6, 1, brick, brick, false);
	fillWithBlocks(world, b, 6, 2, 5, 6, 6, 6, brick, brick, false);
	fillWithBlocks(world, b, 2, 6, 0, 4, 6, 0, brick, brick, false);
	fillWithBlocks(world, b, 2, 5, 0, 4, 5, 0, fence, fence, false);
	fillWithBlocks(world, b, 2, 6, 6, 4, 6, 6, brick, brick, false);
	fillWithBlocks(world, b, 2, 5, 6, 4, 5, 6, fence, fence, false);
	fillWithBlocks(world, b, 0, 6, 2, 0, 6, 4, brick, brick, false);
	fillWithBlocks(world, b, 0, 5, 2, 0, 5, 4, fence, fence, false);
	fillWithBlocks(world, b, 6, 6, 2, 6, 6, 4, brick, brick, false);
	fillWithBlocks(world, b, 6, 5, 2, 6, 5, 4, fence, fence, false);
	for (int_t x = 0; x <= 6; ++x)
		for (int_t z = 0; z <= 6; ++z)
			fillCurrentPositionBlocksDownwards(world, brick, 0, x, -1, z, b);
	return true;
}
