#include "ComponentStrongholdPrison.h"

#include "Block.h"
#include "ComponentStrongholdStairs2.h"
#include "StructureBoundingBox.h"
#include "StructureStrongholdPieces.h"
#include "java/Random.h"

ComponentStrongholdPrison::ComponentStrongholdPrison(
	int_t type, Random &random, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: ComponentStronghold(type), doorType(getRandomDoor(random))
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
}

void ComponentStrongholdPrison::buildComponent(StructureComponent *parent,
                                                std::vector<StructureComponent *> &components,
                                                Random &random)
{
	getNextComponentNormal(static_cast<ComponentStrongholdStairs2 *>(parent), components, random, 1, 1);
}

ComponentStrongholdPrison *ComponentStrongholdPrison::findValidPlacement(
	const std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t type)
{
	auto bounds = std::make_unique<StructureBoundingBox>(
		StructureBoundingBox::getComponentToAddBoundingBox(x, y, z, -1, -1, 0, 9, 5, 11, direction));
	if (!canStrongholdGoDeeper(bounds.get()) || StructureComponent::findIntersecting(components, *bounds) != nullptr)
		return nullptr;
	return new ComponentStrongholdPrison(type, random, std::move(bounds), direction);
}

bool ComponentStrongholdPrison::addComponentParts(World *world, Random &random,
                                                   const StructureBoundingBox &chunkBounds)
{
	if (isLiquidInStructureBoundingBox(world, chunkBounds))
		return false;
	fillWithRandomizedBlocks(world, chunkBounds, 0, 0, 0, 8, 4, 10, true, random,
	                         StructureStrongholdPieces::getStrongholdStones());
	placeDoor(world, random, chunkBounds, doorType, 1, 1, 0);
	fillWithBlocks(world, chunkBounds, 1, 1, 10, 3, 3, 10, 0, 0, false);
	fillWithRandomizedBlocks(world, chunkBounds, 4, 1, 1, 4, 3, 1, false, random, StructureStrongholdPieces::getStrongholdStones());
	fillWithRandomizedBlocks(world, chunkBounds, 4, 1, 3, 4, 3, 3, false, random, StructureStrongholdPieces::getStrongholdStones());
	fillWithRandomizedBlocks(world, chunkBounds, 4, 1, 7, 4, 3, 7, false, random, StructureStrongholdPieces::getStrongholdStones());
	fillWithRandomizedBlocks(world, chunkBounds, 4, 1, 9, 4, 3, 9, false, random, StructureStrongholdPieces::getStrongholdStones());
	fillWithBlocks(world, chunkBounds, 4, 1, 4, 4, 3, 6, Block::fenceIron->blockID, Block::fenceIron->blockID, false);
	fillWithBlocks(world, chunkBounds, 5, 1, 5, 7, 3, 5, Block::fenceIron->blockID, Block::fenceIron->blockID, false);
	placeBlockAtCurrentPosition(world, Block::fenceIron->blockID, 0, 4, 3, 2, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::fenceIron->blockID, 0, 4, 3, 8, chunkBounds);
	const int_t doorMeta = getMetadataWithOffset(Block::doorSteel->blockID, 3);
	placeBlockAtCurrentPosition(world, Block::doorSteel->blockID, doorMeta, 4, 1, 2, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::doorSteel->blockID, doorMeta + 8, 4, 2, 2, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::doorSteel->blockID, doorMeta, 4, 1, 8, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::doorSteel->blockID, doorMeta + 8, 4, 2, 8, chunkBounds);
	return true;
}
