#include "ComponentStrongholdChestCorridor.h"

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
const std::vector<StructurePieceTreasure> &getChestLoot()
{
	static const std::vector<StructurePieceTreasure> loot = {
		StructurePieceTreasure(112 + 256, 0, 1, 1, 10),
		StructurePieceTreasure(Item::diamond->shiftedIndex, 0, 1, 3, 3),
		StructurePieceTreasure(Item::ingotIron->shiftedIndex, 0, 1, 5, 10),
		StructurePieceTreasure(Item::ingotGold->shiftedIndex, 0, 1, 3, 5),
		StructurePieceTreasure(Item::redstone->shiftedIndex, 0, 4, 9, 5),
		StructurePieceTreasure(Item::bread->shiftedIndex, 0, 1, 3, 15),
		StructurePieceTreasure(Item::appleRed->shiftedIndex, 0, 1, 3, 15),
		StructurePieceTreasure(Item::pickaxeSteel->shiftedIndex, 0, 1, 1, 5),
		StructurePieceTreasure(Item::swordSteel->shiftedIndex, 0, 1, 1, 5),
		StructurePieceTreasure(Item::plateSteel->shiftedIndex, 0, 1, 1, 5),
		StructurePieceTreasure(Item::helmetSteel->shiftedIndex, 0, 1, 1, 5),
		StructurePieceTreasure(Item::legsSteel->shiftedIndex, 0, 1, 1, 5),
		StructurePieceTreasure(Item::bootsSteel->shiftedIndex, 0, 1, 1, 5),
		StructurePieceTreasure(Item::appleGold->shiftedIndex, 0, 1, 1, 1)
	};
	return loot;
}
}

ComponentStrongholdChestCorridor::ComponentStrongholdChestCorridor(
	int_t type, Random &random, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: ComponentStronghold(type), doorType(getRandomDoor(random))
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
}

void ComponentStrongholdChestCorridor::buildComponent(
	StructureComponent *parent, std::vector<StructureComponent *> &components, Random &random)
{
	getNextComponentNormal(static_cast<ComponentStrongholdStairs2 *>(parent), components, random, 1, 1);
}

ComponentStrongholdChestCorridor *ComponentStrongholdChestCorridor::findValidPlacement(
	const std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t type)
{
	auto bounds = std::make_unique<StructureBoundingBox>(
		StructureBoundingBox::getComponentToAddBoundingBox(x, y, z, -1, -1, 0, 5, 5, 7, direction));
	if (!canStrongholdGoDeeper(bounds.get()) || StructureComponent::findIntersecting(components, *bounds) != nullptr)
		return nullptr;
	return new ComponentStrongholdChestCorridor(type, random, std::move(bounds), direction);
}

bool ComponentStrongholdChestCorridor::addComponentParts(
	World *world, Random &random, const StructureBoundingBox &chunkBounds)
{
	if (isLiquidInStructureBoundingBox(world, chunkBounds))
		return false;
	fillWithRandomizedBlocks(world, chunkBounds, 0, 0, 0, 4, 4, 6, true, random,
	                         StructureStrongholdPieces::getStrongholdStones());
	placeDoor(world, random, chunkBounds, doorType, 1, 1, 0);
	placeDoor(world, random, chunkBounds, EnumDoor::OPENING, 1, 1, 6);
	fillWithBlocks(world, chunkBounds, 3, 1, 2, 3, 1, 4,
	               Block::stoneBrick->blockID, Block::stoneBrick->blockID, false);
	placeBlockAtCurrentPosition(world, Block::stairSingle->blockID, 5, 3, 1, 1, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stairSingle->blockID, 5, 3, 1, 5, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stairSingle->blockID, 5, 3, 2, 2, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stairSingle->blockID, 5, 3, 2, 4, chunkBounds);
	for (int_t z = 2; z <= 4; ++z)
		placeBlockAtCurrentPosition(world, Block::stairSingle->blockID, 5, 2, 1, z, chunkBounds);

	if (!hasMadeChest)
	{
		const int_t worldY = getYWithOffset(2);
		const int_t worldX = getXWithOffset(3, 3);
		const int_t worldZ = getZWithOffset(3, 3);
		if (chunkBounds.isVecInside(worldX, worldY, worldZ))
		{
			hasMadeChest = true;
			createTreasureChestAtCurrentPosition(world, chunkBounds, random, 3, 2, 3,
			                                     getChestLoot(), 2 + random.nextInt(2));
		}
	}
	return true;
}
