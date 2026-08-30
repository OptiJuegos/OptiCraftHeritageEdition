#include "ComponentStrongholdLibrary.h"

#include <vector>

#include "Block.h"
#include "Item.h"
#include "StructureBoundingBox.h"
#include "StructurePieceTreasure.h"
#include "StructureStrongholdPieces.h"
#include "java/Random.h"

namespace
{
const std::vector<StructurePieceTreasure> &getLibraryLoot()
{
	static const std::vector<StructurePieceTreasure> loot = {
		StructurePieceTreasure(Item::book->shiftedIndex, 0, 1, 3, 20),
		StructurePieceTreasure(Item::paper->shiftedIndex, 0, 2, 7, 20),
		StructurePieceTreasure(Item::mapItem->shiftedIndex, 0, 1, 1, 1),
		StructurePieceTreasure(Item::compass->shiftedIndex, 0, 1, 1, 1)
	};
	return loot;
}
}

ComponentStrongholdLibrary::ComponentStrongholdLibrary(
	int_t type, Random &random, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: ComponentStronghold(type), doorType(getRandomDoor(random)), isLargeRoom(bounds->getYSize() > 6)
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
}

void ComponentStrongholdLibrary::buildComponent(StructureComponent *,
                                                 std::vector<StructureComponent *> &, Random &)
{
}

ComponentStrongholdLibrary *ComponentStrongholdLibrary::findValidPlacement(
	const std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t type)
{
	auto bounds = std::make_unique<StructureBoundingBox>(
		StructureBoundingBox::getComponentToAddBoundingBox(x, y, z, -4, -1, 0, 14, 11, 15, direction));
	if (!canStrongholdGoDeeper(bounds.get()) || StructureComponent::findIntersecting(components, *bounds) != nullptr)
	{
		bounds = std::make_unique<StructureBoundingBox>(
			StructureBoundingBox::getComponentToAddBoundingBox(x, y, z, -4, -1, 0, 14, 6, 15, direction));
		if (!canStrongholdGoDeeper(bounds.get()) || StructureComponent::findIntersecting(components, *bounds) != nullptr)
			return nullptr;
	}
	return new ComponentStrongholdLibrary(type, random, std::move(bounds), direction);
}

