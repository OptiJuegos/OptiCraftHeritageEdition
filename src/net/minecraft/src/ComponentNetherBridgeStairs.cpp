#include "ComponentNetherBridgeStairs.h"

#include <memory>

#include "Block.h"
#include "ComponentNetherBridgeStartPiece.h"
#include "StructureBoundingBox.h"
#include "World.h"
#include "java/Random.h"

ComponentNetherBridgeStairs::ComponentNetherBridgeStairs(
	int_t type, Random &, const StructureBoundingBox &bounds, int_t direction)
	: ComponentNetherBridgePiece(type)
{
	coordBaseMode = direction;
	boundingBox = std::make_unique<StructureBoundingBox>(bounds);
}

void ComponentNetherBridgeStairs::buildComponent(
	StructureComponent *parent, std::vector<StructureComponent *> &components, Random &random)
{
	getNextComponentZ(static_cast<ComponentNetherBridgeStartPiece *>(parent), components, random, 6, 2, false);
}

ComponentNetherBridgeStairs *ComponentNetherBridgeStairs::createValidComponent(
	std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t depth)
{
	const StructureBoundingBox bounds = StructureBoundingBox::getComponentToAddBoundingBox(
		x, y, z, -2, 0, 0, 7, 11, 7, direction);
	return isAboveGround(&bounds) && StructureComponent::findIntersecting(components, bounds) == nullptr
		? new ComponentNetherBridgeStairs(depth, random, bounds, direction)
		: nullptr;
}

bool ComponentNetherBridgeStairs::addComponentParts(
	World *world, Random &, const StructureBoundingBox &b)
{
	const int_t brick = Block::netherBrick->blockID;
	const int_t fence = Block::netherFence->blockID;
	fillWithBlocks(world, b, 0, 0, 0, 6, 1, 6, brick, brick, false);
	fillWithBlocks(world, b, 0, 2, 0, 6, 10, 6, 0, 0, false);
	fillWithBlocks(world, b, 0, 2, 0, 1, 8, 0, brick, brick, false);
	fillWithBlocks(world, b, 5, 2, 0, 6, 8, 0, brick, brick, false);
	fillWithBlocks(world, b, 0, 2, 1, 0, 8, 6, brick, brick, false);
	fillWithBlocks(world, b, 6, 2, 1, 6, 8, 6, brick, brick, false);
	fillWithBlocks(world, b, 1, 2, 6, 5, 8, 6, brick, brick, false);
	fillWithBlocks(world, b, 0, 3, 2, 0, 5, 4, fence, fence, false);
	fillWithBlocks(world, b, 6, 3, 2, 6, 5, 2, fence, fence, false);
	fillWithBlocks(world, b, 6, 3, 4, 6, 5, 4, fence, fence, false);
	placeBlockAtCurrentPosition(world, brick, 0, 5, 2, 5, b);
	fillWithBlocks(world, b, 4, 2, 5, 4, 3, 5, brick, brick, false);
	fillWithBlocks(world, b, 3, 2, 5, 3, 4, 5, brick, brick, false);
	fillWithBlocks(world, b, 2, 2, 5, 2, 5, 5, brick, brick, false);
	fillWithBlocks(world, b, 1, 2, 5, 1, 6, 5, brick, brick, false);
	fillWithBlocks(world, b, 1, 7, 1, 5, 7, 4, brick, brick, false);
	fillWithBlocks(world, b, 6, 8, 2, 6, 8, 4, 0, 0, false);
	fillWithBlocks(world, b, 2, 6, 0, 4, 8, 0, brick, brick, false);
	fillWithBlocks(world, b, 2, 5, 0, 4, 5, 0, fence, fence, false);
	for (int_t x = 0; x <= 6; ++x)
		for (int_t z = 0; z <= 6; ++z)
			fillCurrentPositionBlocksDownwards(world, brick, 0, x, -1, z, b);
	return true;
}
