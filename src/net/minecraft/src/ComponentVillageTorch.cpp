#include "ComponentVillageTorch.h"

#include "Block.h"
#include "StructureBoundingBox.h"
#include "World.h"
#include "java/Random.h"

ComponentVillageTorch::ComponentVillageTorch(
	int_t type, Random &, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: ComponentVillage(type), averageGroundLevel(-1)
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
}

void ComponentVillageTorch::buildComponent(
	StructureComponent *, std::vector<StructureComponent *> &, Random &)
{
}

std::unique_ptr<StructureBoundingBox> ComponentVillageTorch::findValidPlacement(
	const std::vector<StructureComponent *> &components, Random &,
	int_t x, int_t y, int_t z, int_t direction)
{
	auto bounds = std::make_unique<StructureBoundingBox>(
		StructureBoundingBox::getComponentToAddBoundingBox(
			x, y, z, 0, 0, 0, 3, 4, 2, direction));
	return StructureComponent::findIntersecting(components, *bounds) != nullptr
		? nullptr : std::move(bounds);
}

bool ComponentVillageTorch::addComponentParts(
	World *world, Random &, const StructureBoundingBox &chunkBounds)
{
	if (averageGroundLevel < 0)
	{
		averageGroundLevel = getAverageGroundLevel(world, chunkBounds);
		if (averageGroundLevel < 0)
			return true;
		boundingBox->offset(0, averageGroundLevel - boundingBox->maxY + 3, 0);
	}

	fillWithBlocks(world, chunkBounds, 0, 0, 0, 2, 3, 1, 0, 0, false);
	placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, 1, 0, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, 1, 1, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::fence->blockID, 0, 1, 2, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::cloth->blockID, 15, 1, 3, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 15, 0, 3, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 15, 1, 3, 1, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 15, 2, 3, 0, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::torchWood->blockID, 15, 1, 3, -1, chunkBounds);
	return true;
}
