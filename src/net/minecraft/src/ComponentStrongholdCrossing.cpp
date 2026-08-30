#include "ComponentStrongholdCrossing.h"

#include "Block.h"
#include "ComponentStrongholdStairs2.h"
#include "StructureBoundingBox.h"
#include "StructureStrongholdPieces.h"
#include "java/Random.h"

ComponentStrongholdCrossing::ComponentStrongholdCrossing(
	int_t type, Random &random, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: ComponentStronghold(type), doorType(getRandomDoor(random)),
	  field_35042_b(random.nextBoolean()), field_35043_c(random.nextBoolean()),
	  field_35040_d(random.nextBoolean()), field_35041_e(random.nextInt(3) > 0)
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
}

void ComponentStrongholdCrossing::buildComponent(StructureComponent *parent,
                                                  std::vector<StructureComponent *> &components,
                                                  Random &random)
{
	auto *start = static_cast<ComponentStrongholdStairs2 *>(parent);
	int_t lower = 3;
	int_t upper = 5;
	if (coordBaseMode == 1 || coordBaseMode == 2)
	{
		lower = 8 - lower;
		upper = 8 - upper;
	}
	getNextComponentNormal(start, components, random, 5, 1);
	if (field_35042_b) getNextComponentX(start, components, random, lower, 1);
	if (field_35043_c) getNextComponentX(start, components, random, upper, 7);
	if (field_35040_d) getNextComponentZ(start, components, random, lower, 1);
	if (field_35041_e) getNextComponentZ(start, components, random, upper, 7);
}

ComponentStrongholdCrossing *ComponentStrongholdCrossing::findValidPlacement(
	const std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t type)
{
	auto bounds = std::make_unique<StructureBoundingBox>(
		StructureBoundingBox::getComponentToAddBoundingBox(x, y, z, -4, -3, 0, 10, 9, 11, direction));
	if (!canStrongholdGoDeeper(bounds.get()) || StructureComponent::findIntersecting(components, *bounds) != nullptr)
		return nullptr;
	return new ComponentStrongholdCrossing(type, random, std::move(bounds), direction);
}

bool ComponentStrongholdCrossing::addComponentParts(World *world, Random &random,
                                                     const StructureBoundingBox &chunkBounds)
{
	if (isLiquidInStructureBoundingBox(world, chunkBounds)) return false;
	fillWithRandomizedBlocks(world, chunkBounds, 0, 0, 0, 9, 8, 10, true, random, StructureStrongholdPieces::getStrongholdStones());
	placeDoor(world, random, chunkBounds, doorType, 4, 3, 0);
	if (field_35042_b) fillWithBlocks(world, chunkBounds, 0, 3, 1, 0, 5, 3, 0, 0, false);
	if (field_35040_d) fillWithBlocks(world, chunkBounds, 9, 3, 1, 9, 5, 3, 0, 0, false);
	if (field_35043_c) fillWithBlocks(world, chunkBounds, 0, 5, 7, 0, 7, 9, 0, 0, false);
	if (field_35041_e) fillWithBlocks(world, chunkBounds, 9, 5, 7, 9, 7, 9, 0, 0, false);
	fillWithBlocks(world, chunkBounds, 5, 1, 10, 7, 3, 10, 0, 0, false);
	fillWithRandomizedBlocks(world, chunkBounds, 1, 2, 1, 8, 2, 6, false, random, StructureStrongholdPieces::getStrongholdStones());
	fillWithRandomizedBlocks(world, chunkBounds, 4, 1, 5, 4, 4, 9, false, random, StructureStrongholdPieces::getStrongholdStones());
	fillWithRandomizedBlocks(world, chunkBounds, 8, 1, 5, 8, 4, 9, false, random, StructureStrongholdPieces::getStrongholdStones());
	fillWithRandomizedBlocks(world, chunkBounds, 1, 4, 7, 3, 4, 9, false, random, StructureStrongholdPieces::getStrongholdStones());
	fillWithRandomizedBlocks(world, chunkBounds, 1, 3, 5, 3, 3, 6, false, random, StructureStrongholdPieces::getStrongholdStones());
	fillWithBlocks(world, chunkBounds, 1, 3, 4, 3, 3, 4, Block::stairSingle->blockID, Block::stairSingle->blockID, false);
	fillWithBlocks(world, chunkBounds, 1, 4, 6, 3, 4, 6, Block::stairSingle->blockID, Block::stairSingle->blockID, false);
	fillWithRandomizedBlocks(world, chunkBounds, 5, 1, 7, 7, 1, 8, false, random, StructureStrongholdPieces::getStrongholdStones());
	fillWithBlocks(world, chunkBounds, 5, 1, 9, 7, 1, 9, Block::stairSingle->blockID, Block::stairSingle->blockID, false);
	fillWithBlocks(world, chunkBounds, 5, 2, 7, 7, 2, 7, Block::stairSingle->blockID, Block::stairSingle->blockID, false);
	fillWithBlocks(world, chunkBounds, 4, 5, 7, 4, 5, 9, Block::stairSingle->blockID, Block::stairSingle->blockID, false);
	fillWithBlocks(world, chunkBounds, 8, 5, 7, 8, 5, 9, Block::stairSingle->blockID, Block::stairSingle->blockID, false);
	fillWithBlocks(world, chunkBounds, 5, 5, 7, 7, 5, 9, Block::stairDouble->blockID, Block::stairDouble->blockID, false);
	placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 0, 6, 5, 6, chunkBounds);
	return true;
}
