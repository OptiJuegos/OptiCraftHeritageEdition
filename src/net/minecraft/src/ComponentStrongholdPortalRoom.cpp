#include "ComponentStrongholdPortalRoom.h"

#include "Block.h"
#include "ComponentStrongholdStairs2.h"
#include "StructureBoundingBox.h"
#include "StructureStrongholdPieces.h"
#include "TileEntity.h"
#include "TileEntityMobSpawner.h"
#include "World.h"
#include "java/Random.h"

ComponentStrongholdPortalRoom::ComponentStrongholdPortalRoom(
	int_t type, Random &, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: ComponentStronghold(type)
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
}

void ComponentStrongholdPortalRoom::buildComponent(
	StructureComponent *parent, std::vector<StructureComponent *> &, Random &)
{
	if (parent != nullptr)
		static_cast<ComponentStrongholdStairs2 *>(parent)->portalRoom = this;
}

ComponentStrongholdPortalRoom *ComponentStrongholdPortalRoom::findValidPlacement(
	const std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction, int_t type)
{
	auto bounds = std::make_unique<StructureBoundingBox>(
		StructureBoundingBox::getComponentToAddBoundingBox(x, y, z, -4, -1, 0, 11, 8, 16, direction));
	if (!canStrongholdGoDeeper(bounds.get()) || StructureComponent::findIntersecting(components, *bounds) != nullptr)
		return nullptr;
	return new ComponentStrongholdPortalRoom(type, random, std::move(bounds), direction);
}

