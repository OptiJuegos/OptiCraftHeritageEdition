#include "ComponentMineshaftCorridor.h"

#include "Block.h"
#include "StructureBoundingBox.h"
#include "StructureMineshaftPieces.h"
#include "TileEntityMobSpawner.h"
#include "World.h"
#include "java/Random.h"

ComponentMineshaftCorridor::ComponentMineshaftCorridor(
	int_t type, Random &random, std::unique_ptr<StructureBoundingBox> bounds, int_t direction)
	: StructureComponent(type),
	  hasRails(random.nextInt(3) == 0),
	  hasSpiders(false)
{
	coordBaseMode = direction;
	boundingBox = std::move(bounds);
	hasSpiders = !hasRails && random.nextInt(23) == 0;
	if (boundingBox)
		sectionCount = (coordBaseMode == 0 || coordBaseMode == 2)
			? boundingBox->getZSize() / 5
			: boundingBox->getXSize() / 5;
}

std::unique_ptr<StructureBoundingBox> ComponentMineshaftCorridor::findValidPlacement(
	const std::vector<StructureComponent *> &components, Random &random,
	int_t x, int_t y, int_t z, int_t direction)
{
	auto bounds = std::make_unique<StructureBoundingBox>(x, y, z, x, y + 2, z);
	int_t sections;
	for (sections = random.nextInt(3) + 2; sections > 0; --sections)
	{
		const int_t length = sections * 5;
		switch (direction)
		{
		case 0:
			bounds->maxX = x + 2;
			bounds->maxZ = z + length - 1;
			break;
		case 1:
			bounds->minX = x - (length - 1);
			bounds->maxZ = z + 2;
			break;
		case 2:
			bounds->maxX = x + 2;
			bounds->minZ = z - (length - 1);
			break;
		case 3:
			bounds->maxX = x + length - 1;
			bounds->maxZ = z + 2;
			break;
		default:
			break;
		}
		if (StructureComponent::findIntersecting(components, *bounds) == nullptr)
			break;
	}
	return sections > 0 ? std::move(bounds) : nullptr;
}

