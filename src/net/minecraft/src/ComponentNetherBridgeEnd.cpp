#include "ComponentNetherBridgeEnd.h"

#include <memory>

#include "Block.h"
#include "StructureBoundingBox.h"
#include "World.h"
#include "java/Random.h"

ComponentNetherBridgeEnd::ComponentNetherBridgeEnd(
	int_t type, Random &random, const StructureBoundingBox &bounds, int_t direction)
	: ComponentNetherBridgePiece(type), fillSeed(random.nextInt())
{
	coordBaseMode = direction;
	boundingBox = std::make_unique<StructureBoundingBox>(bounds);
}

void ComponentNetherBridgeEnd::buildComponent(StructureComponent *,
                                               std::vector<StructureComponent *> &,
                                               Random &)
{
}

ComponentNetherBridgeEnd *ComponentNetherBridgeEnd::createValidComponent(
	std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t depth)
{
	const StructureBoundingBox bounds = StructureBoundingBox::getComponentToAddBoundingBox(
		x, y, z, -1, -3, 0, 5, 10, 8, direction);
	return isAboveGround(&bounds) && StructureComponent::findIntersecting(components, bounds) == nullptr
		? new ComponentNetherBridgeEnd(depth, random, bounds, direction)
		: nullptr;
}

bool ComponentNetherBridgeEnd::addComponentParts(
	World *world, Random &, const StructureBoundingBox &chunkBounds)
{
	Random fillRandom(static_cast<long_t>(fillSeed));
	for (int_t x = 0; x <= 4; ++x)
	{
		for (int_t y = 3; y <= 4; ++y)
		{
			const int_t length = fillRandom.nextInt(8);
			fillWithBlocks(world, chunkBounds, x, y, 0, x, y, length,
			               Block::netherBrick->blockID, Block::netherBrick->blockID, false);
		}
	}

	int_t length = fillRandom.nextInt(8);
	fillWithBlocks(world, chunkBounds, 0, 5, 0, 0, 5, length,
	               Block::netherBrick->blockID, Block::netherBrick->blockID, false);
	length = fillRandom.nextInt(8);
	fillWithBlocks(world, chunkBounds, 4, 5, 0, 4, 5, length,
	               Block::netherBrick->blockID, Block::netherBrick->blockID, false);

	for (int_t x = 0; x <= 4; ++x)
	{
		length = fillRandom.nextInt(5);
		fillWithBlocks(world, chunkBounds, x, 2, 0, x, 2, length,
		               Block::netherBrick->blockID, Block::netherBrick->blockID, false);
	}

	for (int_t x = 0; x <= 4; ++x)
	{
		for (int_t y = 0; y <= 1; ++y)
		{
			length = fillRandom.nextInt(3);
			fillWithBlocks(world, chunkBounds, x, y, 0, x, y, length,
			               Block::netherBrick->blockID, Block::netherBrick->blockID, false);
		}
	}
	return true;
}