bool ComponentStrongholdLibrary::addComponentParts(
	World *world, Random &random, const StructureBoundingBox &chunkBounds)
{
	if (isLiquidInStructureBoundingBox(world, chunkBounds))
		return false;
	const int_t height = isLargeRoom ? 11 : 6;
	fillWithRandomizedBlocks(world, chunkBounds, 0, 0, 0, 13, height - 1, 14, true, random,
	                         StructureStrongholdPieces::getStrongholdStones());
	placeDoor(world, random, chunkBounds, doorType, 4, 1, 0);
	randomlyFillWithBlocks(world, chunkBounds, random, 0.07F,
	                       2, 1, 1, 11, 4, 13,
	                       Block::web->blockID, Block::web->blockID, false);

	for (int_t z = 1; z <= 13; ++z)
	{
		if ((z - 1) % 4 == 0)
		{
			fillWithBlocks(world, chunkBounds, 1, 1, z, 1, 4, z, Block::planks->blockID, Block::planks->blockID, false);
			fillWithBlocks(world, chunkBounds, 12, 1, z, 12, 4, z, Block::planks->blockID, Block::planks->blockID, false);
			placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 0, 2, 3, z, chunkBounds);
			placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 0, 11, 3, z, chunkBounds);
			if (isLargeRoom)
			{
				fillWithBlocks(world, chunkBounds, 1, 6, z, 1, 9, z, Block::planks->blockID, Block::planks->blockID, false);
				fillWithBlocks(world, chunkBounds, 12, 6, z, 12, 9, z, Block::planks->blockID, Block::planks->blockID, false);
			}
		}
		else
		{
			fillWithBlocks(world, chunkBounds, 1, 1, z, 1, 4, z, Block::bookShelf->blockID, Block::bookShelf->blockID, false);
			fillWithBlocks(world, chunkBounds, 12, 1, z, 12, 4, z, Block::bookShelf->blockID, Block::bookShelf->blockID, false);
			if (isLargeRoom)
			{
				fillWithBlocks(world, chunkBounds, 1, 6, z, 1, 9, z, Block::bookShelf->blockID, Block::bookShelf->blockID, false);
				fillWithBlocks(world, chunkBounds, 12, 6, z, 12, 9, z, Block::bookShelf->blockID, Block::bookShelf->blockID, false);
			}
		}
	}

	for (int_t z = 3; z < 12; z += 2)
	{
		fillWithBlocks(world, chunkBounds, 3, 1, z, 4, 3, z, Block::bookShelf->blockID, Block::bookShelf->blockID, false);
		fillWithBlocks(world, chunkBounds, 6, 1, z, 7, 3, z, Block::bookShelf->blockID, Block::bookShelf->blockID, false);
		fillWithBlocks(world, chunkBounds, 9, 1, z, 10, 3, z, Block::bookShelf->blockID, Block::bookShelf->blockID, false);
	}

	if (isLargeRoom)
	{
		fillWithBlocks(world, chunkBounds, 1, 5, 1, 3, 5, 13, Block::planks->blockID, Block::planks->blockID, false);
		fillWithBlocks(world, chunkBounds, 10, 5, 1, 12, 5, 13, Block::planks->blockID, Block::planks->blockID, false);
		fillWithBlocks(world, chunkBounds, 4, 5, 1, 9, 5, 2, Block::planks->blockID, Block::planks->blockID, false);
		fillWithBlocks(world, chunkBounds, 4, 5, 12, 9, 5, 13, Block::planks->blockID, Block::planks->blockID, false);
		placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 9, 5, 11, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 8, 5, 11, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 9, 5, 10, chunkBounds);
		fillWithBlocks(world, chunkBounds, 3, 6, 2, 3, 6, 12, Block::fence->blockID, Block::fence->blockID, false);
		fillWithBlocks(world, chunkBounds, 10, 6, 2, 10, 6, 10, Block::fence->blockID, Block::fence->blockID, false);
		fillWithBlocks(world, chunkBounds, 4, 6, 2, 9, 6, 2, Block::fence->blockID, Block::fence->blockID, false);
		fillWithBlocks(world, chunkBounds, 4, 6, 12, 8, 6, 12, Block::fence->blockID, Block::fence->blockID, false);
		placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, 9, 6, 11, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, 8, 6, 11, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, 9, 6, 10, chunkBounds);
		const int_t ladderMeta = getMetadataWithOffset(Block::ladder->blockID, 3);
		for (int_t y = 1; y <= 7; ++y)
			placeBlockAtCurrentPosition(world, Block::ladder->blockID, ladderMeta, 10, y, 13, chunkBounds);

		const int_t centerX = 7;
		const int_t centerZ = 7;
		placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, centerX - 1, 9, centerZ, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, centerX, 9, centerZ, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, centerX - 1, 8, centerZ, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, centerX, 8, centerZ, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, centerX - 1, 7, centerZ, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, centerX, 7, centerZ, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, centerX - 2, 7, centerZ, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, centerX + 1, 7, centerZ, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, centerX - 1, 7, centerZ - 1, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, centerX - 1, 7, centerZ + 1, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, centerX, 7, centerZ - 1, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, centerX, 7, centerZ + 1, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 0, centerX - 2, 8, centerZ, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 0, centerX + 1, 8, centerZ, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 0, centerX - 1, 8, centerZ - 1, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 0, centerX - 1, 8, centerZ + 1, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 0, centerX, 8, centerZ - 1, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 0, centerX, 8, centerZ + 1, chunkBounds);
	}

	createTreasureChestAtCurrentPosition(world, chunkBounds, random, 3, 3, 5,
	                                     getLibraryLoot(), 1 + random.nextInt(4));
	if (isLargeRoom)
	{
		placeBlockAtCurrentPosition(world, 0, 0, 12, 9, 1, chunkBounds);
		createTreasureChestAtCurrentPosition(world, chunkBounds, random, 12, 8, 1,
		                                     getLibraryLoot(), 1 + random.nextInt(4));
	}
	return true;
}