void ComponentMineshaftCorridor::buildComponent(
	StructureComponent *parent, std::vector<StructureComponent *> &components, Random &random)
{
	if (!boundingBox)
		return;
	const int_t type = getComponentType();
	const int_t branch = random.nextInt(4);
	switch (coordBaseMode)
	{
	case 0:
		if (branch <= 1)
			StructureMineshaftPieces::getNextComponent(parent, components, random,
				boundingBox->minX, boundingBox->minY - 1 + random.nextInt(3), boundingBox->maxZ + 1,
				coordBaseMode, type);
		else if (branch == 2)
			StructureMineshaftPieces::getNextComponent(parent, components, random,
				boundingBox->minX - 1, boundingBox->minY - 1 + random.nextInt(3), boundingBox->maxZ - 3,
				1, type);
		else
			StructureMineshaftPieces::getNextComponent(parent, components, random,
				boundingBox->maxX + 1, boundingBox->minY - 1 + random.nextInt(3), boundingBox->maxZ - 3,
				3, type);
		break;
	case 1:
		if (branch <= 1)
			StructureMineshaftPieces::getNextComponent(parent, components, random,
				boundingBox->minX - 1, boundingBox->minY - 1 + random.nextInt(3), boundingBox->minZ,
				coordBaseMode, type);
		else if (branch == 2)
			StructureMineshaftPieces::getNextComponent(parent, components, random,
				boundingBox->minX, boundingBox->minY - 1 + random.nextInt(3), boundingBox->minZ - 1,
				2, type);
		else
			StructureMineshaftPieces::getNextComponent(parent, components, random,
				boundingBox->minX, boundingBox->minY - 1 + random.nextInt(3), boundingBox->maxZ + 1,
				0, type);
		break;
	case 2:
		if (branch <= 1)
			StructureMineshaftPieces::getNextComponent(parent, components, random,
				boundingBox->minX, boundingBox->minY - 1 + random.nextInt(3), boundingBox->minZ - 1,
				coordBaseMode, type);
		else if (branch == 2)
			StructureMineshaftPieces::getNextComponent(parent, components, random,
				boundingBox->minX - 1, boundingBox->minY - 1 + random.nextInt(3), boundingBox->minZ,
				1, type);
		else
			StructureMineshaftPieces::getNextComponent(parent, components, random,
				boundingBox->maxX + 1, boundingBox->minY - 1 + random.nextInt(3), boundingBox->minZ,
				3, type);
		break;
	case 3:
		if (branch <= 1)
			StructureMineshaftPieces::getNextComponent(parent, components, random,
				boundingBox->maxX + 1, boundingBox->minY - 1 + random.nextInt(3), boundingBox->minZ,
				coordBaseMode, type);
		else if (branch == 2)
			StructureMineshaftPieces::getNextComponent(parent, components, random,
				boundingBox->maxX - 3, boundingBox->minY - 1 + random.nextInt(3), boundingBox->minZ - 1,
				2, type);
		else
			StructureMineshaftPieces::getNextComponent(parent, components, random,
				boundingBox->maxX - 3, boundingBox->minY - 1 + random.nextInt(3), boundingBox->maxZ + 1,
				0, type);
		break;
	default:
		break;
	}

	if (type >= 8)
		return;

	if (coordBaseMode != 0 && coordBaseMode != 2)
	{
		for (int_t x = boundingBox->minX + 3; x + 3 <= boundingBox->maxX; x += 5)
		{
			const int_t branchRoll = random.nextInt(5);
			if (branchRoll == 0)
				StructureMineshaftPieces::getNextComponent(parent, components, random,
					x, boundingBox->minY, boundingBox->minZ - 1, 2, type + 1);
			else if (branchRoll == 1)
				StructureMineshaftPieces::getNextComponent(parent, components, random,
					x, boundingBox->minY, boundingBox->maxZ + 1, 0, type + 1);
		}
	}
	else
	{
		for (int_t z = boundingBox->minZ + 3; z + 3 <= boundingBox->maxZ; z += 5)
		{
			const int_t branchRoll = random.nextInt(5);
			if (branchRoll == 0)
				StructureMineshaftPieces::getNextComponent(parent, components, random,
					boundingBox->minX - 1, boundingBox->minY, z, 1, type + 1);
			else if (branchRoll == 1)
				StructureMineshaftPieces::getNextComponent(parent, components, random,
					boundingBox->maxX + 1, boundingBox->minY, z, 3, type + 1);
		}
	}
}

