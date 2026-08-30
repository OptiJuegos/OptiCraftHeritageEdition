#include "ComponentNetherBridgeCorridor3.h"

#include <algorithm>
#include <memory>

#include "Block.h"
#include "ComponentNetherBridgeStartPiece.h"
#include "StructureBoundingBox.h"
#include "World.h"
#include "java/Random.h"

ComponentNetherBridgeCorridor3::ComponentNetherBridgeCorridor3(
	int_t type, Random &, const StructureBoundingBox &bounds, int_t direction)
	: ComponentNetherBridgePiece(type)
{
	coordBaseMode = direction;
	boundingBox = std::make_unique<StructureBoundingBox>(bounds);
}

void ComponentNetherBridgeCorridor3::buildComponent(
	StructureComponent *parent, std::vector<StructureComponent *> &components, Random &random)
{
	getNextComponentNormal(static_cast<ComponentNetherBridgeStartPiece *>(parent), components, random, 1, 0, true);
}

ComponentNetherBridgeCorridor3 *ComponentNetherBridgeCorridor3::createValidComponent(
	std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t depth)
{
	const StructureBoundingBox bounds = StructureBoundingBox::getComponentToAddBoundingBox(
		x, y, z, -1, -7, 0, 5, 14, 10, direction);
	return isAboveGround(&bounds) && StructureComponent::findIntersecting(components, bounds) == nullptr
		? new ComponentNetherBridgeCorridor3(depth, random, bounds, direction)
		: nullptr;
}

bool ComponentNetherBridgeCorridor3::addComponentParts(
	World *world, Random &, const StructureBoundingBox &b)
{
	const int_t brick = Block::netherBrick->blockID;
	const int_t fence = Block::netherFence->blockID;
	const int_t stair = Block::stairsNetherBrick->blockID;
	const int_t stairMeta = getMetadataWithOffset(stair, 2);

	for (int_t z = 0; z <= 9; ++z)
	{
		const int_t floorY = std::max(1, 7 - z);
		const int_t roofY = std::min(std::max(floorY + 5, 14 - z), 13);
		fillWithBlocks(world, b, 0, 0, z, 4, floorY, z, brick, brick, false);
		fillWithBlocks(world, b, 1, floorY + 1, z, 3, roofY - 1, z, 0, 0, false);
		if (z <= 6)
		{
			placeBlockAtCurrentPosition(world, stair, stairMeta, 1, floorY + 1, z, b);
			placeBlockAtCurrentPosition(world, stair, stairMeta, 2, floorY + 1, z, b);
			placeBlockAtCurrentPosition(world, stair, stairMeta, 3, floorY + 1, z, b);
		}
		fillWithBlocks(world, b, 0, roofY, z, 4, roofY, z, brick, brick, false);
		fillWithBlocks(world, b, 0, floorY + 1, z, 0, roofY - 1, z, brick, brick, false);
		fillWithBlocks(world, b, 4, floorY + 1, z, 4, roofY - 1, z, brick, brick, false);
		if ((z & 1) == 0)
		{
			fillWithBlocks(world, b, 0, floorY + 2, z, 0, floorY + 3, z, fence, fence, false);
			fillWithBlocks(world, b, 4, floorY + 2, z, 4, floorY + 3, z, fence, fence, false);
		}
		for (int_t x = 0; x <= 4; ++x)
			fillCurrentPositionBlocksDownwards(world, brick, 0, x, -1, z, b);
	}
	return true;
}
