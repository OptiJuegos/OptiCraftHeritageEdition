#include "ComponentNetherBridgeStraight.h"

#include <memory>

#include "Block.h"
#include "ComponentNetherBridgeStartPiece.h"
#include "StructureBoundingBox.h"
#include "World.h"
#include "java/Random.h"

ComponentNetherBridgeStraight::ComponentNetherBridgeStraight(
	int_t type, Random &, const StructureBoundingBox &bounds, int_t direction)
	: ComponentNetherBridgePiece(type)
{
	coordBaseMode = direction;
	boundingBox = std::make_unique<StructureBoundingBox>(bounds);
}

void ComponentNetherBridgeStraight::buildComponent(
	StructureComponent *parent, std::vector<StructureComponent *> &components, Random &random)
{
	getNextComponentNormal(static_cast<ComponentNetherBridgeStartPiece *>(parent), components, random, 1, 3, false);
}

ComponentNetherBridgeStraight *ComponentNetherBridgeStraight::createValidComponent(
	std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t depth)
{
	const StructureBoundingBox bounds = StructureBoundingBox::getComponentToAddBoundingBox(
		x, y, z, -1, -3, 0, 5, 10, 19, direction);
	return isAboveGround(&bounds) && StructureComponent::findIntersecting(components, bounds) == nullptr
		? new ComponentNetherBridgeStraight(depth, random, bounds, direction)
		: nullptr;
}

bool ComponentNetherBridgeStraight::addComponentParts(
	World *world, Random &, const StructureBoundingBox &b)
{
	const int_t brick = Block::netherBrick->blockID;
	const int_t fence = Block::netherFence->blockID;
	fillWithBlocks(world, b, 0, 3, 0, 4, 4, 18, brick, brick, false);
	fillWithBlocks(world, b, 1, 5, 0, 3, 7, 18, 0, 0, false);
	fillWithBlocks(world, b, 0, 5, 0, 0, 5, 18, brick, brick, false);
	fillWithBlocks(world, b, 4, 5, 0, 4, 5, 18, brick, brick, false);
	fillWithBlocks(world, b, 0, 2, 0, 4, 2, 5, brick, brick, false);
	fillWithBlocks(world, b, 0, 2, 13, 4, 2, 18, brick, brick, false);
	fillWithBlocks(world, b, 0, 0, 0, 4, 1, 3, brick, brick, false);
	fillWithBlocks(world, b, 0, 0, 15, 4, 1, 18, brick, brick, false);
	for (int_t x = 0; x <= 4; ++x)
	{
		for (int_t z = 0; z <= 2; ++z)
		{
			fillCurrentPositionBlocksDownwards(world, brick, 0, x, -1, z, b);
			fillCurrentPositionBlocksDownwards(world, brick, 0, x, -1, 18 - z, b);
		}
	}
	fillWithBlocks(world, b, 0, 1, 1, 0, 4, 1, fence, fence, false);
	fillWithBlocks(world, b, 0, 3, 4, 0, 4, 4, fence, fence, false);
	fillWithBlocks(world, b, 0, 3, 14, 0, 4, 14, fence, fence, false);
	fillWithBlocks(world, b, 0, 1, 17, 0, 4, 17, fence, fence, false);
	fillWithBlocks(world, b, 4, 1, 1, 4, 4, 1, fence, fence, false);
	fillWithBlocks(world, b, 4, 3, 4, 4, 4, 4, fence, fence, false);
	fillWithBlocks(world, b, 4, 3, 14, 4, 4, 14, fence, fence, false);
	fillWithBlocks(world, b, 4, 1, 17, 4, 4, 17, fence, fence, false);
	return true;
}
