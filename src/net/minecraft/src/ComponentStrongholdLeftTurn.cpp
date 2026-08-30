#include "ComponentStrongholdLeftTurn.h"

#include "ComponentStrongholdStairs2.h"
#include "StructureBoundingBox.h"
#include "StructureStrongholdPieces.h"
#include "java/Random.h"

ComponentStrongholdLeftTurn::ComponentStrongholdLeftTurn(
	int_t type, Random &random, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: ComponentStronghold(type), doorType(getRandomDoor(random))
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
}

void ComponentStrongholdLeftTurn::buildComponent(StructureComponent *parent,
                                                  std::vector<StructureComponent *> &components,
                                                  Random &random)
{
	auto *start = static_cast<ComponentStrongholdStairs2 *>(parent);
	if (coordBaseMode != 2 && coordBaseMode != 3)
		getNextComponentZ(start, components, random, 1, 1);
	else
		getNextComponentX(start, components, random, 1, 1);
}

ComponentStrongholdLeftTurn *ComponentStrongholdLeftTurn::findValidPlacement(
	const std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t type)
{
	auto bounds = std::make_unique<StructureBoundingBox>(
		StructureBoundingBox::getComponentToAddBoundingBox(x, y, z, -1, -1, 0, 5, 5, 5, direction));
	if (!canStrongholdGoDeeper(bounds.get()) || StructureComponent::findIntersecting(components, *bounds) != nullptr)
		return nullptr;
	return new ComponentStrongholdLeftTurn(type, random, std::move(bounds), direction);
}

bool ComponentStrongholdLeftTurn::addComponentParts(World *world, Random &random,
                                                     const StructureBoundingBox &chunkBounds)
{
	if (isLiquidInStructureBoundingBox(world, chunkBounds))
		return false;
	fillWithRandomizedBlocks(world, chunkBounds, 0, 0, 0, 4, 4, 4, true, random,
	                         StructureStrongholdPieces::getStrongholdStones());
	placeDoor(world, random, chunkBounds, doorType, 1, 1, 0);
	if (coordBaseMode != 2 && coordBaseMode != 3)
		fillWithBlocks(world, chunkBounds, 4, 1, 1, 4, 3, 3, 0, 0, false);
	else
		fillWithBlocks(world, chunkBounds, 0, 1, 1, 0, 3, 3, 0, 0, false);
	return true;
}
