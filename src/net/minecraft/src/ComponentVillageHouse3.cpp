#include "ComponentVillageHouse3.h"

#include "Block.h"
#include "StructureBoundingBox.h"
#include "World.h"
#include "java/Random.h"

ComponentVillageHouse3::ComponentVillageHouse3(
	int_t type, Random &, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: ComponentVillage(type), averageGroundLevel(-1)
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
}

void ComponentVillageHouse3::buildComponent(
	StructureComponent *, std::vector<StructureComponent *> &, Random &)
{
}

ComponentVillageHouse3 *ComponentVillageHouse3::findValidPlacement(
	const std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t type)
{
	auto bounds = std::make_unique<StructureBoundingBox>(
		StructureBoundingBox::getComponentToAddBoundingBox(x, y, z, 0, 0, 0, 9, 7, 12, direction));
	if (!canVillageGoDeeper(bounds.get()) || StructureComponent::findIntersecting(components, *bounds) != nullptr)
		return nullptr;
	return new ComponentVillageHouse3(type, random, std::move(bounds), direction);
}

bool ComponentVillageHouse3::addComponentParts(
	World *world, Random &random, const StructureBoundingBox &chunkBounds)
{
	if (averageGroundLevel < 0)
	{
		averageGroundLevel = getAverageGroundLevel(world, chunkBounds);
		if (averageGroundLevel < 0)
			return true;
		boundingBox->offset(0, averageGroundLevel - boundingBox->maxY + 6, 0);
	}

	fillWithBlocks(world, chunkBounds, 1, 1, 1, 7, 4, 4, 0, 0, false);
	fillWithBlocks(world, chunkBounds, 2, 1, 6, 8, 4, 10, 0, 0, false);
	fillWithBlocks(world, chunkBounds, 2, 0, 5, 8, 0, 10, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 0, 1, 7, 0, 4, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 0, 0, 0, 3, 5, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 8, 0, 0, 8, 3, 10, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 0, 0, 7, 2, 0, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 0, 5, 2, 1, 5, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 2, 0, 6, 2, 3, 10, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 3, 0, 10, 7, 3, 10, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 2, 0, 7, 3, 0, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 2, 5, 2, 3, 5, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 4, 1, 8, 4, 1, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 4, 4, 3, 4, 4, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 5, 2, 8, 5, 3, Block::planks->blockID, Block::planks->blockID, false);
	placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 0, 4, 2, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 0, 4, 3, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 8, 4, 2, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 8, 4, 3, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 8, 4, 4, chunkBounds);

	const int_t stairA = getMetadataWithOffset(Block::stairCompactPlanks->blockID, 3);
	const int_t stairB = getMetadataWithOffset(Block::stairCompactPlanks->blockID, 2);
	for (int_t dz = -1; dz <= 2; ++dz)
	{
		for (int_t x = 0; x <= 8; ++x)
		{
			placeBlockAtCurrentPosition(world, Block::stairCompactPlanks->blockID, stairA, x, 4 + dz, dz, chunkBounds);
			if ((dz > -1 || x <= 1) && (dz > 0 || x <= 3) && (dz > 1 || x <= 4 || x >= 6))
				placeBlockAtCurrentPosition(world, Block::stairCompactPlanks->blockID, stairB, x, 4 + dz, 5 - dz, chunkBounds);
		}
	}

	fillWithBlocks(world, chunkBounds, 3, 4, 5, 3, 4, 10, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 7, 4, 2, 7, 4, 10, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 4, 5, 4, 4, 5, 10, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 6, 5, 4, 6, 5, 10, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 5, 6, 3, 5, 6, 10, Block::planks->blockID, Block::planks->blockID, false);

	const int_t stairForward = getMetadataWithOffset(Block::stairCompactPlanks->blockID, 0);
	for (int_t x = 4; x >= 1; --x)
	{
		placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, x, 2 + x, 7 - x, chunkBounds);
		for (int_t z = 8 - x; z <= 10; ++z)
			placeBlockAtCurrentPosition(world, Block::stairCompactPlanks->blockID, stairForward, x, 2 + x, z, chunkBounds);
	}

	const int_t stairBack = getMetadataWithOffset(Block::stairCompactPlanks->blockID, 1);
	placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 6, 6, 3, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 7, 5, 4, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stairCompactPlanks->blockID, stairBack, 6, 6, 4, chunkBounds);
	for (int_t x = 6; x <= 8; ++x)
		for (int_t z = 5; z <= 10; ++z)
			placeBlockAtCurrentPosition(world, Block::stairCompactPlanks->blockID, stairBack, x, 12 - x, z, chunkBounds);

	placeBlockAtCurrentPosition(world, Block::wood->blockID, 0, 0, 2, 1, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::wood->blockID, 0, 0, 2, 4, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 0, 2, 2, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 0, 2, 3, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::wood->blockID, 0, 4, 2, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 5, 2, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::wood->blockID, 0, 6, 2, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::wood->blockID, 0, 8, 2, 1, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 8, 2, 2, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 8, 2, 3, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::wood->blockID, 0, 8, 2, 4, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 8, 2, 5, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::wood->blockID, 0, 8, 2, 6, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 8, 2, 7, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 8, 2, 8, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::wood->blockID, 0, 8, 2, 9, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::wood->blockID, 0, 2, 2, 6, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 2, 2, 7, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 2, 2, 8, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::wood->blockID, 0, 2, 2, 9, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::wood->blockID, 0, 4, 4, 10, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 5, 4, 10, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::wood->blockID, 0, 6, 4, 10, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 5, 5, 10, chunkBounds);

	placeBlockAtCurrentPosition(world, 0, 0, 2, 1, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, 0, 0, 2, 2, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 0, 2, 3, 1, chunkBounds);
	placeDoorAtCurrentPosition(world, chunkBounds, random, 2, 1, 0,
		getMetadataWithOffset(Block::doorWood->blockID, 1));
	fillWithBlocks(world, chunkBounds, 1, 0, -1, 3, 2, -1, 0, 0, false);
	if (getBlockIdAtCurrentPosition(world, 2, 0, -1, chunkBounds) == 0 &&
	    getBlockIdAtCurrentPosition(world, 2, -1, -1, chunkBounds) != 0)
	{
		placeBlockAtCurrentPosition(world, Block::stairCompactCobblestone->blockID,
			getMetadataWithOffset(Block::stairCompactCobblestone->blockID, 3), 2, 0, -1, chunkBounds);
	}

	for (int_t z = 0; z < 5; ++z)
		for (int_t x = 0; x < 9; ++x)
		{
			clearCurrentPositionBlocksUpwards(world, x, 7, z, chunkBounds);
			fillCurrentPositionBlocksDownwards(world, Block::cobblestone->blockID, 0, x, -1, z, chunkBounds);
		}
	for (int_t z = 5; z < 11; ++z)
		for (int_t x = 2; x < 9; ++x)
		{
			clearCurrentPositionBlocksUpwards(world, x, 7, z, chunkBounds);
			fillCurrentPositionBlocksDownwards(world, Block::cobblestone->blockID, 0, x, -1, z, chunkBounds);
		}

	spawnVillagers(world, chunkBounds, 4, 1, 2, 2);
	return true;
}
