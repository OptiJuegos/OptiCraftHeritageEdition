#include "ComponentVillagePathGen.h"

#include <algorithm>
#include <cstdlib>

#include "Block.h"
#include "ComponentVillageStartPiece.h"
#include "MathHelper.h"
#include "StructureBoundingBox.h"
#include "StructureVillagePieces.h"
#include "World.h"
#include "java/Random.h"

ComponentVillagePathGen::ComponentVillagePathGen(
	int_t type, Random &, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: ComponentVillageRoadPiece(type), averageGroundLevel(0)
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
	averageGroundLevel = std::max(boundingBox->getXSize(), boundingBox->getZSize());
}

void ComponentVillagePathGen::buildComponent(
	StructureComponent *parent, std::vector<StructureComponent *> &components, Random &random)
{
	ComponentVillageStartPiece *start = static_cast<ComponentVillageStartPiece *>(parent);
	bool addedBuilding = false;

	for (int_t offset = random.nextInt(5); offset < averageGroundLevel - 8;
	     offset += 2 + random.nextInt(5))
	{
		StructureComponent *component = getNextComponentNN(start, components, random, 0, offset);
		if (component != nullptr && component->getBoundingBox() != nullptr)
		{
			offset += std::max(component->getBoundingBox()->getXSize(), component->getBoundingBox()->getZSize());
			addedBuilding = true;
		}
	}

	for (int_t offset = random.nextInt(5); offset < averageGroundLevel - 8;
	     offset += 2 + random.nextInt(5))
	{
		StructureComponent *component = getNextComponentPP(start, components, random, 0, offset);
		if (component != nullptr && component->getBoundingBox() != nullptr)
		{
			offset += std::max(component->getBoundingBox()->getXSize(), component->getBoundingBox()->getZSize());
			addedBuilding = true;
		}
	}

	if (addedBuilding && random.nextInt(3) > 0)
	{
		switch (coordBaseMode)
		{
		case 0:
			StructureVillagePieces::getNextStructureComponentVillagePath(start, components, random,
				boundingBox->minX - 1, boundingBox->minY, boundingBox->maxZ - 2, 1, getComponentType());
			break;
		case 1:
			StructureVillagePieces::getNextStructureComponentVillagePath(start, components, random,
				boundingBox->minX, boundingBox->minY, boundingBox->minZ - 1, 2, getComponentType());
			break;
		case 2:
			StructureVillagePieces::getNextStructureComponentVillagePath(start, components, random,
				boundingBox->minX - 1, boundingBox->minY, boundingBox->minZ, 1, getComponentType());
			break;
		case 3:
			StructureVillagePieces::getNextStructureComponentVillagePath(start, components, random,
				boundingBox->maxX - 2, boundingBox->minY, boundingBox->minZ - 1, 2, getComponentType());
			break;
		}
	}

	if (addedBuilding && random.nextInt(3) > 0)
	{
		switch (coordBaseMode)
		{
		case 0:
			StructureVillagePieces::getNextStructureComponentVillagePath(start, components, random,
				boundingBox->maxX + 1, boundingBox->minY, boundingBox->maxZ - 2, 3, getComponentType());
			break;
		case 1:
			StructureVillagePieces::getNextStructureComponentVillagePath(start, components, random,
				boundingBox->minX, boundingBox->minY, boundingBox->maxZ + 1, 0, getComponentType());
			break;
		case 2:
			StructureVillagePieces::getNextStructureComponentVillagePath(start, components, random,
				boundingBox->maxX + 1, boundingBox->minY, boundingBox->minZ, 3, getComponentType());
			break;
		case 3:
			StructureVillagePieces::getNextStructureComponentVillagePath(start, components, random,
				boundingBox->maxX - 2, boundingBox->minY, boundingBox->maxZ + 1, 0, getComponentType());
			break;
		}
	}
}

std::unique_ptr<StructureBoundingBox> ComponentVillagePathGen::func_35087_a(
	ComponentVillageStartPiece *, const std::vector<StructureComponent *> &components,
	Random &random, int_t x, int_t y, int_t z, int_t direction)
{
	for (int_t length = 7 * MathHelper::getRandomIntegerInRange(random, 3, 5);
	     length >= 7; length -= 7)
	{
		auto bounds = std::make_unique<StructureBoundingBox>(
			StructureBoundingBox::getComponentToAddBoundingBox(
				x, y, z, 0, 0, 0, 3, 3, length, direction));
		if (StructureComponent::findIntersecting(components, *bounds) == nullptr)
			return bounds;
	}
	return nullptr;
}

bool ComponentVillagePathGen::addComponentParts(
	World *world, Random &, const StructureBoundingBox &chunkBounds)
{
	for (int_t x = boundingBox->minX; x <= boundingBox->maxX; ++x)
	{
		for (int_t z = boundingBox->minZ; z <= boundingBox->maxZ; ++z)
		{
			if (chunkBounds.isVecInside(x, 64, z))
			{
				const int_t y = world->getTopSolidOrLiquidBlock(x, z) - 1;
				world->setBlock(x, y, z, Block::gravel->blockID);
			}
		}
	}
	return true;
}
