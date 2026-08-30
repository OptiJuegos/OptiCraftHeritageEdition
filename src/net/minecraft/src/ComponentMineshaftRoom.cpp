#include "ComponentMineshaftRoom.h"

#include <algorithm>

#include "Block.h"
#include "StructureBoundingBox.h"
#include "StructureMineshaftPieces.h"
#include "World.h"
#include "java/Random.h"

ComponentMineshaftRoom::ComponentMineshaftRoom(
	int_t type, Random &random, int_t x, int_t z)
	: StructureComponent(type)
{
	const int_t sizeX = random.nextInt(6);
	const int_t sizeY = random.nextInt(6);
	const int_t sizeZ = random.nextInt(6);
	boundingBox = std::make_unique<StructureBoundingBox>(
		x, 50, z, x + 7 + sizeX, 54 + sizeY, z + 7 + sizeZ);
}

void ComponentMineshaftRoom::buildComponent(
	StructureComponent *parent, std::vector<StructureComponent *> &components, Random &random)
{
	if (!boundingBox)
		return;

	const int_t type = getComponentType();
	int_t verticalRange = boundingBox->getYSize() - 4;
	if (verticalRange <= 0)
		verticalRange = 1;

	auto addOpening = [&](StructureComponent *component,
	                      int_t minX, int_t minY, int_t minZ,
	                      int_t maxX, int_t maxY, int_t maxZ)
	{
		if (component != nullptr)
			childStructures.push_back(std::make_unique<StructureBoundingBox>(
				minX, minY, minZ, maxX, maxY, maxZ));
	};

	for (int_t offset = 0; offset < boundingBox->getXSize(); offset += 4)
	{
		offset += random.nextInt(boundingBox->getXSize());
		if (offset + 3 > boundingBox->getXSize())
			break;
		StructureComponent *component = StructureMineshaftPieces::getNextComponent(
			parent, components, random,
			boundingBox->minX + offset, boundingBox->minY + random.nextInt(verticalRange) + 1,
			boundingBox->minZ - 1, 2, type);
		if (component != nullptr)
		{
			StructureBoundingBox *child = component->getBoundingBox();
			addOpening(component, child->minX, child->minY, boundingBox->minZ,
				child->maxX, child->maxY, boundingBox->minZ + 1);
		}
	}

	for (int_t offset = 0; offset < boundingBox->getXSize(); offset += 4)
	{
		offset += random.nextInt(boundingBox->getXSize());
		if (offset + 3 > boundingBox->getXSize())
			break;
		StructureComponent *component = StructureMineshaftPieces::getNextComponent(
			parent, components, random,
			boundingBox->minX + offset, boundingBox->minY + random.nextInt(verticalRange) + 1,
			boundingBox->maxZ + 1, 0, type);
		if (component != nullptr)
		{
			StructureBoundingBox *child = component->getBoundingBox();
			addOpening(component, child->minX, child->minY, boundingBox->maxZ - 1,
				child->maxX, child->maxY, boundingBox->maxZ);
		}
	}

	for (int_t offset = 0; offset < boundingBox->getZSize(); offset += 4)
	{
		offset += random.nextInt(boundingBox->getZSize());
		if (offset + 3 > boundingBox->getZSize())
			break;
		StructureComponent *component = StructureMineshaftPieces::getNextComponent(
			parent, components, random,
			boundingBox->minX - 1, boundingBox->minY + random.nextInt(verticalRange) + 1,
			boundingBox->minZ + offset, 1, type);
		if (component != nullptr)
		{
			StructureBoundingBox *child = component->getBoundingBox();
			addOpening(component, boundingBox->minX, child->minY, child->minZ,
				boundingBox->minX + 1, child->maxY, child->maxZ);
		}
	}

	for (int_t offset = 0; offset < boundingBox->getZSize(); offset += 4)
	{
		offset += random.nextInt(boundingBox->getZSize());
		if (offset + 3 > boundingBox->getZSize())
			break;
		StructureComponent *component = StructureMineshaftPieces::getNextComponent(
			parent, components, random,
			boundingBox->maxX + 1, boundingBox->minY + random.nextInt(verticalRange) + 1,
			boundingBox->minZ + offset, 3, type);
		if (component != nullptr)
		{
			StructureBoundingBox *child = component->getBoundingBox();
			addOpening(component, boundingBox->maxX - 1, child->minY, child->minZ,
				boundingBox->maxX, child->maxY, child->maxZ);
		}
	}
}

bool ComponentMineshaftRoom::addComponentParts(
	World *world, Random &, const StructureBoundingBox &chunkBounds)
{
	if (!boundingBox || isLiquidInStructureBoundingBox(world, chunkBounds))
		return false;

	fillWithBlocks(world, chunkBounds,
		boundingBox->minX, boundingBox->minY, boundingBox->minZ,
		boundingBox->maxX, boundingBox->minY, boundingBox->maxZ,
		Block::dirt->blockID, 0, true);
	fillWithBlocks(world, chunkBounds,
		boundingBox->minX, boundingBox->minY + 1, boundingBox->minZ,
		boundingBox->maxX, std::min(boundingBox->minY + 3, boundingBox->maxY), boundingBox->maxZ,
		0, 0, false);

	for (const auto &child : childStructures)
	{
		fillWithBlocks(world, chunkBounds,
			child->minX, child->maxY - 2, child->minZ,
			child->maxX, child->maxY, child->maxZ,
			0, 0, false);
	}

	randomlyRareFillWithBlocks(world, chunkBounds,
		boundingBox->minX, boundingBox->minY + 4, boundingBox->minZ,
		boundingBox->maxX, boundingBox->maxY, boundingBox->maxZ,
		0, false);
	return true;
}