bool ComponentStrongholdPortalRoom::addComponentParts(
	World *world, Random &random, const StructureBoundingBox &chunkBounds)
{
	fillWithRandomizedBlocks(world, chunkBounds, 0, 0, 0, 10, 7, 15, false, random,
	                         StructureStrongholdPieces::getStrongholdStones());
	placeDoor(world, random, chunkBounds, EnumDoor::GRATES, 4, 1, 0);
	const int_t ceilingY = 6;
	fillWithRandomizedBlocks(world, chunkBounds, 1, ceilingY, 1, 1, ceilingY, 14, false, random, StructureStrongholdPieces::getStrongholdStones());
	fillWithRandomizedBlocks(world, chunkBounds, 9, ceilingY, 1, 9, ceilingY, 14, false, random, StructureStrongholdPieces::getStrongholdStones());
	fillWithRandomizedBlocks(world, chunkBounds, 2, ceilingY, 1, 8, ceilingY, 2, false, random, StructureStrongholdPieces::getStrongholdStones());
	fillWithRandomizedBlocks(world, chunkBounds, 2, ceilingY, 14, 8, ceilingY, 14, false, random, StructureStrongholdPieces::getStrongholdStones());
	fillWithRandomizedBlocks(world, chunkBounds, 1, 1, 1, 2, 1, 4, false, random, StructureStrongholdPieces::getStrongholdStones());
	fillWithRandomizedBlocks(world, chunkBounds, 8, 1, 1, 9, 1, 4, false, random, StructureStrongholdPieces::getStrongholdStones());
	fillWithBlocks(world, chunkBounds, 1, 1, 1, 1, 1, 3, Block::lavaMoving->blockID, Block::lavaMoving->blockID, false);
	fillWithBlocks(world, chunkBounds, 9, 1, 1, 9, 1, 3, Block::lavaMoving->blockID, Block::lavaMoving->blockID, false);
	fillWithRandomizedBlocks(world, chunkBounds, 3, 1, 8, 7, 1, 12, false, random, StructureStrongholdPieces::getStrongholdStones());
	fillWithBlocks(world, chunkBounds, 4, 1, 9, 6, 1, 11, Block::lavaMoving->blockID, Block::lavaMoving->blockID, false);

	for (int_t z = 3; z < 14; z += 2)
	{
		fillWithBlocks(world, chunkBounds, 0, 3, z, 0, 4, z, Block::fenceIron->blockID, Block::fenceIron->blockID, false);
		fillWithBlocks(world, chunkBounds, 10, 3, z, 10, 4, z, Block::fenceIron->blockID, Block::fenceIron->blockID, false);
	}
	for (int_t x = 2; x < 9; x += 2)
		fillWithBlocks(world, chunkBounds, x, 3, 15, x, 4, 15, Block::fenceIron->blockID, Block::fenceIron->blockID, false);

	const int_t stairMeta = getMetadataWithOffset(Block::stairsStoneBrickSmooth->blockID, 3);
	fillWithRandomizedBlocks(world, chunkBounds, 4, 1, 5, 6, 1, 7, false, random, StructureStrongholdPieces::getStrongholdStones());
	fillWithRandomizedBlocks(world, chunkBounds, 4, 2, 6, 6, 2, 7, false, random, StructureStrongholdPieces::getStrongholdStones());
	fillWithRandomizedBlocks(world, chunkBounds, 4, 3, 7, 6, 3, 7, false, random, StructureStrongholdPieces::getStrongholdStones());
	for (int_t x = 4; x <= 6; ++x)
	{
		placeBlockAtCurrentPosition(world, Block::stairsStoneBrickSmooth->blockID, stairMeta, x, 1, 4, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::stairsStoneBrickSmooth->blockID, stairMeta, x, 2, 5, chunkBounds);
		placeBlockAtCurrentPosition(world, Block::stairsStoneBrickSmooth->blockID, stairMeta, x, 3, 6, chunkBounds);
	}

	int_t north = 2;
	int_t south = 0;
	int_t west = 3;
	int_t east = 1;
	switch (coordBaseMode)
	{
	case 0:
		north = 0;
		south = 2;
		break;
	case 1:
		north = 1;
		south = 3;
		west = 0;
		east = 2;
		break;
	case 3:
		north = 3;
		south = 1;
		west = 0;
		east = 2;
		break;
	case 2:
	default:
		break;
	}

	auto frameMeta = [&random](int_t direction) {
		return direction + (random.nextFloat() > 0.9F ? 4 : 0);
	};
	placeBlockAtCurrentPosition(world, Block::endPortalFrame->blockID, frameMeta(north), 4, 3, 8, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::endPortalFrame->blockID, frameMeta(north), 5, 3, 8, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::endPortalFrame->blockID, frameMeta(north), 6, 3, 8, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::endPortalFrame->blockID, frameMeta(south), 4, 3, 12, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::endPortalFrame->blockID, frameMeta(south), 5, 3, 12, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::endPortalFrame->blockID, frameMeta(south), 6, 3, 12, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::endPortalFrame->blockID, frameMeta(west), 3, 3, 9, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::endPortalFrame->blockID, frameMeta(west), 3, 3, 10, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::endPortalFrame->blockID, frameMeta(west), 3, 3, 11, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::endPortalFrame->blockID, frameMeta(east), 7, 3, 9, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::endPortalFrame->blockID, frameMeta(east), 7, 3, 10, chunkBounds);
	placeBlockAtCurrentPosition(world, Block::endPortalFrame->blockID, frameMeta(east), 7, 3, 11, chunkBounds);

	if (!hasSpawner)
	{
		const int_t worldY = getYWithOffset(3);
		const int_t worldX = getXWithOffset(5, 6);
		const int_t worldZ = getZWithOffset(5, 6);
		if (chunkBounds.isVecInside(worldX, worldY, worldZ))
		{
			hasSpawner = true;
			world->setBlockWithNotify(worldX, worldY, worldZ, Block::mobSpawner->blockID);
			auto *spawner = dynamic_cast<TileEntityMobSpawner *>(world->getBlockTileEntity(worldX, worldY, worldZ));
			if (spawner != nullptr)
				spawner->setMobID("Silverfish");
		}
	}
	return true;
}
