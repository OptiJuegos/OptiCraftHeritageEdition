#include "ComponentVillageHall.h"

#include "Block.h"
#include "StructureBoundingBox.h"
#include "World.h"
#include "java/Random.h"

ComponentVillageHall::ComponentVillageHall(
	int_t type, Random &, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: ComponentVillage(type), averageGroundLevel(-1)
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
}

void ComponentVillageHall::buildComponent(
	StructureComponent *, std::vector<StructureComponent *> &, Random &)
{
}

ComponentVillageHall *ComponentVillageHall::findValidPlacement(
	const std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t type)
{
	auto bounds = std::make_unique<StructureBoundingBox>(
		StructureBoundingBox::getComponentToAddBoundingBox(x, y, z, 0, 0, 0, 9, 7, 11, direction));
	if (!canVillageGoDeeper(bounds.get()) || StructureComponent::findIntersecting(components, *bounds) != nullptr)
		return nullptr;
	return new ComponentVillageHall(type, random, std::move(bounds), direction);
}

bool ComponentVillageHall::addComponentParts(
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
	fillWithBlocks(world, chunkBounds, 2, 0, 6, 8, 0, 10, Block::dirt->blockID, Block::dirt->blockID, false);
	placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 6, 0, 6, chunkBounds);
	fillWithBlocks(world, chunkBounds, 2, 1, 6, 2, 1, 10, Block::fence->blockID, Block::fence->blockID, false);
	fillWithBlocks(world, chunkBounds, 8, 1, 6, 8, 1, 10, Block::fence->blockID, Block::fence->blockID, false);
	fillWithBlocks(world, chunkBounds, 3, 1, 10, 7, 1, 10, Block::fence->blockID, Block::fence->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 0, 1, 7, 0, 4, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 0, 0, 0, 3, 5, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 8, 0, 0, 8, 3, 5, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 0, 0, 7, 1, 0, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 0, 5, 7, 1, 5, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 2, 0, 7, 3, 0, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 2, 5, 7, 3, 5, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 4, 1, 8, 4, 1, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 4, 4, 8, 4, 4, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 5, 2, 8, 5, 3, Block::planks->blockID, Block::planks->blockID, false);
	for (int_t x : {0, 8})
	{
		placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, x, 4, 2, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, x, 4, 3, chunkBounds);
	}

	const int_t stairA = getMetadataWithOffset(Block::stairCompactPlanks->blockID, 3);
	const int_t stairB = getMetadataWithOffset(Block::stairCompactPlanks->blockID, 2);
	for (int_t dz = -1; dz <= 2; ++dz)
	{
		for (int_t x = 0; x <= 8; ++x)
		{
			placeBlockAtCurrentPosition(world, Block::stairCompactPlanks->blockID, stairA, x, 4 + dz, dz, chunkBounds);
			placeBlockAtCurrentPosition(world, Block::stairCompactPlanks->blockID, stairB, x, 4 + dz, 5 - dz, chunkBounds);
		}
	}

	for (int_t x : {0, 8})
	{
		placeBlockAtCurrentPosition(world, Block::wood->blockID, 0, x, 2, 1, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::wood->blockID, 0, x, 2, 4, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, x, 2, 2, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, x, 2, 3, chunkBounds);
	}
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 2, 2, 5, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 3, 2, 5, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 5, 2, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 6, 2, 5, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, 2, 1, 3, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::pressurePlatePlanks->blockID, 0, 2, 2, 3, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 1, 1, 4, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stairCompactPlanks->blockID,
		getMetadataWithOffset(Block::stairCompactPlanks->blockID, 3), 2, 1, 4, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stairCompactPlanks->blockID,
		getMetadataWithOffset(Block::stairCompactPlanks->blockID, 1), 1, 1, 3, chunkBounds);
	fillWithBlocks(world, chunkBounds, 5, 0, 1, 7, 0, 3, Block::stairDouble->blockID, Block::stairDouble->blockID, false);
	placeBlockAtCurrentPosition(world, Block::stairDouble->blockID, 0, 6, 1, 1, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stairDouble->blockID, 0, 6, 1, 2, chunkBounds);

	placeBlockAtCurrentPosition(world, 0, 0, 2, 1, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, 0, 0, 2, 2, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 0, 2, 3, 1, chunkBounds);
	placeDoorAtCurrentPosition(world, chunkBounds, random, 2, 1, 0,
		getMetadataWithOffset(Block::doorWood->blockID, 1));
	if (getBlockIdAtCurrentPosition(world, 2, 0, -1, chunkBounds) == 0 &&
	    getBlockIdAtCurrentPosition(world, 2, -1, -1, chunkBounds) != 0)
	{
		placeBlockAtCurrentPosition(world, Block::stairCompactCobblestone->blockID,
			getMetadataWithOffset(Block::stairCompactCobblestone->blockID, 3), 2, 0, -1, chunkBounds);
	}

	placeBlockAtCurrentPosition(world, 0, 0, 6, 1, 5, chunkBounds);
	placeBlockAtCurrentPosition(world, 0, 0, 6, 2, 5, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 0, 6, 3, 4, chunkBounds);
	placeDoorAtCurrentPosition(world, chunkBounds, random, 6, 1, 5,
		getMetadataWithOffset(Block::doorWood->blockID, 1));

	for (int_t z = 0; z < 5; ++z)
	{
		for (int_t x = 0; x < 9; ++x)
		{
			clearCurrentPositionBlocksUpwards(world, x, 7, z, chunkBounds);
			fillCurrentPositionBlocksDownwards(world, Block::cobblestone->blockID, 0, x, -1, z, chunkBounds);
		}
	}
	spawnVillagers(world, chunkBounds, 4, 1, 2, 2);
	return true;
}

int_t ComponentVillageHall::getVillagerType(int_t index) const
{
	return index == 0 ? 4 : 0;
}
