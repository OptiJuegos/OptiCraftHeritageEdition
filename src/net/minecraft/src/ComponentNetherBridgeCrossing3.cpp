#include "ComponentNetherBridgeCrossing3.h"

#include <memory>

#include "Block.h"
#include "ComponentNetherBridgeStartPiece.h"
#include "StructureBoundingBox.h"
#include "World.h"
#include "java/Random.h"

ComponentNetherBridgeCrossing3::ComponentNetherBridgeCrossing3(
	int_t type, Random &, const StructureBoundingBox &bounds, int_t direction)
	: ComponentNetherBridgePiece(type)
{
	coordBaseMode = direction;
	boundingBox = std::make_unique<StructureBoundingBox>(bounds);
}

ComponentNetherBridgeCrossing3::ComponentNetherBridgeCrossing3(Random &random, int_t x, int_t z)
	: ComponentNetherBridgePiece(0)
{
	coordBaseMode = random.nextInt(4);
	boundingBox = std::make_unique<StructureBoundingBox>(x, 64, z, x + 18, 73, z + 18);
}

void ComponentNetherBridgeCrossing3::buildComponent(
	StructureComponent *parent, std::vector<StructureComponent *> &components, Random &random)
{
	auto *start = static_cast<ComponentNetherBridgeStartPiece *>(parent);
	getNextComponentNormal(start, components, random, 8, 3, false);
	getNextComponentX(start, components, random, 3, 8, false);
	getNextComponentZ(start, components, random, 3, 8, false);
}

ComponentNetherBridgeCrossing3 *ComponentNetherBridgeCrossing3::createValidComponent(
	std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t depth)
{
	const StructureBoundingBox bounds = StructureBoundingBox::getComponentToAddBoundingBox(
		x, y, z, -8, -3, 0, 19, 10, 19, direction);
	return isAboveGround(&bounds) && StructureComponent::findIntersecting(components, bounds) == nullptr
		? new ComponentNetherBridgeCrossing3(depth, random, bounds, direction)
		: nullptr;
}

bool ComponentNetherBridgeCrossing3::addComponentParts(
	World *world, Random &, const StructureBoundingBox &b)
{
	const int_t brick = Block::netherBrick->blockID;
	fillWithBlocks(world, b, 7, 3, 0, 11, 4, 18, brick, brick, false);
	fillWithBlocks(world, b, 0, 3, 7, 18, 4, 11, brick, brick, false);
	fillWithBlocks(world, b, 8, 5, 0, 10, 7, 18, 0, 0, false);
	fillWithBlocks(world, b, 0, 5, 8, 18, 7, 10, 0, 0, false);
	fillWithBlocks(world, b, 7, 5, 0, 7, 5, 7, brick, brick, false);
	fillWithBlocks(world, b, 7, 5, 11, 7, 5, 18, brick, brick, false);
	fillWithBlocks(world, b, 11, 5, 0, 11, 5, 7, brick, brick, false);
	fillWithBlocks(world, b, 11, 5, 11, 11, 5, 18, brick, brick, false);
	fillWithBlocks(world, b, 0, 5, 7, 7, 5, 7, brick, brick, false);
	fillWithBlocks(world, b, 11, 5, 7, 18, 5, 7, brick, brick, false);
	fillWithBlocks(world, b, 0, 5, 11, 7, 5, 11, brick, brick, false);
	fillWithBlocks(world, b, 11, 5, 11, 18, 5, 11, brick, brick, false);
	fillWithBlocks(world, b, 7, 2, 0, 11, 2, 5, brick, brick, false);
	fillWithBlocks(world, b, 7, 2, 13, 11, 2, 18, brick, brick, false);
	fillWithBlocks(world, b, 7, 0, 0, 11, 1, 3, brick, brick, false);
	fillWithBlocks(world, b, 7, 0, 15, 11, 1, 18, brick, brick, false);

	for (int_t x = 7; x <= 11; ++x)
	{
		for (int_t z = 0; z <= 2; ++z)
		{
			fillCurrentPositionBlocksDownwards(world, brick, 0, x, -1, z, b);
			fillCurrentPositionBlocksDownwards(world, brick, 0, x, -1, 18 - z, b);
		}
	}

	fillWithBlocks(world, b, 0, 2, 7, 5, 2, 11, brick, brick, false);
	fillWithBlocks(world, b, 13, 2, 7, 18, 2, 11, brick, brick, false);
	fillWithBlocks(world, b, 0, 0, 7, 3, 1, 11, brick, brick, false);
	fillWithBlocks(world, b, 15, 0, 7, 18, 1, 11, brick, brick, false);

	for (int_t x = 0; x <= 2; ++x)
	{
		for (int_t z = 7; z <= 11; ++z)
		{
			fillCurrentPositionBlocksDownwards(world, brick, 0, x, -1, z, b);
			fillCurrentPositionBlocksDownwards(world, brick, 0, 18 - x, -1, z, b);
		}
	}
	return true;
}
