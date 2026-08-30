#include "ComponentNetherBridgeThrone.h"

#include <memory>

#include "Block.h"
#include "StructureBoundingBox.h"
#include "TileEntity.h"
#include "TileEntityMobSpawner.h"
#include "World.h"
#include "java/Random.h"

ComponentNetherBridgeThrone::ComponentNetherBridgeThrone(
	int_t type, Random &, const StructureBoundingBox &bounds, int_t direction)
	: ComponentNetherBridgePiece(type), hasSpawner(false)
{
	coordBaseMode = direction;
	boundingBox = std::make_unique<StructureBoundingBox>(bounds);
}

void ComponentNetherBridgeThrone::buildComponent(
	StructureComponent *, std::vector<StructureComponent *> &, Random &)
{
}

ComponentNetherBridgeThrone *ComponentNetherBridgeThrone::createValidComponent(
	std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t depth)
{
	const StructureBoundingBox bounds = StructureBoundingBox::getComponentToAddBoundingBox(
		x, y, z, -2, 0, 0, 7, 8, 9, direction);
	return isAboveGround(&bounds) && StructureComponent::findIntersecting(components, bounds) == nullptr
		? new ComponentNetherBridgeThrone(depth, random, bounds, direction)
		: nullptr;
}

bool ComponentNetherBridgeThrone::addComponentParts(
	World *world, Random &, const StructureBoundingBox &b)
{
	const int_t brick = Block::netherBrick->blockID;
	const int_t fence = Block::netherFence->blockID;
	fillWithBlocks(world, b, 0, 2, 0, 6, 7, 7, 0, 0, false);
	fillWithBlocks(world, b, 1, 0, 0, 5, 1, 7, brick, brick, false);
	fillWithBlocks(world, b, 1, 2, 1, 5, 2, 7, brick, brick, false);
	fillWithBlocks(world, b, 1, 3, 2, 5, 3, 7, brick, brick, false);
	fillWithBlocks(world, b, 1, 4, 3, 5, 4, 7, brick, brick, false);
	fillWithBlocks(world, b, 1, 2, 0, 1, 4, 2, brick, brick, false);
	fillWithBlocks(world, b, 5, 2, 0, 5, 4, 2, brick, brick, false);
	fillWithBlocks(world, b, 1, 5, 2, 1, 5, 3, brick, brick, false);
	fillWithBlocks(world, b, 5, 5, 2, 5, 5, 3, brick, brick, false);
	fillWithBlocks(world, b, 0, 5, 3, 0, 5, 8, brick, brick, false);
	fillWithBlocks(world, b, 6, 5, 3, 6, 5, 8, brick, brick, false);
	fillWithBlocks(world, b, 1, 5, 8, 5, 5, 8, brick, brick, false);
	placeBlockAtCurrentPosition(world, fence, 0, 1, 6, 3, b);
	placeBlockAtCurrentPosition(world, fence, 0, 5, 6, 3, b);
	fillWithBlocks(world, b, 0, 6, 3, 0, 6, 8, fence, fence, false);
	fillWithBlocks(world, b, 6, 6, 3, 6, 6, 8, fence, fence, false);
	fillWithBlocks(world, b, 1, 6, 8, 5, 7, 8, fence, fence, false);
	fillWithBlocks(world, b, 2, 8, 8, 4, 8, 8, fence, fence, false);

	if (!hasSpawner)
	{
		const int_t y = getYWithOffset(5);
		const int_t x = getXWithOffset(3, 5);
		const int_t z = getZWithOffset(3, 5);
		if (b.isVecInside(x, y, z))
		{
			hasSpawner = true;
			world->setBlockWithNotify(x, y, z, Block::mobSpawner->blockID);
			auto *spawner = dynamic_cast<TileEntityMobSpawner *>(world->getBlockTileEntity(x, y, z));
			if (spawner != nullptr)
				spawner->setMobID("Blaze");
		}
	}

	for (int_t x = 0; x <= 6; ++x)
		for (int_t z = 0; z <= 6; ++z)
			fillCurrentPositionBlocksDownwards(world, brick, 0, x, -1, z, b);
	return true;
}
