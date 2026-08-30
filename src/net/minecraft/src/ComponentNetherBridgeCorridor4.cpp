#include "ComponentNetherBridgeCorridor4.h"

#include <memory>

#include "Block.h"
#include "ComponentNetherBridgeStartPiece.h"
#include "StructureBoundingBox.h"
#include "World.h"
#include "java/Random.h"

ComponentNetherBridgeCorridor4::ComponentNetherBridgeCorridor4(
	int_t type, Random &, const StructureBoundingBox &bounds, int_t direction)
	: ComponentNetherBridgePiece(type)
{
	coordBaseMode = direction;
	boundingBox = std::make_unique<StructureBoundingBox>(bounds);
}

void ComponentNetherBridgeCorridor4::buildComponent(
	StructureComponent *parent, std::vector<StructureComponent *> &components, Random &random)
{
	int_t offset = 1;
	if (coordBaseMode == 1 || coordBaseMode == 2)
		offset = 5;
	auto *start = static_cast<ComponentNetherBridgeStartPiece *>(parent);
	getNextComponentX(start, components, random, 0, offset, random.nextInt(8) > 0);
	getNextComponentZ(start, components, random, 0, offset, random.nextInt(8) > 0);
}

ComponentNetherBridgeCorridor4 *ComponentNetherBridgeCorridor4::createValidComponent(
	std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t depth)
{
	const StructureBoundingBox bounds = StructureBoundingBox::getComponentToAddBoundingBox(
		x, y, z, -3, 0, 0, 9, 7, 9, direction);
	return isAboveGround(&bounds) && StructureComponent::findIntersecting(components, bounds) == nullptr
		? new ComponentNetherBridgeCorridor4(depth, random, bounds, direction)
		: nullptr;
}

bool ComponentNetherBridgeCorridor4::addComponentParts(
	World *world, Random &, const StructureBoundingBox &b)
{
	const int_t brick = Block::netherBrick->blockID;
	const int_t fence = Block::netherFence->blockID;
	fillWithBlocks(world, b, 0, 0, 0, 8, 1, 8, brick, brick, false);
	fillWithBlocks(world, b, 0, 2, 0, 8, 5, 8, 0, 0, false);
	fillWithBlocks(world, b, 0, 6, 0, 8, 6, 5, brick, brick, false);
	fillWithBlocks(world, b, 0, 2, 0, 2, 5, 0, brick, brick, false);
	fillWithBlocks(world, b, 6, 2, 0, 8, 5, 0, brick, brick, false);
	fillWithBlocks(world, b, 1, 3, 0, 1, 4, 0, fence, fence, false);
	fillWithBlocks(world, b, 7, 3, 0, 7, 4, 0, fence, fence, false);
	fillWithBlocks(world, b, 0, 2, 4, 8, 2, 8, brick, brick, false);
	fillWithBlocks(world, b, 1, 1, 4, 2, 2, 4, 0, 0, false);
	fillWithBlocks(world, b, 6, 1, 4, 7, 2, 4, 0, 0, false);
	fillWithBlocks(world, b, 0, 3, 8, 8, 3, 8, fence, fence, false);
	fillWithBlocks(world, b, 0, 3, 6, 0, 3, 7, fence, fence, false);
	fillWithBlocks(world, b, 8, 3, 6, 8, 3, 7, fence, fence, false);
	fillWithBlocks(world, b, 0, 3, 4, 0, 5, 5, brick, brick, false);
	fillWithBlocks(world, b, 8, 3, 4, 8, 5, 5, brick, brick, false);
	fillWithBlocks(world, b, 1, 3, 5, 2, 5, 5, brick, brick, false);
	fillWithBlocks(world, b, 6, 3, 5, 7, 5, 5, brick, brick, false);
	fillWithBlocks(world, b, 1, 4, 5, 1, 5, 5, fence, fence, false);
	fillWithBlocks(world, b, 7, 4, 5, 7, 5, 5, fence, fence, false);
	for (int_t z = 0; z <= 5; ++z)
		for (int_t x = 0; x <= 8; ++x)
			fillCurrentPositionBlocksDownwards(world, brick, 0, x, -1, z, b);
	return true;
}
