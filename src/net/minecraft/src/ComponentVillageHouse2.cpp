#include "ComponentVillageHouse2.h"

#include <vector>

#include "Block.h"
#include "Item.h"
#include "StructureBoundingBox.h"
#include "StructurePieceTreasure.h"
#include "World.h"
#include "java/Random.h"

namespace
{
const std::vector<StructurePieceTreasure> &blacksmithChestLoot()
{
	static const std::vector<StructurePieceTreasure> loot = {
		{Item::diamond->shiftedIndex, 0, 1, 3, 3},
		{Item::ingotIron->shiftedIndex, 0, 1, 5, 10},
		{Item::ingotGold->shiftedIndex, 0, 1, 3, 5},
		{Item::bread->shiftedIndex, 0, 1, 3, 15},
		{Item::appleRed->shiftedIndex, 0, 1, 3, 15},
		{Item::pickaxeSteel->shiftedIndex, 0, 1, 1, 5},
		{Item::swordSteel->shiftedIndex, 0, 1, 1, 5},
		{Item::plateSteel->shiftedIndex, 0, 1, 1, 5},
		{Item::helmetSteel->shiftedIndex, 0, 1, 1, 5},
		{Item::legsSteel->shiftedIndex, 0, 1, 1, 5},
		{Item::bootsSteel->shiftedIndex, 0, 1, 1, 5},
		{Block::obsidian->blockID, 0, 3, 7, 5},
		{Block::sapling->blockID, 0, 3, 7, 5}
	};
	return loot;
}
}

ComponentVillageHouse2::ComponentVillageHouse2(
	int_t type, Random &, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: ComponentVillage(type), averageGroundLevel(-1), hasMadeChest(false)
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
}

void ComponentVillageHouse2::buildComponent(
	StructureComponent *, std::vector<StructureComponent *> &, Random &)
{
}

ComponentVillageHouse2 *ComponentVillageHouse2::findValidPlacement(
	const std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t type)
{
	auto bounds = std::make_unique<StructureBoundingBox>(
		StructureBoundingBox::getComponentToAddBoundingBox(x, y, z, 0, 0, 0, 10, 6, 7, direction));
	if (!canVillageGoDeeper(bounds.get()) || StructureComponent::findIntersecting(components, *bounds) != nullptr)
		return nullptr;
	return new ComponentVillageHouse2(type, random, std::move(bounds), direction);
}

bool ComponentVillageHouse2::addComponentParts(
	World *world, Random &random, const StructureBoundingBox &chunkBounds)
{
	if (averageGroundLevel < 0)
	{
		averageGroundLevel = getAverageGroundLevel(world, chunkBounds);
		if (averageGroundLevel < 0)
			return true;
		boundingBox->offset(0, averageGroundLevel - boundingBox->maxY + 5, 0);
	}

	fillWithBlocks(world, chunkBounds, 0, 1, 0, 9, 4, 6, 0, 0, false);
	fillWithBlocks(world, chunkBounds, 0, 0, 0, 9, 0, 6, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 4, 0, 9, 4, 6, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 5, 0, 9, 5, 6, Block::stairSingle->blockID, Block::stairSingle->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 5, 1, 8, 5, 5, 0, 0, false);
	fillWithBlocks(world, chunkBounds, 1, 1, 0, 2, 3, 0, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 1, 0, 0, 4, 0, Block::wood->blockID, Block::wood->blockID, false);
	fillWithBlocks(world, chunkBounds, 3, 1, 0, 3, 4, 0, Block::wood->blockID, Block::wood->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 1, 6, 0, 4, 6, Block::wood->blockID, Block::wood->blockID, false);
	placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 3, 3, 1, chunkBounds);
	fillWithBlocks(world, chunkBounds, 3, 1, 2, 3, 3, 2, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 4, 1, 3, 5, 3, 3, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 1, 1, 0, 3, 5, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 1, 6, 5, 3, 6, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 5, 1, 0, 5, 3, 0, Block::fence->blockID, Block::fence->blockID, false);
	fillWithBlocks(world, chunkBounds, 9, 1, 0, 9, 3, 0, Block::fence->blockID, Block::fence->blockID, false);
	fillWithBlocks(world, chunkBounds, 6, 1, 4, 9, 4, 6, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	placeBlockAtCurrentPosition(world, Block::lavaMoving->blockID, 0, 7, 1, 5, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::lavaMoving->blockID, 0, 8, 1, 5, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::fenceIron->blockID, 0, 9, 2, 5, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::fenceIron->blockID, 0, 9, 2, 4, chunkBounds);
	fillWithBlocks(world, chunkBounds, 7, 2, 4, 8, 2, 5, 0, 0, false);
	placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 6, 1, 3, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stoneOvenIdle->blockID, 0, 6, 2, 3, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stoneOvenIdle->blockID, 0, 6, 3, 3, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stairDouble->blockID, 0, 8, 1, 1, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 0, 2, 2, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 0, 2, 4, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 2, 2, 6, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 4, 2, 6, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, 2, 1, 4, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::pressurePlatePlanks->blockID, 0, 2, 2, 4, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 1, 1, 5, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stairCompactPlanks->blockID,
		getMetadataWithOffset(Block::stairCompactPlanks->blockID, 3), 2, 1, 5, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stairCompactPlanks->blockID,
		getMetadataWithOffset(Block::stairCompactPlanks->blockID, 1), 1, 1, 4, chunkBounds);

	if (!hasMadeChest)
	{
		const int_t y = getYWithOffset(1);
		const int_t x = getXWithOffset(5, 5);
		const int_t z = getZWithOffset(5, 5);
		if (chunkBounds.isVecInside(x, y, z))
		{
			hasMadeChest = true;
			createTreasureChestAtCurrentPosition(world, chunkBounds, random, 5, 1, 5,
				blacksmithChestLoot(), 3 + random.nextInt(6));
		}
	}

	for (int_t x = 6; x <= 8; ++x)
	{
		if (getBlockIdAtCurrentPosition(world, x, 0, -1, chunkBounds) == 0 &&
		    getBlockIdAtCurrentPosition(world, x, -1, -1, chunkBounds) != 0)
		{
			placeBlockAtCurrentPosition(world, Block::stairCompactCobblestone->blockID,
				getMetadataWithOffset(Block::stairCompactCobblestone->blockID, 3), x, 0, -1, chunkBounds);
		}
	}

	for (int_t z = 0; z < 7; ++z)
	{
		for (int_t x = 0; x < 10; ++x)
		{
			clearCurrentPositionBlocksUpwards(world, x, 6, z, chunkBounds);
			fillCurrentPositionBlocksDownwards(world, Block::cobblestone->blockID, 0, x, -1, z, chunkBounds);
		}
	}
	spawnVillagers(world, chunkBounds, 7, 1, 1, 1);
	return true;
}

int_t ComponentVillageHouse2::getVillagerType(int_t) const
{
	return 3;
}
