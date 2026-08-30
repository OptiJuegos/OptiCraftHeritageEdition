#include "ComponentVillageWell.h"

#include "Block.h"
#include "ComponentVillageStartPiece.h"
#include "StructureBoundingBox.h"
#include "StructureVillagePieces.h"
#include "World.h"
#include "java/Random.h"

ComponentVillageWell::ComponentVillageWell(int_t type, Random &random, int_t x, int_t z)
	: ComponentVillage(type), field_35104_a(true), averageGroundLevel(-1)
{
	coordBaseMode = random.nextInt(4);
	boundingBox.reset(new StructureBoundingBox(x, 64, z, x + 5, 78, z + 5));
}

void ComponentVillageWell::buildComponent(
	StructureComponent *parent, std::vector<StructureComponent *> &components, Random &random)
{
	ComponentVillageStartPiece *start = static_cast<ComponentVillageStartPiece *>(parent);
	StructureVillagePieces::getNextStructureComponentVillagePath(
		start, components, random, boundingBox->minX - 1, boundingBox->maxY - 4,
		boundingBox->minZ + 1, 1, getComponentType());
	StructureVillagePieces::getNextStructureComponentVillagePath(
		start, components, random, boundingBox->maxX + 1, boundingBox->maxY - 4,
		boundingBox->minZ + 1, 3, getComponentType());
	StructureVillagePieces::getNextStructureComponentVillagePath(
		start, components, random, boundingBox->minX + 1, boundingBox->maxY - 4,
		boundingBox->minZ - 1, 2, getComponentType());
	StructureVillagePieces::getNextStructureComponentVillagePath(
		start, components, random, boundingBox->minX + 1, boundingBox->maxY - 4,
		boundingBox->maxZ + 1, 0, getComponentType());
}

bool ComponentVillageWell::addComponentParts(
	World *world, Random &, const StructureBoundingBox &chunkBounds)
{
	if (averageGroundLevel < 0)
	{
		averageGroundLevel = getAverageGroundLevel(world, chunkBounds);
		if (averageGroundLevel < 0)
			return true;
		boundingBox->offset(0, averageGroundLevel - boundingBox->maxY + 3, 0);
	}

	fillWithBlocks(world, chunkBounds, 1, 0, 1, 4, 12, 4,
	               Block::cobblestone->blockID, Block::waterMoving->blockID, false);
	placeBlockAtCurrentPosition(world, 0, 0, 2, 12, 2, chunkBounds);
	placeBlockAtCurrentPosition(world, 0, 0, 3, 12, 2, chunkBounds);
	placeBlockAtCurrentPosition(world, 0, 0, 2, 12, 3, chunkBounds);
	placeBlockAtCurrentPosition(world, 0, 0, 3, 12, 3, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, 1, 13, 1, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, 1, 14, 1, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, 4, 13, 1, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, 4, 14, 1, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, 1, 13, 4, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, 1, 14, 4, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, 4, 13, 4, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, 4, 14, 4, chunkBounds);
	fillWithBlocks(world, chunkBounds, 1, 15, 1, 4, 15, 4,
	               Block::cobblestone->blockID, Block::cobblestone->blockID, false);

	for (int_t z = 0; z <= 5; ++z)
	{
		for (int_t x = 0; x <= 5; ++x)
		{
			if (x == 0 || x == 5 || z == 0 || z == 5)
			{
				placeBlockAtCurrentPosition(world, Block::gravel->blockID, 0, x, 11, z, chunkBounds);
				clearCurrentPositionBlocksUpwards(world, x, 12, z, chunkBounds);
			}
		}
	}
	return true;
}
