#include "ComponentNetherBridgeCrossing2.h"

#include <memory>

#include "Block.h"
#include "ComponentNetherBridgeStartPiece.h"
#include "StructureBoundingBox.h"
#include "World.h"
#include "java/Random.h"

ComponentNetherBridgeCrossing2::ComponentNetherBridgeCrossing2(
	int_t type, Random &, const StructureBoundingBox &bounds, int_t direction)
	: ComponentNetherBridgePiece(type)
{
	coordBaseMode = direction;
	boundingBox = std::make_unique<StructureBoundingBox>(bounds);
}

void ComponentNetherBridgeCrossing2::buildComponent(
	StructureComponent *parent, std::vector<StructureComponent *> &components, Random &random)
{
	auto *start = static_cast<ComponentNetherBridgeStartPiece *>(parent);
	getNextComponentNormal(start, components, random, 1, 0, true);
	getNextComponentX(start, components, random, 0, 1, true);
	getNextComponentZ(start, components, random, 0, 1, true);
}

ComponentNetherBridgeCrossing2 *ComponentNetherBridgeCrossing2::createValidComponent(
	std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t depth)
{
	const StructureBoundingBox bounds = StructureBoundingBox::getComponentToAddBoundingBox(
		x, y, z, -1, 0, 0, 5, 7, 5, direction);
	return isAboveGround(&bounds) && StructureComponent::findIntersecting(components, bounds) == nullptr
		? new ComponentNetherBridgeCrossing2(depth, random, bounds, direction)
		: nullptr;
}

bool ComponentNetherBridgeCrossing2::addComponentParts(
	World *world, Random &, const StructureBoundingBox &b)
{
	const int_t brick = Block::netherBrick->blockID;
	fillWithBlocks(world, b, 0, 0, 0, 4, 1, 4, brick, brick, false);
	fillWithBlocks(world, b, 0, 2, 0, 4, 5, 4, 0, 0, false);
	fillWithBlocks(world, b, 0, 2, 0, 0, 5, 0, brick, brick, false);
	fillWithBlocks(world, b, 4, 2, 0, 4, 5, 0, brick, brick, false);
	fillWithBlocks(world, b, 0, 2, 4, 0, 5, 4, brick, brick, false);
	fillWithBlocks(world, b, 4, 2, 4, 4, 5, 4, brick, brick, false);
	fillWithBlocks(world, b, 0, 6, 0, 4, 6, 4, brick, brick, false);
	for (int_t x = 0; x <= 4; ++x)
		for (int_t z = 0; z <= 4; ++z)
			fillCurrentPositionBlocksDownwards(world, brick, 0, x, -1, z, b);
	return true;
}
