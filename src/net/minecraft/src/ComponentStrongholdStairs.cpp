#include "ComponentStrongholdStairs.h"

#include "Block.h"
#include "ComponentStrongholdCrossing.h"
#include "ComponentStrongholdStairs2.h"
#include "StructureBoundingBox.h"
#include "StructureStrongholdPieces.h"
#include "java/Random.h"

ComponentStrongholdStairs::ComponentStrongholdStairs(int_t type, Random &random, int_t x, int_t z)
	: ComponentStronghold(type), field_35036_a(true), doorType(EnumDoor::OPENING)
{
	coordBaseMode = random.nextInt(4);
	boundingBox = std::make_unique<StructureBoundingBox>(x, 64, z, x + 4, 74, z + 4);
}

ComponentStrongholdStairs::ComponentStrongholdStairs(
	int_t type, Random &random, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: ComponentStronghold(type), field_35036_a(false), doorType(getRandomDoor(random))
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
}

void ComponentStrongholdStairs::buildComponent(StructureComponent *parent,
                                                std::vector<StructureComponent *> &components,
                                                Random &random)
{
	if (field_35036_a)
		StructureStrongholdPieces::setComponentType(StrongholdPieceType::Crossing);
	getNextComponentNormal(static_cast<ComponentStrongholdStairs2 *>(parent), components, random, 1, 1);
}

ComponentStrongholdStairs *ComponentStrongholdStairs::getStrongholdStairsComponent(
	const std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t type)
{
	auto bounds = std::make_unique<StructureBoundingBox>(
		StructureBoundingBox::getComponentToAddBoundingBox(x, y, z, -1, -7, 0, 5, 11, 5, direction));
	if (!canStrongholdGoDeeper(bounds.get()) || StructureComponent::findIntersecting(components, *bounds) != nullptr)
		return nullptr;
	return new ComponentStrongholdStairs(type, random, std::move(bounds), direction);
}

bool ComponentStrongholdStairs::addComponentParts(World *world, Random &random,
                                                   const StructureBoundingBox &chunkBounds)
{
	if (isLiquidInStructureBoundingBox(world, chunkBounds))
		return false;
	fillWithRandomizedBlocks(world, chunkBounds, 0, 0, 0, 4, 10, 4, true, random,
	                         StructureStrongholdPieces::getStrongholdStones());
	placeDoor(world, random, chunkBounds, doorType, 1, 7, 0);
	placeDoor(world, random, chunkBounds, EnumDoor::OPENING, 1, 1, 4);
	placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 2, 6, 1, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 1, 5, 1, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stairSingle->blockID, 0, 1, 6, 1, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 1, 5, 2, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 1, 4, 3, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stairSingle->blockID, 0, 1, 5, 3, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 2, 4, 3, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 3, 3, 3, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stairSingle->blockID, 0, 3, 4, 3, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 3, 3, 2, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 3, 2, 1, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stairSingle->blockID, 0, 3, 3, 1, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 2, 2, 1, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 1, 1, 1, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stairSingle->blockID, 0, 1, 2, 1, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 1, 1, 2, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::stairSingle->blockID, 0, 1, 1, 3, chunkBounds);
	return true;
}
