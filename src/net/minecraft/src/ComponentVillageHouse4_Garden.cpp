#include "ComponentVillageHouse4_Garden.h"

#include "Block.h"
#include "StructureBoundingBox.h"
#include "World.h"
#include "java/Random.h"

ComponentVillageHouse4_Garden::ComponentVillageHouse4_Garden(
	int_t type, Random &random, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: ComponentVillage(type), averageGroundLevel(-1), isRoofAccessible(random.nextBoolean())
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
}

void ComponentVillageHouse4_Garden::buildComponent(
	StructureComponent *, std::vector<StructureComponent *> &, Random &)
{
}

ComponentVillageHouse4_Garden *ComponentVillageHouse4_Garden::findValidPlacement(
	const std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t type)
{
	auto bounds = std::make_unique<StructureBoundingBox>(
		StructureBoundingBox::getComponentToAddBoundingBox(x, y, z, 0, 0, 0, 5, 6, 5, direction));
	if (StructureComponent::findIntersecting(components, *bounds) != nullptr)
		return nullptr;
	return new ComponentVillageHouse4_Garden(type, random, std::move(bounds), direction);
}

bool ComponentVillageHouse4_Garden::addComponentParts(
	World *world, Random &random, const StructureBoundingBox &chunkBounds)
{
	if (averageGroundLevel < 0)
	{
		averageGroundLevel = getAverageGroundLevel(world, chunkBounds);
		if (averageGroundLevel < 0)
			return true;
		boundingBox->offset(0, averageGroundLevel - boundingBox->maxY + 5, 0);
	}

	fillWithBlocks(world, chunkBounds, 0, 0, 0, 4, 0, 4, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 4, 0, 4, 4, 4, Block::wood->blockID, Block::wood->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 4, 1, 3, 4, 3, Block::planks->blockID, Block::planks->blockID, false);
	for (int_t x : {0, 4})
	{
		for (int_t z : {0, 4})
		{
			for (int_t y = 1; y <= 3; ++y)
				placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, x, y, z, chunkBounds);
		}
	}
	fillWithBlocks(world, chunkBounds, 0, 1, 1, 0, 3, 3, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 4, 1, 1, 4, 3, 3, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 1, 4, 3, 3, 4, Block::planks->blockID, Block::planks->blockID, false);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 0, 2, 2, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 2, 2, 4, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 4, 2, 2, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 1, 1, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 1, 2, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 1, 3, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 2, 3, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 3, 3, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 3, 2, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 3, 1, 0, chunkBounds);
	if (getBlockIdAtCurrentPosition(world, 2, 0, -1, chunkBounds) == 0 &&
	    getBlockIdAtCurrentPosition(world, 2, -1, -1, chunkBounds) != 0)
	{
		placeBlockAtCurrentPosition(world, Block::stairCompactCobblestone->blockID,
			getMetadataWithOffset(Block::stairCompactCobblestone->blockID, 3), 2, 0, -1, chunkBounds);
	}
	fillWithBlocks(world, chunkBounds, 1, 1, 1, 3, 3, 3, 0, 0, false);

	if (isRoofAccessible)
	{
		for (int_t x = 0; x <= 4; ++x)
		{
			placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, x, 5, 0, chunkBounds);
			placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, x, 5, 4, chunkBounds);
		}
		for (int_t z = 1; z <= 3; ++z)
		{
			placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, 0, 5, z, chunkBounds);
			placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, 4, 5, z, chunkBounds);
		}
		const int_t ladderMeta = getMetadataWithOffset(Block::ladder->blockID, 3);
		for (int_t y = 1; y <= 4; ++y)
			placeBlockAtCurrentPosition(world, Block::ladder->blockID, ladderMeta, 3, y, 3, chunkBounds);
	}

	placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 0, 2, 3, 1, chunkBounds);
	for (int_t z = 0; z < 5; ++z)
	{
		for (int_t x = 0; x < 5; ++x)
		{
			clearCurrentPositionBlocksUpwards(world, x, 6, z, chunkBounds);
			fillCurrentPositionBlocksDownwards(world, Block::cobblestone->blockID, 0, x, -1, z, chunkBounds);
		}
	}
	spawnVillagers(world, chunkBounds, 1, 1, 2, 1);
	return true;
}
