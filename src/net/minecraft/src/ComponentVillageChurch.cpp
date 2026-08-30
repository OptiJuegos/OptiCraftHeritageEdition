#include "ComponentVillageChurch.h"

#include "Block.h"
#include "StructureBoundingBox.h"
#include "World.h"
#include "java/Random.h"

ComponentVillageChurch::ComponentVillageChurch(
	int_t type, Random &, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: ComponentVillage(type), averageGroundLevel(-1)
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
}

void ComponentVillageChurch::buildComponent(
	StructureComponent *, std::vector<StructureComponent *> &, Random &)
{
}

ComponentVillageChurch *ComponentVillageChurch::findValidPlacement(
	const std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t type)
{
	auto bounds = std::make_unique<StructureBoundingBox>(
		StructureBoundingBox::getComponentToAddBoundingBox(x, y, z, 0, 0, 0, 5, 12, 9, direction));
	if (!canVillageGoDeeper(bounds.get()) || StructureComponent::findIntersecting(components, *bounds) != nullptr)
		return nullptr;
	return new ComponentVillageChurch(type, random, std::move(bounds), direction);
}

bool ComponentVillageChurch::addComponentParts(
	World *world, Random &random, const StructureBoundingBox &chunkBounds)
{
	if (averageGroundLevel < 0)
	{
		averageGroundLevel = getAverageGroundLevel(world, chunkBounds);
		if (averageGroundLevel < 0)
			return true;
		boundingBox->offset(0, averageGroundLevel - boundingBox->maxY + 11, 0);
	}

	fillWithBlocks(world, chunkBounds, 1, 1, 1, 3, 3, 7, 0, 0, false);
	fillWithBlocks(world, chunkBounds, 1, 5, 1, 3, 9, 3, 0, 0, false);
	fillWithBlocks(world, chunkBounds, 1, 0, 0, 3, 0, 8, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 1, 0, 3, 10, 0, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 1, 1, 0, 10, 3, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 4, 1, 1, 4, 10, 3, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 0, 4, 0, 4, 7, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 4, 0, 4, 4, 4, 7, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 1, 8, 3, 4, 8, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 5, 4, 3, 10, 4, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 5, 5, 3, 5, 7, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 9, 0, 4, 9, 4, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 4, 0, 4, 4, 4, Block::cobblestone->blockID, Block::cobblestone->blockID, false);

	placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 0, 11, 2, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 4, 11, 2, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 2, 11, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 2, 11, 4, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 1, 1, 6, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 1, 1, 7, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 2, 1, 7, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 3, 1, 6, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 3, 1, 7, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stairCompactCobblestone->blockID,
		getMetadataWithOffset(Block::stairCompactCobblestone->blockID, 3), 1, 1, 5, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stairCompactCobblestone->blockID,
		getMetadataWithOffset(Block::stairCompactCobblestone->blockID, 3), 2, 1, 6, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stairCompactCobblestone->blockID,
		getMetadataWithOffset(Block::stairCompactCobblestone->blockID, 3), 3, 1, 5, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stairCompactCobblestone->blockID,
		getMetadataWithOffset(Block::stairCompactCobblestone->blockID, 1), 1, 2, 7, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stairCompactCobblestone->blockID,
		getMetadataWithOffset(Block::stairCompactCobblestone->blockID, 0), 3, 2, 7, chunkBounds);

	for (int_t y : {2, 3})
	{
		placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 0, y, 2, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 4, y, 2, chunkBounds);
	}
	for (int_t y : {6, 7})
	{
		placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 0, y, 2, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 4, y, 2, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 2, y, 0, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 2, y, 4, chunkBounds);
	}
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 0, 3, 6, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 4, 3, 6, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 2, 3, 8, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 0, 2, 4, 7, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 0, 1, 4, 6, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 0, 3, 4, 6, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 0, 2, 4, 5, chunkBounds);

	const int_t ladderMeta = getMetadataWithOffset(Block::ladder->blockID, 4);
	for (int_t y = 1; y <= 9; ++y)
		placeBlockAtCurrentPosition(world, Block::ladder->blockID, ladderMeta, 3, y, 3, chunkBounds);

	placeBlockAtCurrentPosition(world, 0, 0, 2, 1, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, 0, 0, 2, 2, 0, chunkBounds);
	placeDoorAtCurrentPosition(world, chunkBounds, random, 2, 1, 0,
	                           getMetadataWithOffset(Block::doorWood->blockID, 1));
	if (getBlockIdAtCurrentPosition(world, 2, 0, -1, chunkBounds) == 0 &&
	    getBlockIdAtCurrentPosition(world, 2, -1, -1, chunkBounds) != 0)
	{
		placeBlockAtCurrentPosition(world, Block::stairCompactCobblestone->blockID,
			getMetadataWithOffset(Block::stairCompactCobblestone->blockID, 3), 2, 0, -1, chunkBounds);
	}

	for (int_t z = 0; z < 9; ++z)
	{
		for (int_t x = 0; x < 5; ++x)
		{
			clearCurrentPositionBlocksUpwards(world, x, 12, z, chunkBounds);
			fillCurrentPositionBlocksDownwards(world, Block::cobblestone->blockID, 0, x, -1, z, chunkBounds);
		}
	}
	spawnVillagers(world, chunkBounds, 2, 1, 2, 1);
	return true;
}

int_t ComponentVillageChurch::getVillagerType(int_t) const
{
	return 2;
}
