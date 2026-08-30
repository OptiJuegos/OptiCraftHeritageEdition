#include "ComponentMineshaftStairs.h"

#include "StructureBoundingBox.h"
#include "StructureMineshaftPieces.h"
#include "World.h"
#include "java/Random.h"

ComponentMineshaftStairs::ComponentMineshaftStairs(
	int_t type, Random &, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: StructureComponent(type)
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
}

std::unique_ptr<StructureBoundingBox> ComponentMineshaftStairs::findValidPlacement(
	const std::vector<StructureComponent *> &components, Random &,
	int_t x, int_t y, int_t z, int_t direction)
{
	auto bounds = std::make_unique<StructureBoundingBox>(x, y - 5, z, x, y + 2, z);
	switch (direction)
	{
	case 0:
		bounds->maxX = x + 2;
		bounds->maxZ = z + 8;
		break;
	case 1:
		bounds->minX = x - 8;
		bounds->maxZ = z + 2;
		break;
	case 2:
		bounds->maxX = x + 2;
		bounds->minZ = z - 8;
		break;
	case 3:
		bounds->maxX = x + 8;
		bounds->maxZ = z + 2;
		break;
	default:
		break;
	}

	return StructureComponent::findIntersecting(components, *bounds) != nullptr
		? nullptr
		: std::move(bounds);
}

void ComponentMineshaftStairs::buildComponent(
	StructureComponent *parent, std::vector<StructureComponent *> &components, Random &random)
{
	if (!boundingBox)
		return;

	const int_t type = getComponentType();
	switch (coordBaseMode)
	{
	case 0:
		StructureMineshaftPieces::getNextComponent(parent, components, random,
			boundingBox->minX, boundingBox->minY, boundingBox->maxZ + 1, 0, type);
		break;
	case 1:
		StructureMineshaftPieces::getNextComponent(parent, components, random,
			boundingBox->minX - 1, boundingBox->minY, boundingBox->minZ, 1, type);
		break;
	case 2:
		StructureMineshaftPieces::getNextComponent(parent, components, random,
			boundingBox->minX, boundingBox->minY, boundingBox->minZ - 1, 2, type);
		break;
	case 3:
		StructureMineshaftPieces::getNextComponent(parent, components, random,
			boundingBox->maxX + 1, boundingBox->minY, boundingBox->minZ, 3, type);
		break;
	default:
		break;
	}
}

bool ComponentMineshaftStairs::addComponentParts(
	World *world, Random &, const StructureBoundingBox &chunkBounds)
{
	if (isLiquidInStructureBoundingBox(world, chunkBounds))
		return false;

	fillWithBlocks(world, chunkBounds, 0, 5, 0, 2, 7, 1, 0, 0, false);
	fillWithBlocks(world, chunkBounds, 0, 0, 7, 2, 2, 8, 0, 0, false);
	for (int_t step = 0; step < 5; ++step)
	{
		fillWithBlocks(world, chunkBounds,
			0, 5 - step - (step < 4 ? 1 : 0), 2 + step,
			2, 7 - step, 2 + step,
			0, 0, false);
	}
	return true;
}
