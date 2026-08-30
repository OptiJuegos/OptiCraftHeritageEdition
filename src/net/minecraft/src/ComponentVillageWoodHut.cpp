#include "ComponentVillageWoodHut.h"

#include "Block.h"
#include "StructureBoundingBox.h"
#include "World.h"
#include "java/Random.h"

ComponentVillageWoodHut::ComponentVillageWoodHut(
	int_t type, Random &random, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: ComponentVillage(type), averageGroundLevel(-1), isTallHouse(random.nextBoolean()),
	  tablePosition(random.nextInt(3))
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
}

void ComponentVillageWoodHut::buildComponent(
	StructureComponent *, std::vector<StructureComponent *> &, Random &)
{
}

ComponentVillageWoodHut *ComponentVillageWoodHut::findValidPlacement(
	const std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t type)
{
	auto bounds = std::make_unique<StructureBoundingBox>(
		StructureBoundingBox::getComponentToAddBoundingBox(x, y, z, 0, 0, 0, 4, 6, 5, direction));
	if (!canVillageGoDeeper(bounds.get()) || StructureComponent::findIntersecting(components, *bounds) != nullptr)
		return nullptr;
	return new ComponentVillageWoodHut(type, random, std::move(bounds), direction);
}

bool ComponentVillageWoodHut::addComponentParts(
	World *world, Random &random, const StructureBoundingBox &chunkBounds)
{
	if (averageGroundLevel < 0)
	{
		averageGroundLevel = getAverageGroundLevel(world, chunkBounds);
		if (averageGroundLevel < 0)
			return true;
		boundingBox->offset(0, averageGroundLevel - boundingBox->maxY + 5, 0);
	}

	fillWithBlocks(world, chunkBounds, 1, 1, 1, 3, 5, 4, 0, 0, false);
	fillWithBlocks(world, chunkBounds, 0, 0, 0, 3, 0, 4, Block::cobblestone->blockID, Block::cobblestone->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 0, 1, 2, 0, 3, Block::dirt->blockID, Block::dirt->blockID, false);
	if (isTallHouse)
		fillWithBlocks(world, chunkBounds, 1, 4, 1, 2, 4, 3, Block::wood->blockID, Block::wood->blockID, false);
	else
		fillWithBlocks(world, chunkBounds, 1, 5, 1, 2, 5, 3, Block::wood->blockID, Block::wood->blockID, false);

	placeBlockAtCurrentPosition(world, Block::wood->blockID, 0, 1, 4, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::wood->blockID, 0, 2, 4, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::wood->blockID, 0, 1, 4, 4, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::wood->blockID, 0, 2, 4, 4, chunkBounds);
	for (int_t z = 1; z <= 3; ++z)
	{
		placeBlockAtCurrentPosition(world, Block::wood->blockID, 0, 0, 4, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::wood->blockID, 0, 3, 4, z, chunkBounds);
	}
	fillWithBlocks(world, chunkBounds, 0, 1, 0, 0, 3, 0, Block::wood->blockID, Block::wood->blockID, false);
	fillWithBlocks(world, chunkBounds, 3, 1, 0, 3, 3, 0, Block::wood->blockID, Block::wood->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 1, 4, 0, 3, 4, Block::wood->blockID, Block::wood->blockID, false);
	fillWithBlocks(world, chunkBounds, 3, 1, 4, 3, 3, 4, Block::wood->blockID, Block::wood->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 1, 1, 0, 3, 3, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 3, 1, 1, 3, 3, 3, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 1, 0, 2, 3, 0, Block::planks->blockID, Block::planks->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 1, 4, 2, 3, 4, Block::planks->blockID, Block::planks->blockID, false);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 0, 2, 2, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::thinGlass->blockID, 0, 3, 2, 2, chunkBounds);
	if (tablePosition > 0)
	{
		placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, tablePosition, 1, 3, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::pressurePlatePlanks->blockID, 0, tablePosition, 2, 3, chunkBounds);
	}

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

	for (int_t z = 0; z < 5; ++z)
	{
		for (int_t x = 0; x < 4; ++x)
		{
			clearCurrentPositionBlocksUpwards(world, x, 6, z, chunkBounds);
			fillCurrentPositionBlocksDownwards(world, Block::cobblestone->blockID, 0, x, -1, z, chunkBounds);
		}
	}
	spawnVillagers(world, chunkBounds, 1, 1, 2, 1);
	return true;
}
