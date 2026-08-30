#include "ComponentVillageField.h"

#include "Block.h"
#include "MathHelper.h"
#include "StructureBoundingBox.h"
#include "World.h"
#include "java/Random.h"

ComponentVillageField::ComponentVillageField(
	int_t type, Random &, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: ComponentVillage(type), averageGroundLevel(-1)
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
}

void ComponentVillageField::buildComponent(
	StructureComponent *, std::vector<StructureComponent *> &, Random &)
{
}

ComponentVillageField *ComponentVillageField::findValidPlacement(
	const std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t type)
{
	auto bounds = std::make_unique<StructureBoundingBox>(
		StructureBoundingBox::getComponentToAddBoundingBox(x, y, z, 0, 0, 0, 13, 4, 9, direction));
	if (!canVillageGoDeeper(bounds.get()) || StructureComponent::findIntersecting(components, *bounds) != nullptr)
		return nullptr;
	return new ComponentVillageField(type, random, std::move(bounds), direction);
}

bool ComponentVillageField::addComponentParts(
	World *world, Random &random, const StructureBoundingBox &chunkBounds)
{
	if (averageGroundLevel < 0)
	{
		averageGroundLevel = getAverageGroundLevel(world, chunkBounds);
		if (averageGroundLevel < 0)
			return true;
		boundingBox->offset(0, averageGroundLevel - boundingBox->maxY + 3, 0);
	}

	fillWithBlocks(world, chunkBounds, 0, 1, 0, 12, 4, 8, 0, 0, false);
	fillWithBlocks(world, chunkBounds, 1, 0, 1, 2, 0, 7, Block::tilledField->blockID, Block::tilledField->blockID, false);
	fillWithBlocks(world, chunkBounds, 4, 0, 1, 5, 0, 7, Block::tilledField->blockID, Block::tilledField->blockID, false);
	fillWithBlocks(world, chunkBounds, 7, 0, 1, 8, 0, 7, Block::tilledField->blockID, Block::tilledField->blockID, false);
	fillWithBlocks(world, chunkBounds, 10, 0, 1, 11, 0, 7, Block::tilledField->blockID, Block::tilledField->blockID, false);
	fillWithBlocks(world, chunkBounds, 0, 0, 0, 0, 0, 8, Block::wood->blockID, Block::wood->blockID, false);
	fillWithBlocks(world, chunkBounds, 6, 0, 0, 6, 0, 8, Block::wood->blockID, Block::wood->blockID, false);
	fillWithBlocks(world, chunkBounds, 12, 0, 0, 12, 0, 8, Block::wood->blockID, Block::wood->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 0, 0, 11, 0, 0, Block::wood->blockID, Block::wood->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 0, 8, 11, 0, 8, Block::wood->blockID, Block::wood->blockID, false);
	fillWithBlocks(world, chunkBounds, 3, 0, 1, 3, 0, 7, Block::waterMoving->blockID, Block::waterMoving->blockID, false);
	fillWithBlocks(world, chunkBounds, 9, 0, 1, 9, 0, 7, Block::waterMoving->blockID, Block::waterMoving->blockID, false);

	for (int_t z = 1; z <= 7; ++z)
	{
		for (int_t x : {1, 2, 4, 5, 7, 8, 10, 11})
			placeBlockAtCurrentPosition(world, Block::crops->blockID,
				MathHelper::getRandomIntegerInRange(random, 2, 7), x, 1, z, chunkBounds);
	}

	for (int_t z = 0; z < 9; ++z)
	{
		for (int_t x = 0; x < 13; ++x)
		{
			clearCurrentPositionBlocksUpwards(world, x, 4, z, chunkBounds);
			fillCurrentPositionBlocksDownwards(world, Block::dirt->blockID, 0, x, -1, z, chunkBounds);
		}
	}
	return true;
}
