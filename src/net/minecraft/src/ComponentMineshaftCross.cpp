#include "ComponentMineshaftCross.h"

#include "Block.h"
#include "StructureBoundingBox.h"
#include "StructureMineshaftPieces.h"
#include "World.h"
#include "java/Random.h"

ComponentMineshaftCross::ComponentMineshaftCross(
	int_t type, Random &, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: StructureComponent(type),
	  corridorDirection(direction),
	  isMultipleFloors(bounds != nullptr && bounds->getYSize() > 3)
{
	boundingBox = std::move(bounds);
}

std::unique_ptr<StructureBoundingBox> ComponentMineshaftCross::findValidPlacement(
	const std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction)
{
	auto bounds = std::make_unique<StructureBoundingBox>(x, y, z, x, y + 2, z);
	if (random.nextInt(4) == 0)
		bounds->maxY += 4;

	switch (direction)
	{
	case 0:
		bounds->minX = x - 1;
		bounds->maxX = x + 3;
		bounds->maxZ = z + 4;
		break;
	case 1:
		bounds->minX = x - 4;
		bounds->minZ = z - 1;
		bounds->maxZ = z + 3;
		break;
	case 2:
		bounds->minX = x - 1;
		bounds->maxX = x + 3;
		bounds->minZ = z - 4;
		break;
	case 3:
		bounds->maxX = x + 4;
		bounds->minZ = z - 1;
		bounds->maxZ = z + 3;
		break;
	default:
		break;
	}

	return StructureComponent::findIntersecting(components, *bounds) != nullptr
		? nullptr
		: std::move(bounds);
}

void ComponentMineshaftCross::buildComponent(
	StructureComponent *parent, std::vector<StructureComponent *> &components, Random &random)
{
	if (!boundingBox)
		return;

	const int_t type = getComponentType();
	switch (corridorDirection)
	{
	case 0:
		StructureMineshaftPieces::getNextComponent(parent, components, random,
			boundingBox->minX + 1, boundingBox->minY, boundingBox->maxZ + 1, 0, type);
		StructureMineshaftPieces::getNextComponent(parent, components, random,
			boundingBox->minX - 1, boundingBox->minY, boundingBox->minZ + 1, 1, type);
		StructureMineshaftPieces::getNextComponent(parent, components, random,
			boundingBox->maxX + 1, boundingBox->minY, boundingBox->minZ + 1, 3, type);
		break;
	case 1:
		StructureMineshaftPieces::getNextComponent(parent, components, random,
			boundingBox->minX + 1, boundingBox->minY, boundingBox->minZ - 1, 2, type);
		StructureMineshaftPieces::getNextComponent(parent, components, random,
			boundingBox->minX + 1, boundingBox->minY, boundingBox->maxZ + 1, 0, type);
		StructureMineshaftPieces::getNextComponent(parent, components, random,
			boundingBox->minX - 1, boundingBox->minY, boundingBox->minZ + 1, 1, type);
		break;
	case 2:
		StructureMineshaftPieces::getNextComponent(parent, components, random,
			boundingBox->minX + 1, boundingBox->minY, boundingBox->minZ - 1, 2, type);
		StructureMineshaftPieces::getNextComponent(parent, components, random,
			boundingBox->minX - 1, boundingBox->minY, boundingBox->minZ + 1, 1, type);
		StructureMineshaftPieces::getNextComponent(parent, components, random,
			boundingBox->maxX + 1, boundingBox->minY, boundingBox->minZ + 1, 3, type);
		break;
	case 3:
		StructureMineshaftPieces::getNextComponent(parent, components, random,
			boundingBox->minX + 1, boundingBox->minY, boundingBox->minZ - 1, 2, type);
		StructureMineshaftPieces::getNextComponent(parent, components, random,
			boundingBox->minX + 1, boundingBox->minY, boundingBox->maxZ + 1, 0, type);
		StructureMineshaftPieces::getNextComponent(parent, components, random,
			boundingBox->maxX + 1, boundingBox->minY, boundingBox->minZ + 1, 3, type);
		break;
	default:
		break;
	}

	if (!isMultipleFloors)
		return;

	if (random.nextBoolean())
		StructureMineshaftPieces::getNextComponent(parent, components, random,
			boundingBox->minX + 1, boundingBox->minY + 4, boundingBox->minZ - 1, 2, type);
	if (random.nextBoolean())
		StructureMineshaftPieces::getNextComponent(parent, components, random,
			boundingBox->minX - 1, boundingBox->minY + 4, boundingBox->minZ + 1, 1, type);
	if (random.nextBoolean())
		StructureMineshaftPieces::getNextComponent(parent, components, random,
			boundingBox->maxX + 1, boundingBox->minY + 4, boundingBox->minZ + 1, 3, type);
	if (random.nextBoolean())
		StructureMineshaftPieces::getNextComponent(parent, components, random,
			boundingBox->minX + 1, boundingBox->minY + 4, boundingBox->maxZ + 1, 0, type);
}

bool ComponentMineshaftCross::addComponentParts(
	World *world, Random &, const StructureBoundingBox &chunkBounds)
{
	if (!boundingBox || isLiquidInStructureBoundingBox(world, chunkBounds))
		return false;

	if (isMultipleFloors)
	{
		fillWithBlocks(world, chunkBounds,
			boundingBox->minX + 1, boundingBox->minY, boundingBox->minZ,
			boundingBox->maxX - 1, boundingBox->minY + 2, boundingBox->maxZ,
			0, 0, false);
		fillWithBlocks(world, chunkBounds,
			boundingBox->minX, boundingBox->minY, boundingBox->minZ + 1,
			boundingBox->maxX, boundingBox->minY + 2, boundingBox->maxZ - 1,
			0, 0, false);
		fillWithBlocks(world, chunkBounds,
			boundingBox->minX + 1, boundingBox->maxY - 2, boundingBox->minZ,
			boundingBox->maxX - 1, boundingBox->maxY, boundingBox->maxZ,
			0, 0, false);
		fillWithBlocks(world, chunkBounds,
			boundingBox->minX, boundingBox->maxY - 2, boundingBox->minZ + 1,
			boundingBox->maxX, boundingBox->maxY, boundingBox->maxZ - 1,
			0, 0, false);
		fillWithBlocks(world, chunkBounds,
			boundingBox->minX + 1, boundingBox->minY + 3, boundingBox->minZ + 1,
			boundingBox->maxX - 1, boundingBox->minY + 3, boundingBox->maxZ - 1,
			0, 0, false);
	}
	else
	{
		fillWithBlocks(world, chunkBounds,
			boundingBox->minX + 1, boundingBox->minY, boundingBox->minZ,
			boundingBox->maxX - 1, boundingBox->maxY, boundingBox->maxZ,
			0, 0, false);
		fillWithBlocks(world, chunkBounds,
			boundingBox->minX, boundingBox->minY, boundingBox->minZ + 1,
			boundingBox->maxX, boundingBox->maxY, boundingBox->maxZ - 1,
			0, 0, false);
	}

	fillWithBlocks(world, chunkBounds,
		boundingBox->minX + 1, boundingBox->minY, boundingBox->minZ + 1,
		boundingBox->minX + 1, boundingBox->maxY, boundingBox->minZ + 1,
		Block::planks->blockID, 0, false);
	fillWithBlocks(world, chunkBounds,
		boundingBox->minX + 1, boundingBox->minY, boundingBox->maxZ - 1,
		boundingBox->minX + 1, boundingBox->maxY, boundingBox->maxZ - 1,
		Block::planks->blockID, 0, false);
	fillWithBlocks(world, chunkBounds,
		boundingBox->maxX - 1, boundingBox->minY, boundingBox->minZ + 1,
		boundingBox->maxX - 1, boundingBox->maxY, boundingBox->minZ + 1,
		Block::planks->blockID, 0, false);
	fillWithBlocks(world, chunkBounds,
		boundingBox->maxX - 1, boundingBox->minY, boundingBox->maxZ - 1,
		boundingBox->maxX - 1, boundingBox->maxY, boundingBox->maxZ - 1,
		Block::planks->blockID, 0, false);

	for (int_t x = boundingBox->minX; x <= boundingBox->maxX; ++x)
	{
		for (int_t z = boundingBox->minZ; z <= boundingBox->maxZ; ++z)
		{
			if (getBlockIdAtCurrentPosition(world, x, boundingBox->minY - 1, z, chunkBounds) == 0)
				placeBlockAtCurrentPosition(world, Block::planks->blockID, 0,
					x, boundingBox->minY - 1, z, chunkBounds);
		}
	}

	return true;
}