bool ComponentMineshaftCorridor::addComponentParts(
	World *world, Random &random, const StructureBoundingBox &chunkBounds)
{
	if (!boundingBox || isLiquidInStructureBoundingBox(world, chunkBounds))
		return false;

	const int_t maxLocalZ = sectionCount * 5 - 1;
	fillWithBlocks(world, chunkBounds, 0, 0, 0, 2, 1, maxLocalZ, 0, 0, false);
	randomlyFillWithBlocks(world, chunkBounds, random, 0.8f,
	                      0, 2, 0, 2, 2, maxLocalZ, 0, 0, false);
	if (hasSpiders)
	{
		randomlyFillWithBlocks(world, chunkBounds, random, 0.6f,
		                      0, 0, 0, 2, 1, maxLocalZ, Block::web->blockID, 0, false);
	}

	for (int_t section = 0; section < sectionCount; ++section)
	{
		const int_t z = 2 + section * 5;
		fillWithBlocks(world, chunkBounds, 0, 0, z, 0, 1, z, Block::fence->blockID, 0, false);
		fillWithBlocks(world, chunkBounds, 2, 0, z, 2, 1, z, Block::fence->blockID, 0, false);
		if (random.nextInt(4) != 0)
			fillWithBlocks(world, chunkBounds, 0, 2, z, 2, 2, z, Block::planks->blockID, 0, false);
		else
		{
			fillWithBlocks(world, chunkBounds, 0, 2, z, 0, 2, z, Block::planks->blockID, 0, false);
			fillWithBlocks(world, chunkBounds, 2, 2, z, 2, 2, z, Block::planks->blockID, 0, false);
		}

		randomlyPlaceBlock(world, chunkBounds, random, 0.1f, 0, 2, z - 1, Block::web->blockID, 0);
		randomlyPlaceBlock(world, chunkBounds, random, 0.1f, 2, 2, z - 1, Block::web->blockID, 0);
		randomlyPlaceBlock(world, chunkBounds, random, 0.1f, 0, 2, z + 1, Block::web->blockID, 0);
		randomlyPlaceBlock(world, chunkBounds, random, 0.1f, 2, 2, z + 1, Block::web->blockID, 0);
		randomlyPlaceBlock(world, chunkBounds, random, 0.05f, 0, 2, z - 2, Block::web->blockID, 0);
		randomlyPlaceBlock(world, chunkBounds, random, 0.05f, 2, 2, z - 2, Block::web->blockID, 0);
		randomlyPlaceBlock(world, chunkBounds, random, 0.05f, 0, 2, z + 2, Block::web->blockID, 0);
		randomlyPlaceBlock(world, chunkBounds, random, 0.05f, 2, 2, z + 2, Block::web->blockID, 0);
		randomlyPlaceBlock(world, chunkBounds, random, 0.05f, 1, 2, z - 1, Block::torchWood->blockID, 0);
		randomlyPlaceBlock(world, chunkBounds, random, 0.05f, 1, 2, z + 1, Block::torchWood->blockID, 0);

		if (random.nextInt(100) == 0)
			createTreasureChestAtCurrentPosition(world, chunkBounds, random, 2, 0, z - 1,
			                                     StructureMineshaftPieces::getTreasurePieces(), 3 + random.nextInt(4));
		if (random.nextInt(100) == 0)
			createTreasureChestAtCurrentPosition(world, chunkBounds, random, 0, 0, z + 1,
			                                     StructureMineshaftPieces::getTreasurePieces(), 3 + random.nextInt(4));

		if (hasSpiders && !spawnerPlaced)
		{
			const int_t worldY = getYWithOffset(0);
			int_t localZ = z - 1 + random.nextInt(3);
			const int_t worldX = getXWithOffset(1, localZ);
			const int_t worldZ = getZWithOffset(1, localZ);
			if (chunkBounds.isVecInside(worldX, worldY, worldZ))
			{
				spawnerPlaced = true;
				world->setBlockWithNotify(worldX, worldY, worldZ, Block::mobSpawner->blockID);
				auto *spawner = dynamic_cast<TileEntityMobSpawner *>(world->getBlockTileEntity(worldX, worldY, worldZ));
				if (spawner != nullptr)
					spawner->setMobID("CaveSpider");
			}
		}
	}

	for (int_t x = 0; x <= 2; ++x)
	{
		for (int_t z = 0; z <= maxLocalZ; ++z)
		{
			if (getBlockIdAtCurrentPosition(world, x, -1, z, chunkBounds) == 0)
				placeBlockAtCurrentPosition(world, Block::planks->blockID, 0, x, -1, z, chunkBounds);
		}
	}

	if (hasRails)
	{
		for (int_t z = 0; z <= maxLocalZ; ++z)
		{
			const int_t below = getBlockIdAtCurrentPosition(world, 1, -1, z, chunkBounds);
			if (below > 0 && below < Block::BLOCK_REGISTRY_SIZE && Block::opaqueCubeLookup[below])
			{
				randomlyPlaceBlock(world, chunkBounds, random, 0.7f,
				                   1, 0, z, Block::rail->blockID,
				                   getMetadataWithOffset(Block::rail->blockID, 0));
			}
		}
	}
	return true;
}
