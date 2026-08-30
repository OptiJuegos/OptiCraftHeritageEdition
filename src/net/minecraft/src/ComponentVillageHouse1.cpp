#include "ComponentVillageHouse1.h"

#include "Block.h"
#include "StructureBoundingBox.h"
#include "World.h"
#include "java/Random.h"

ComponentVillageHouse1::ComponentVillageHouse1(
	int_t type, Random &, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: ComponentVillage(type), averageGroundLevel(-1)
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
}

void ComponentVillageHouse1::buildComponent(
	StructureComponent *, std::vector<StructureComponent *> &, Random &)
{
}

ComponentVillageHouse1 *ComponentVillageHouse1::findValidPlacement(
	const std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t type)
{
	auto bounds = std::make_unique<StructureBoundingBox>(
		StructureBoundingBox::getComponentToAddBoundingBox(x, y, z, 0, 0, 0, 9, 9, 6, direction));
	if (!canVillageGoDeeper(bounds.get()) || StructureComponent::findIntersecting(components, *bounds) != nullptr)
		return nullptr;
	return new ComponentVillageHouse1(type, random, std::move(bounds), direction);
}

bool ComponentVillageHouse1::addComponentParts(
	World *world, Random &random, const StructureBoundingBox &chunkBounds)
{
	if (averageGroundLevel < 0)
	{
		averageGroundLevel = getAverageGroundLevel(world, chunkBounds);
		if (averageGroundLevel < 0)
			return true;
		boundingBox->offset(0, averageGroundLevel - boundingBox->maxY + 8, 0);
	}

	fillWithBlocks(world, chunkBounds, 1, 1, 1, 7, 5, 4, 0, 0, false);
	fillWithBlocks(world, chunkBounds, 0, 0, 0, 8, 0, 5, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 5, 0, 8, 5, 5, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 6, 1, 8, 6, 4, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 7, 2, 8, 7, 3, Block::cobblestone->blockID, Block::cobblestone->blockID, false);

	const int_t stairMetaA = getMetadataWithOffset(Block::stairCompactPlanks->blockID, 3);
	const int_t stairMetaB = getMetadataWithOffset(Block::stairCompactPlanks->blockID, 2);
	for (int_t roofOffset = -1; roofOffset <= 2; ++roofOffset)
	{
		for (int_t x = 0; x <= 8; ++x)
		{
			placeBlockAtCurrentPosition(world, Block::stairCompactPlanks->blockID, stairMetaA,
			                            x, 6 + roofOffset, roofOffset, chunkBounds);
			placeBlockAtCurrentPosition(world, Block::stairCompactPlanks->blockID, stairMetaB,
			                            x, 6 + roofOffset, 5 - roofOffset, chunkBounds);
		}
	}

	fillWithBlocks(world, chunkBounds, 0, 1, 0, 0, 1, 5, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 1, 5, 8, 1, 5, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 8, 1, 0, 8, 1, 4, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 2, 1, 0, 7, 1, 0, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 2, 0, 0, 4, 0, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 2, 5, 0, 4, 5, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 8, 2, 5, 8, 4, 5, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 8, 2, 0, 8, 4, 0, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 2, 1, 0, 4, 4, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 2, 5, 7, 4, 5, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 8, 2, 1, 8, 4, 4, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 2, 0, 7, 4, 0, Block::planks->blockID, Block::planks->blockID, false);

	for (int_t x = 4; x <= 6; ++x)
	{
		placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, x, 2, 0, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, x, 3, 0, chunkBounds);
	}
	for (int_t z = 2; z <= 3; ++z)
	{
		placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 0, 2, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 0, 3, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 8, 2, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 8, 3, z, chunkBounds);
	}
	for (int_t x : {2, 3, 5, 6})
		placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, x, 2, 5, chunkBounds);

	fillWithBlocks(world, chunkBounds, 1, 4, 1, 7, 4, 1, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 4, 4, 7, 4, 4, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 3, 4, 7, 3, 4, Block::bookShelf->blockID, Block::bookShelf->blockID, false);
	placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 7, 1, 4, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stairCompactPlanks->blockID,
		getMetadataWithOffset(Block::stairCompactPlanks->blockID, 0), 7, 1, 3, chunkBounds);
	const int_t benchStairMeta = getMetadataWithOffset(Block::stairCompactPlanks->blockID, 3);
	for (int_t x = 3; x <= 6; ++x)
		placeBlockAtCurrentPosition(world, Block::stairCompactPlanks->blockID, benchStairMeta, x, 1, 4, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, 6, 1, 3, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::pressurePlatePlanks->blockID, 0, 6, 2, 3, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, 4, 1, 3, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::pressurePlatePlanks->blockID, 0, 4, 2, 3, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::workbench->blockID, 0, 7, 1, 1, chunkBounds);

	placeBlockAtCurrentPosition(world, 0, 0, 1, 1, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, 0, 0, 1, 2, 0, chunkBounds);
	placeDoorAtCurrentPosition(world, chunkBounds, random, 1, 1, 0,
	                           getMetadataWithOffset(Block::doorWood->blockID, 1));
	if (getBlockIdAtCurrentPosition(world, 1, 0, -1, chunkBounds) == 0 &&
	    getBlockIdAtCurrentPosition(world, 1, -1, -1, chunkBounds) != 0)
	{
		placeBlockAtCurrentPosition(world, Block::stairCompactCobblestone->blockID,
			getMetadataWithOffset(Block::stairCompactCobblestone->blockID, 3), 1, 0, -1, chunkBounds);
	}

	for (int_t z = 0; z < 6; ++z)
	{
		for (int_t x = 0; x < 9; ++x)
		{
			clearCurrentPositionBlocksUpwards(world, x, 9, z, chunkBounds);
			fillCurrentPositionBlocksDownwards(world, Block::cobblestone->blockID, 0, x, -1, z, chunkBounds);
		}
	}
	spawnVillagers(world, chunkBounds, 2, 1, 2, 1);
	return true;
}

int_t ComponentVillageHouse1::getVillagerType(int_t) const
{
	return 1;
}
