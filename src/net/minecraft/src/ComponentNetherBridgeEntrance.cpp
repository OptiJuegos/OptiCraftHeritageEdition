#include "ComponentNetherBridgeEntrance.h"

#include <memory>

#include "Block.h"
#include "ComponentNetherBridgeStartPiece.h"
#include "StructureBoundingBox.h"
#include "World.h"
#include "java/Random.h"

ComponentNetherBridgeEntrance::ComponentNetherBridgeEntrance(
	int_t type, Random &, const StructureBoundingBox &bounds, int_t direction)
	: ComponentNetherBridgePiece(type)
{
	coordBaseMode = direction;
	boundingBox = std::make_unique<StructureBoundingBox>(bounds);
}

void ComponentNetherBridgeEntrance::buildComponent(
	StructureComponent *parent, std::vector<StructureComponent *> &components, Random &random)
{
	getNextComponentNormal(static_cast<ComponentNetherBridgeStartPiece *>(parent), components, random, 5, 3, true);
}

ComponentNetherBridgeEntrance *ComponentNetherBridgeEntrance::createValidComponent(
	std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t depth)
{
	const StructureBoundingBox bounds = StructureBoundingBox::getComponentToAddBoundingBox(
		x, y, z, -5, -3, 0, 13, 14, 13, direction);
	return isAboveGround(&bounds) && StructureComponent::findIntersecting(components, bounds) == nullptr
		? new ComponentNetherBridgeEntrance(depth, random, bounds, direction)
		: nullptr;
}

bool ComponentNetherBridgeEntrance::addComponentParts(
	World *world, Random &random, const StructureBoundingBox &b)
{
	const int_t brick = Block::netherBrick->blockID;
	const int_t fence = Block::netherFence->blockID;
	fillWithBlocks(world, b, 0, 3, 0, 12, 4, 12, brick, brick, false);
	fillWithBlocks(world, b, 0, 5, 0, 12, 13, 12, 0, 0, false);
	fillWithBlocks(world, b, 0, 5, 0, 1, 12, 12, brick, brick, false);
	fillWithBlocks(world, b, 11, 5, 0, 12, 12, 12, brick, brick, false);
	fillWithBlocks(world, b, 2, 5, 11, 4, 12, 12, brick, brick, false);
	fillWithBlocks(world, b, 8, 5, 11, 10, 12, 12, brick, brick, false);
	fillWithBlocks(world, b, 5, 9, 11, 7, 12, 12, brick, brick, false);
	fillWithBlocks(world, b, 2, 5, 0, 4, 12, 1, brick, brick, false);
	fillWithBlocks(world, b, 8, 5, 0, 10, 12, 1, brick, brick, false);
	fillWithBlocks(world, b, 5, 9, 0, 7, 12, 1, brick, brick, false);
	fillWithBlocks(world, b, 2, 11, 2, 10, 12, 10, brick, brick, false);
	fillWithBlocks(world, b, 5, 8, 0, 7, 8, 0, fence, fence, false);

	for (int_t i = 1; i <= 11; i += 2)
	{
		fillWithBlocks(world, b, i, 10, 0, i, 11, 0, fence, fence, false);
		fillWithBlocks(world, b, i, 10, 12, i, 11, 12, fence, fence, false);
		fillWithBlocks(world, b, 0, 10, i, 0, 11, i, fence, fence, false);
		fillWithBlocks(world, b, 12, 10, i, 12, 11, i, fence, fence, false);
		placeBlockAtCurrentPosition(world, brick, 0, i, 13, 0, b);
		placeBlockAtCurrentPosition(world, brick, 0, i, 13, 12, b);
		placeBlockAtCurrentPosition(world, brick, 0, 0, 13, i, b);
		placeBlockAtCurrentPosition(world, brick, 0, 12, 13, i, b);
		placeBlockAtCurrentPosition(world, fence, 0, i + 1, 13, 0, b);
		placeBlockAtCurrentPosition(world, fence, 0, i + 1, 13, 12, b);
		placeBlockAtCurrentPosition(world, fence, 0, 0, 13, i + 1, b);
		placeBlockAtCurrentPosition(world, fence, 0, 12, 13, i + 1, b);
	}
	placeBlockAtCurrentPosition(world, fence, 0, 0, 13, 0, b);
	placeBlockAtCurrentPosition(world, fence, 0, 0, 13, 12, b);
	placeBlockAtCurrentPosition(world, fence, 0, 0, 13, 0, b);
	placeBlockAtCurrentPosition(world, fence, 0, 12, 13, 0, b);
	for (int_t i = 3; i <= 9; i += 2)
	{
		fillWithBlocks(world, b, 1, 7, i, 1, 8, i, fence, fence, false);
		fillWithBlocks(world, b, 11, 7, i, 11, 8, i, fence, fence, false);
	}

	fillWithBlocks(world, b, 4, 2, 0, 8, 2, 12, brick, brick, false);
	fillWithBlocks(world, b, 0, 2, 4, 12, 2, 8, brick, brick, false);
	fillWithBlocks(world, b, 4, 0, 0, 8, 1, 3, brick, brick, false);
	fillWithBlocks(world, b, 4, 0, 9, 8, 1, 12, brick, brick, false);
	fillWithBlocks(world, b, 0, 0, 4, 3, 1, 8, brick, brick, false);
	fillWithBlocks(world, b, 9, 0, 4, 12, 1, 8, brick, brick, false);
	for (int_t x = 4; x <= 8; ++x)
	{
		for (int_t z = 0; z <= 2; ++z)
		{
			fillCurrentPositionBlocksDownwards(world, brick, 0, x, -1, z, b);
			fillCurrentPositionBlocksDownwards(world, brick, 0, x, -1, 12 - z, b);
		}
	}
	for (int_t x = 0; x <= 2; ++x)
	{
		for (int_t z = 4; z <= 8; ++z)
		{
			fillCurrentPositionBlocksDownwards(world, brick, 0, x, -1, z, b);
			fillCurrentPositionBlocksDownwards(world, brick, 0, 12 - x, -1, z, b);
		}
	}

	fillWithBlocks(world, b, 5, 5, 5, 7, 5, 7, brick, brick, false);
	fillWithBlocks(world, b, 6, 1, 6, 6, 4, 6, 0, 0, false);
	placeBlockAtCurrentPosition(world, brick, 0, 6, 0, 6, b);
	placeBlockAtCurrentPosition(world, Block::lavaMoving->blockID, 0, 6, 5, 6, b);
	const int_t worldX = getXWithOffset(6, 6);
	const int_t worldY = getYWithOffset(5);
	const int_t worldZ = getZWithOffset(6, 6);
	if (b.isVecInside(worldX, worldY, worldZ))
	{
		world->scheduledUpdatesAreImmediate = true;
		Block::blocksList[Block::lavaMoving->blockID]->updateTick(world, worldX, worldY, worldZ, random);
		world->scheduledUpdatesAreImmediate = false;
	}
	return true;
}
