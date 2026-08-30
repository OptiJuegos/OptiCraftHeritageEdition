#include "ComponentStronghold.h"

#include "Block.h"
#include "ComponentStrongholdStairs2.h"
#include "StructureBoundingBox.h"
#include "StructureStrongholdPieces.h"
#include "World.h"
#include "java/Random.h"

ComponentStronghold::ComponentStronghold(int_t componentType)
	: StructureComponent(componentType)
{
}

void ComponentStronghold::placeDoor(World *world, Random &, const StructureBoundingBox &chunkBounds,
                                    EnumDoor doorType, int_t x, int_t y, int_t z) const
{
	switch (doorType)
	{
	case EnumDoor::OPENING:
	default:
		fillWithBlocks(world, chunkBounds, x, y, z, x + 2, y + 2, z, 0, 0, false);
		break;
	case EnumDoor::WOOD_DOOR:
		placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, x, y, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, x, y + 1, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, x, y + 2, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, x + 1, y + 2, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, x + 2, y + 2, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, x + 2, y + 1, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, x + 2, y, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::doorWood->blockID, 0, x + 1, y, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::doorWood->blockID, 8, x + 1, y + 1, z, chunkBounds);
		break;
	case EnumDoor::GRATES:
		placeBlockAtCurrentPosition(world, 0, 0, x + 1, y, z, chunkBounds);
		placeBlockAtCurrentPosition(world, 0, 0, x + 1, y + 1, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::fenceIron->blockID, 0, x, y, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::fenceIron->blockID, 0, x, y + 1, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::fenceIron->blockID, 0, x, y + 2, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::fenceIron->blockID, 0, x + 1, y + 2, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::fenceIron->blockID, 0, x + 2, y + 2, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::fenceIron->blockID, 0, x + 2, y + 1, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::fenceIron->blockID, 0, x + 2, y, z, chunkBounds);
		break;
	case EnumDoor::IRON_DOOR:
		placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, x, y, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, x, y + 1, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, x, y + 2, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, x + 1, y + 2, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, x + 2, y + 2, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, x + 2, y + 1, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::stoneBrick->blockID, 0, x + 2, y, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::doorSteel->blockID, 0, x + 1, y, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::doorSteel->blockID, 8, x + 1, y + 1, z, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::button->blockID,
		                            getMetadataWithOffset(Block::button->blockID, 4),
		                            x + 2, y + 1, z + 1, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::button->blockID,
		                            getMetadataWithOffset(Block::button->blockID, 3),
		                            x + 2, y + 1, z - 1, chunkBounds);
		break;
	}
}

EnumDoor ComponentStronghold::getRandomDoor(Random &random) const
{
	switch (random.nextInt(5))
	{
	case 2:
		return EnumDoor::WOOD_DOOR;
	case 3:
		return EnumDoor::GRATES;
	case 4:
		return EnumDoor::IRON_DOOR;
	case 0:
	case 1:
	default:
		return EnumDoor::OPENING;
	}
}

StructureComponent *ComponentStronghold::getNextComponentNormal(
	ComponentStrongholdStairs2 *start, std::vector<StructureComponent *> &components,
	Random &random, int_t xOffset, int_t yOffset)
{
	if (start == nullptr || boundingBox == nullptr)
		return nullptr;
	switch (coordBaseMode)
	{
	case 0:
		return StructureStrongholdPieces::getNextValidComponentAccess(start, components, random,
			boundingBox->minX + xOffset, boundingBox->minY + yOffset,
			boundingBox->maxZ + 1, coordBaseMode, getComponentType());
	case 1:
		return StructureStrongholdPieces::getNextValidComponentAccess(start, components, random,
			boundingBox->minX - 1, boundingBox->minY + yOffset,
			boundingBox->minZ + xOffset, coordBaseMode, getComponentType());
	case 2:
		return StructureStrongholdPieces::getNextValidComponentAccess(start, components, random,
			boundingBox->minX + xOffset, boundingBox->minY + yOffset,
			boundingBox->minZ - 1, coordBaseMode, getComponentType());
	case 3:
		return StructureStrongholdPieces::getNextValidComponentAccess(start, components, random,
			boundingBox->maxX + 1, boundingBox->minY + yOffset,
			boundingBox->minZ + xOffset, coordBaseMode, getComponentType());
	default:
		return nullptr;
	}
}

StructureComponent *ComponentStronghold::getNextComponentX(
	ComponentStrongholdStairs2 *start, std::vector<StructureComponent *> &components,
	Random &random, int_t yOffset, int_t zOffset)
{
	if (start == nullptr || boundingBox == nullptr)
		return nullptr;
	switch (coordBaseMode)
	{
	case 0:
	case 2:
		return StructureStrongholdPieces::getNextValidComponentAccess(start, components, random,
			boundingBox->minX - 1, boundingBox->minY + yOffset,
			boundingBox->minZ + zOffset, 1, getComponentType());
	case 1:
	case 3:
		return StructureStrongholdPieces::getNextValidComponentAccess(start, components, random,
			boundingBox->minX + zOffset, boundingBox->minY + yOffset,
			boundingBox->minZ - 1, 2, getComponentType());
	default:
		return nullptr;
	}
}

StructureComponent *ComponentStronghold::getNextComponentZ(
	ComponentStrongholdStairs2 *start, std::vector<StructureComponent *> &components,
	Random &random, int_t yOffset, int_t xOffset)
{
	if (start == nullptr || boundingBox == nullptr)
		return nullptr;
	switch (coordBaseMode)
	{
	case 0:
	case 2:
		return StructureStrongholdPieces::getNextValidComponentAccess(start, components, random,
			boundingBox->maxX + 1, boundingBox->minY + yOffset,
			boundingBox->minZ + xOffset, 3, getComponentType());
	case 1:
	case 3:
		return StructureStrongholdPieces::getNextValidComponentAccess(start, components, random,
			boundingBox->minX + xOffset, boundingBox->minY + yOffset,
			boundingBox->maxZ + 1, 0, getComponentType());
	default:
		return nullptr;
	}
}

bool ComponentStronghold::canStrongholdGoDeeper(const StructureBoundingBox *bounds)
{
	return bounds != nullptr && bounds->minY > 10;
}
