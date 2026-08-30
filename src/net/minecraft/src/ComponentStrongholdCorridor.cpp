#include "ComponentStrongholdCorridor.h"

#include "Block.h"
#include "StructureBoundingBox.h"
#include "java/Random.h"

ComponentStrongholdCorridor::ComponentStrongholdCorridor(
	int_t type, Random &, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: ComponentStronghold(type), field_35052_a(0)
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
	field_35052_a = direction != 2 && direction != 0 ? boundingBox->getXSize() : boundingBox->getZSize();
}

void ComponentStrongholdCorridor::buildComponent(StructureComponent *,
                                                  std::vector<StructureComponent *> &, Random &)
{
}

std::unique_ptr<StructureBoundingBox> ComponentStrongholdCorridor::func_35051_a(
	const std::vector<StructureComponent *> &components, Random &, int_t x, int_t y, int_t z,
	int_t direction)
{
	auto bounds = std::make_unique<StructureBoundingBox>(
		StructureBoundingBox::getComponentToAddBoundingBox(x, y, z, -1, -1, 0, 5, 5, 4, direction));
	StructureComponent *intersection = StructureComponent::findIntersecting(components, *bounds);
	if (intersection == nullptr || intersection->getBoundingBox() == nullptr)
		return nullptr;

	if (intersection->getBoundingBox()->minY == bounds->minY)
	{
		for (int_t length = 3; length >= 1; --length)
		{
			StructureBoundingBox shorter = StructureBoundingBox::getComponentToAddBoundingBox(
				x, y, z, -1, -1, 0, 5, 5, length - 1, direction);
			if (!intersection->getBoundingBox()->intersectsWith(shorter))
				return std::make_unique<StructureBoundingBox>(
					StructureBoundingBox::getComponentToAddBoundingBox(
						x, y, z, -1, -1, 0, 5, 5, length, direction));
		}
	}
	return nullptr;
}

bool ComponentStrongholdCorridor::addComponentParts(World *world, Random &,
                                                     const StructureBoundingBox &chunkBounds)
{
	if (isLiquidInStructureBoundingBox(world, chunkBounds))
		return false;
	for (int_t z = 0; z < field_35052_a; ++z)
	{
		for (int_t x = 0; x <= 4; ++x)
			placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, x, 0, z, chunkBounds);
		for (int_t y = 1; y <= 3; ++y)
		{
			placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 0, y, z, chunkBounds);
			for (int_t x = 1; x <= 3; ++x)
				placeBlockAtCurrentPosition(world, 0, 0, x, y, z, chunkBounds);
			placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, 4, y, z, chunkBounds);
		}
		for (int_t x = 0; x <= 4; ++x)
			placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, x, 4, z, chunkBounds);
	}
	return true;
}
