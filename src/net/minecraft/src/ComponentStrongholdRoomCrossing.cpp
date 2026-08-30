#include "ComponentStrongholdRoomCrossing.h"

#include <vector>

#include "Block.h"
#include "ComponentStrongholdStairs2.h"
#include "Item.h"
#include "StructureBoundingBox.h"
#include "StructurePieceTreasure.h"
#include "StructureStrongholdPieces.h"
#include "java/Random.h"

namespace
{
const std::vector<StructurePieceTreasure> &getRoomCrossingLoot()
{
	static const std::vector<StructurePieceTreasure> loot = {
		StructurePieceTreasure(Item::ingotIron->shiftedIndex, 0, 1, 5, 10),
		StructurePieceTreasure(Item::ingotGold->shiftedIndex, 0, 1, 3, 5),
		StructurePieceTreasure(Item::redstone->shiftedIndex, 0, 4, 9, 5),
		StructurePieceTreasure(Item::coal->shiftedIndex, 0, 3, 8, 10),
		StructurePieceTreasure(Item::bread->shiftedIndex, 0, 1, 3, 15),
		StructurePieceTreasure(Item::appleRed->shiftedIndex, 0, 1, 3, 15),
		StructurePieceTreasure(Item::pickaxeSteel->shiftedIndex, 0, 1, 1, 1)
	};
	return loot;
}
}

ComponentStrongholdRoomCrossing::ComponentStrongholdRoomCrossing(
	int_t type, Random &random, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: ComponentStronghold(type), doorType(getRandomDoor(random)), roomType(random.nextInt(5))
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
}

void ComponentStrongholdRoomCrossing::buildComponent(
	StructureComponent *parent, std::vector<StructureComponent *> &components, Random &random)
{
	auto *start = static_cast<ComponentStrongholdStairs2 *>(parent);
	getNextComponentNormal(start, components, random, 4, 1);
	getNextComponentX(start, components, random, 1, 4);
	getNextComponentZ(start, components, random, 1, 4);
}

ComponentStrongholdRoomCrossing *ComponentStrongholdRoomCrossing::findValidPlacement(
	const std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t type)
{
	auto bounds = std::make_unique<StructureBoundingBox>(
		StructureBoundingBox::getComponentToAddBoundingBox(x, y, z, -4, -1, 0, 11, 7, 11, direction));
	if (!canStrongholdGoDeeper(bounds.get()) || StructureComponent::findIntersecting(components, *bounds) != nullptr)
		return nullptr;
	return new ComponentStrongholdRoomCrossing(type, random, std::move(bounds), direction);
}

bool ComponentStrongholdRoomCrossing::addComponentParts(
	World *world, Random &random, const StructureBoundingBox &chunkBounds)
{
	if (isLiquidInStructureBoundingBox(world, chunkBounds))
		return false;
	fillWithRandomizedBlocks(world, chunkBounds, 0, 0, 0, 10, 6, 10, true, random,
	                         StructureStrongholdPieces::getStrongholdStones());
	placeDoor(world, random, chunkBounds, doorType, 4, 1, 0);
	fillWithBlocks(world, chunkBounds, 4, 1, 10, 6, 3, 10, 0, 0, false);
	fillWithBlocks(world, chunkBounds, 0, 1, 4, 0, 3, 6, 0, 0, false);
	fillWithBlocks(world, chunkBounds, 10, 1, 4, 10, 3, 6, 0, 0, false);

	switch (roomType)
	{
	case 0:
		placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 5, 1, 5, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 5, 2, 5, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 5, 3, 5, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 0, 4, 3, 5, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 0, 6, 3, 5, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 0, 5, 3, 4, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 0, 5, 3, 6, chunkBounds);
		for (int_t z : {4, 5, 6})
		{
			placeBlockAtCurrentPosition(world, Block::stairSingle->blockID, 0, 4, 1, z, chunkBounds);
			placeBlockAtCurrentPosition(world, Block::stairSingle->blockID, 0, 6, 1, z, chunkBounds);
		}
		placeBlockAtCurrentPosition(world, Block::stairSingle->blockID, 0, 5, 1, 4, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::stairSingle->blockID, 0, 5, 1, 6, chunkBounds);
		break;
	case 1:
		for (int_t i = 0; i < 5; ++i)
		{
			placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 3, 1, 3 + i, chunkBounds);
			placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 7, 1, 3 + i, chunkBounds);
			placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 3 + i, 1, 3, chunkBounds);
			placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 3 + i, 1, 7, chunkBounds);
		}
		placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 5, 1, 5, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 5, 2, 5, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 5, 3, 5, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::waterMoving->blockID, 0, 5, 4, 5, chunkBounds);
		break;
	case 2:
		for (int_t i = 1; i <= 9; ++i)
		{
			placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 1, 3, i, chunkBounds);
			placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 9, 3, i, chunkBounds);
			placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, i, 3, 1, chunkBounds);
			placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, i, 3, 9, chunkBounds);
		}
		placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 5, 1, 4, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 5, 1, 6, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 5, 3, 4, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 5, 3, 6, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 4, 1, 5, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 6, 1, 5, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 4, 3, 5, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 6, 3, 5, chunkBounds);
		for (int_t y = 1; y <= 3; ++y)
		{
			placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 4, y, 4, chunkBounds);
			placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 6, y, 4, chunkBounds);
			placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 4, y, 6, chunkBounds);
			placeBlockAtCurrentPosition(world, Block::cobblestone->blockID, 0, 6, y, 6, chunkBounds);
		}
		placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 0, 5, 3, 5, chunkBounds);
		for (int_t z = 2; z <= 8; ++z)
		{
			placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 2, 3, z, chunkBounds);
			placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 3, 3, z, chunkBounds);
			if (z <= 3 || z >= 7)
			{
				for (int_t x = 4; x <= 6; ++x)
					placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, x, 3, z, chunkBounds);
			}
			placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 7, 3, z, chunkBounds);
			placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, 8, 3, z, chunkBounds);
		}
		{
			const int_t ladderMeta = getMetadataWithOffset(Block::ladder->blockID, 4);
			for (int_t y = 1; y <= 3; ++y)
				placeBlockAtCurrentPosition(world, Block::ladder->blockID, ladderMeta, 9, y, 3, chunkBounds);
		}
		createTreasureChestAtCurrentPosition(world, chunkBounds, random, 3, 4, 8,
		                                     getRoomCrossingLoot(), 1 + random.nextInt(4));
		break;
	default:
		break;
	}
	return true;
}
