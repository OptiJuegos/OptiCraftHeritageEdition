#include "ComponentStrongholdStraight.h"

#include "Block.h"
#include "ComponentStrongholdStairs2.h"
#include "StructureBoundingBox.h"
#include "StructureStrongholdPieces.h"
#include "java/Random.h"

ComponentStrongholdStraight::ComponentStrongholdStraight(
	int_t type, Random &random, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: ComponentStronghold(type), doorType(getRandomDoor(random)), expandsX(false), expandsZ(false)
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
	expandsX = random.nextInt(2) == 0;
	expandsZ = random.nextInt(2) == 0;
}

void ComponentStrongholdStraight::buildComponent(StructureComponent *parent,
                                                  std::vector<StructureComponent *> &components,
                                                  Random &random)
{
	auto *start = static_cast<ComponentStrongholdStairs2 *>(parent);
	getNextComponentNormal(start, components, random, 1, 1);
	if (expandsX)
		getNextComponentX(start, components, random, 1, 2);
	if (expandsZ)
		getNextComponentZ(start, components, random, 1, 2);
}

ComponentStrongholdStraight *ComponentStrongholdStraight::findValidPlacement(
	const std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t type)
{
	auto bounds = std::make_unique<StructureBoundingBox>(
		StructureBoundingBox::getComponentToAddBoundingBox(x, y, z, -1, -1, 0, 5, 5, 7, direction));
	if (!canStrongholdGoDeeper(bounds.get()) || StructureComponent::findIntersecting(components, *bounds) != nullptr)
		return nullptr;
	return new ComponentStrongholdStraight(type, random, std::move(bounds), direction);
}

bool ComponentStrongholdStraight::addComponentParts(World *world, Random &random,
                                                     const StructureBoundingBox &chunkBounds)
{
	if (isLiquidInStructureBoundingBox(world, chunkBounds))
		return false;
	fillWithRandomizedBlocks(world, chunkBounds, 0, 0, 0, 4, 4, 6, true, random,
	                         StructureStrongholdPieces::getStrongholdStones());
	placeDoor(world, random, chunkBounds, doorType, 1, 1, 0);
	placeDoor(world, random, chunkBounds, EnumDoor::OPENING, 1, 1, 6);
	randomlyPlaceBlock(world, chunkBounds, random, 0.1F, 1, 2, 1, Block::torchWood->blockID, 0);
	randomlyPlaceBlock(world, chunkBounds, random, 0.1F, 3, 2, 1, Block::torchWood->blockID, 0);
	randomlyPlaceBlock(world, chunkBounds, random, 0.1F, 1, 2, 5, Block::torchWood->blockID, 0);
	randomlyPlaceBlock(world, chunkBounds, random, 0.1F, 3, 2, 5, Block::torchWood->blockID, 0);
	if (expandsX)
		fillWithBlocks(world, chunkBounds, 0, 1, 2, 0, 3, 4, 0, 0, false);
	if (expandsZ)
		fillWithBlocks(world, chunkBounds, 4, 1, 2, 4, 3, 4, 0, 0, false);
	return true;
}
