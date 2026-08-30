#include "ComponentStrongholdStairsStraight.h"

#include "Block.h"
#include "ComponentStrongholdStairs2.h"
#include "StructureBoundingBox.h"
#include "StructureStrongholdPieces.h"
#include "java/Random.h"

ComponentStrongholdStairsStraight::ComponentStrongholdStairsStraight(
	int_t type, Random &random, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: ComponentStronghold(type), doorType(getRandomDoor(random))
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
}

void ComponentStrongholdStairsStraight::buildComponent(
	StructureComponent *parent, std::vector<StructureComponent *> &components, Random &random)
{
	getNextComponentNormal(static_cast<ComponentStrongholdStairs2 *>(parent), components, random, 1, 1);
}

ComponentStrongholdStairsStraight *ComponentStrongholdStairsStraight::findValidPlacement(
	const std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t type)
{
	auto bounds = std::make_unique<StructureBoundingBox>(
		StructureBoundingBox::getComponentToAddBoundingBox(x, y, z, -1, -7, 0, 5, 11, 8, direction));
	if (!canStrongholdGoDeeper(bounds.get()) || StructureComponent::findIntersecting(components, *bounds) != nullptr)
		return nullptr;
	return new ComponentStrongholdStairsStraight(type, random, std::move(bounds), direction);
}

bool ComponentStrongholdStairsStraight::addComponentParts(
	World *world, Random &random, const StructureBoundingBox &chunkBounds)
{
	if (isLiquidInStructureBoundingBox(world, chunkBounds))
		return false;
	fillWithRandomizedBlocks(world, chunkBounds, 0, 0, 0, 4, 10, 7, true, random,
	                         StructureStrongholdPieces::getStrongholdStones());
	placeDoor(world, random, chunkBounds, doorType, 1, 7, 0);
	placeDoor(world, random, chunkBounds, EnumDoor::OPENING, 1, 1, 7);
	const int_t stairMeta = getMetadataWithOffset(Block::stairCompactCobblestone->blockID, 2);
	for (int_t step = 0; step < 6; ++step)
	{
		for (int_t x = 1; x <= 3; ++x)
			placeBlockAtCurrentPosition(world, Block::stairCompactCobblestone->blockID, stairMeta,
			                            x, 6 - step, 1 + step, chunkBounds);
		if (step < 5)
		{
			for (int_t x = 1; x <= 3; ++x)
				placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0,
				                            x, 5 - step, 1 + step, chunkBounds);
		}
	}
	return true;
}
