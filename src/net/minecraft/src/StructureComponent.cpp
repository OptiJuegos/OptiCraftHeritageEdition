#include "StructureComponent.h"

#include <algorithm>
#include <cmath>

#include "Block.h"
#include "ChunkPosition.h"
#include "Item.h"
#include "ItemDoor.h"
#include "ItemStack.h"
#include "Material.h"
#include "StructureBoundingBox.h"
#include "StructurePieceBlockSelector.h"
#include "StructurePieceTreasure.h"
#include "TileEntity.h"
#include "TileEntityChest.h"
#include "WeightedRandom.h"
#include "World.h"
#include "java/Random.h"
#include "java/Arithmetic.h"

StructureComponent::StructureComponent(int_t type)
	: coordBaseMode(-1), componentType(type)
{
}

StructureComponent::~StructureComponent() = default;

void StructureComponent::buildComponent(StructureComponent *,
                                        std::vector<StructureComponent *> &,
                                        Random &)
{
}

StructureBoundingBox *StructureComponent::getBoundingBox() const
{
	return boundingBox.get();
}

int_t StructureComponent::getComponentType() const
{
	return componentType;
}

StructureComponent *StructureComponent::findIntersecting(
	const std::vector<StructureComponent *> &components,
	const StructureBoundingBox &bounds)
{
	for (StructureComponent *component : components)
	{
		if (component != nullptr && component->getBoundingBox() != nullptr &&
		    component->getBoundingBox()->intersectsWith(bounds))
			return component;
	}
	return nullptr;
}

ChunkPosition *StructureComponent::getCenter() const
{
	if (!boundingBox)
		return nullptr;
	return new ChunkPosition(boundingBox->getCenterX(), boundingBox->getCenterY(), boundingBox->getCenterZ());
}

bool StructureComponent::isLiquidInStructureBoundingBox(
	World *world, const StructureBoundingBox &chunkBounds) const
{
	if (world == nullptr || !boundingBox)
		return false;

	const int_t minX = std::max(JavaArithmetic::intSub(boundingBox->minX, 1), chunkBounds.minX);
	const int_t minY = std::max(JavaArithmetic::intSub(boundingBox->minY, 1), chunkBounds.minY);
	const int_t minZ = std::max(JavaArithmetic::intSub(boundingBox->minZ, 1), chunkBounds.minZ);
	const int_t maxX = std::min(JavaArithmetic::intAdd(boundingBox->maxX, 1), chunkBounds.maxX);
	const int_t maxY = std::min(JavaArithmetic::intAdd(boundingBox->maxY, 1), chunkBounds.maxY);
	const int_t maxZ = std::min(JavaArithmetic::intAdd(boundingBox->maxZ, 1), chunkBounds.maxZ);

	auto isLiquid = [world](int_t x, int_t y, int_t z)
	{
		const int_t blockId = world->getBlockId(x, y, z);
		return blockId > 0 && blockId < Block::BLOCK_REGISTRY_SIZE &&
		       Block::blocksList[blockId] != nullptr &&
		       Block::blocksList[blockId]->blockMaterial->getIsLiquid();
	};

	for (int_t x = minX; x <= maxX; ++x)
	{
		for (int_t z = minZ; z <= maxZ; ++z)
		{
			if (isLiquid(x, minY, z) || isLiquid(x, maxY, z))
				return true;
		}
	}

	for (int_t x = minX; x <= maxX; ++x)
	{
		for (int_t y = minY; y <= maxY; ++y)
		{
			if (isLiquid(x, y, minZ) || isLiquid(x, y, maxZ))
				return true;
		}
	}

	for (int_t z = minZ; z <= maxZ; ++z)
	{
		for (int_t y = minY; y <= maxY; ++y)
		{
			if (isLiquid(minX, y, z) || isLiquid(maxX, y, z))
				return true;
		}
	}

	return false;
}

int_t StructureComponent::getXWithOffset(int_t x, int_t z) const
{
	if (!boundingBox)
		return x;
	switch (coordBaseMode)
	{
	case 0:
	case 2:
		return JavaArithmetic::intAdd(boundingBox->minX, x);
	case 1:
		return JavaArithmetic::intSub(boundingBox->maxX, z);
	case 3:
		return JavaArithmetic::intAdd(boundingBox->minX, z);
	default:
		return x;
	}
}

int_t StructureComponent::getYWithOffset(int_t y) const
{
	return coordBaseMode == -1 || !boundingBox ? y : JavaArithmetic::intAdd(y, boundingBox->minY);
}

int_t StructureComponent::getZWithOffset(int_t x, int_t z) const
{
	if (!boundingBox)
		return z;
	switch (coordBaseMode)
	{
	case 0:
		return JavaArithmetic::intAdd(boundingBox->minZ, z);
	case 1:
	case 3:
		return JavaArithmetic::intAdd(boundingBox->minZ, x);
	case 2:
		return JavaArithmetic::intSub(boundingBox->maxZ, z);
	default:
		return z;
	}
}

int_t StructureComponent::getMetadataWithOffset(int_t blockId, int_t metadata) const
{
	if (Block::rail != nullptr && blockId == Block::rail->blockID)
	{
		if (coordBaseMode == 1 || coordBaseMode == 3)
			return metadata == 1 ? 0 : 1;
	}
	else if ((Block::doorWood != nullptr && blockId == Block::doorWood->blockID) ||
	         (Block::doorSteel != nullptr && blockId == Block::doorSteel->blockID))
	{
		if (coordBaseMode == 0)
		{
			if (metadata == 0) return 2;
			if (metadata == 2) return 0;
		}
		else if (coordBaseMode == 1)
		{
			return (metadata + 1) & 3;
		}
		else if (coordBaseMode == 3)
		{
			return (metadata + 3) & 3;
		}
	}
	else
	{
		const bool isStairs =
			(Block::stairCompactCobblestone != nullptr && blockId == Block::stairCompactCobblestone->blockID) ||
			(Block::stairCompactPlanks != nullptr && blockId == Block::stairCompactPlanks->blockID) ||
			blockId == 109 || blockId == 114;
		if (isStairs)
		{
			if (coordBaseMode == 0)
			{
				if (metadata == 2) return 3;
				if (metadata == 3) return 2;
			}
			else if (coordBaseMode == 1)
			{
				if (metadata == 0) return 2;
				if (metadata == 1) return 3;
				if (metadata == 2) return 0;
				if (metadata == 3) return 1;
			}
			else if (coordBaseMode == 3)
			{
				if (metadata == 0) return 2;
				if (metadata == 1) return 3;
				if (metadata == 2) return 1;
				if (metadata == 3) return 0;
			}
		}
		else if (Block::ladder != nullptr && blockId == Block::ladder->blockID)
		{
			if (coordBaseMode == 0)
			{
				if (metadata == 2) return 3;
				if (metadata == 3) return 2;
			}
			else if (coordBaseMode == 1)
			{
				if (metadata == 2) return 4;
				if (metadata == 3) return 5;
				if (metadata == 4) return 2;
				if (metadata == 5) return 3;
			}
			else if (coordBaseMode == 3)
			{
				if (metadata == 2) return 5;
				if (metadata == 3) return 4;
				if (metadata == 4) return 2;
				if (metadata == 5) return 3;
			}
		}
		else if (Block::button != nullptr && blockId == Block::button->blockID)
		{
			if (coordBaseMode == 0)
			{
				if (metadata == 3) return 4;
				if (metadata == 4) return 3;
			}
			else if (coordBaseMode == 1)
			{
				if (metadata == 3) return 1;
				if (metadata == 4) return 2;
				if (metadata == 2) return 3;
				if (metadata == 1) return 4;
			}
			else if (coordBaseMode == 3)
			{
				if (metadata == 3) return 2;
				if (metadata == 4) return 1;
				if (metadata == 2) return 3;
				if (metadata == 1) return 4;
			}
		}
	}

	return metadata;
}

void StructureComponent::placeBlockAtCurrentPosition(
	World *world, int_t blockId, int_t metadata,
	int_t x, int_t y, int_t z, const StructureBoundingBox &chunkBounds) const
{
	const int_t worldX = getXWithOffset(x, z);
	const int_t worldY = getYWithOffset(y);
	const int_t worldZ = getZWithOffset(x, z);
	if (chunkBounds.isVecInside(worldX, worldY, worldZ))
		world->setBlockAndMetadata(worldX, worldY, worldZ, blockId, metadata);
}

int_t StructureComponent::getBlockIdAtCurrentPosition(
	World *world, int_t x, int_t y, int_t z,
	const StructureBoundingBox &chunkBounds) const
{
	const int_t worldX = getXWithOffset(x, z);
	const int_t worldY = getYWithOffset(y);
	const int_t worldZ = getZWithOffset(x, z);
	return chunkBounds.isVecInside(worldX, worldY, worldZ)
		? world->getBlockId(worldX, worldY, worldZ)
		: 0;
}

void StructureComponent::fillWithBlocks(
	World *world, const StructureBoundingBox &chunkBounds,
	int_t minX, int_t minY, int_t minZ,
	int_t maxX, int_t maxY, int_t maxZ,
	int_t boundaryBlockId, int_t insideBlockId, bool onlyReplaceNonAir) const
{
	for (int_t y = minY; y <= maxY; ++y)
	{
		for (int_t x = minX; x <= maxX; ++x)
		{
			for (int_t z = minZ; z <= maxZ; ++z)
			{
				if (onlyReplaceNonAir && getBlockIdAtCurrentPosition(world, x, y, z, chunkBounds) == 0)
					continue;
				const bool boundary = y == minY || y == maxY || x == minX || x == maxX || z == minZ || z == maxZ;
				placeBlockAtCurrentPosition(world, boundary ? boundaryBlockId : insideBlockId, 0,
				                            x, y, z, chunkBounds);
			}
		}
	}
}

void StructureComponent::fillWithRandomizedBlocks(
	World *world, const StructureBoundingBox &chunkBounds,
	int_t minX, int_t minY, int_t minZ,
	int_t maxX, int_t maxY, int_t maxZ,
	bool onlyReplaceNonAir, Random &random,
	StructurePieceBlockSelector &selector) const
{
	for (int_t y = minY; y <= maxY; ++y)
	{
		for (int_t x = minX; x <= maxX; ++x)
		{
			for (int_t z = minZ; z <= maxZ; ++z)
			{
				if (onlyReplaceNonAir && getBlockIdAtCurrentPosition(world, x, y, z, chunkBounds) == 0)
					continue;
				const bool boundary = y == minY || y == maxY || x == minX || x == maxX || z == minZ || z == maxZ;
				selector.selectBlocks(random, x, y, z, boundary);
				placeBlockAtCurrentPosition(world, selector.getSelectedBlockId(), selector.getSelectedBlockMetaData(),
				                            x, y, z, chunkBounds);
			}
		}
	}
}

void StructureComponent::randomlyFillWithBlocks(
	World *world, const StructureBoundingBox &chunkBounds,
	Random &random, float chance,
	int_t minX, int_t minY, int_t minZ,
	int_t maxX, int_t maxY, int_t maxZ,
	int_t boundaryBlockId, int_t insideBlockId, bool onlyReplaceNonAir) const
{
	for (int_t y = minY; y <= maxY; ++y)
	{
		for (int_t x = minX; x <= maxX; ++x)
		{
			for (int_t z = minZ; z <= maxZ; ++z)
			{
				if (random.nextFloat() > chance ||
				    (onlyReplaceNonAir && getBlockIdAtCurrentPosition(world, x, y, z, chunkBounds) == 0))
					continue;
				const bool boundary = y == minY || y == maxY || x == minX || x == maxX || z == minZ || z == maxZ;
				placeBlockAtCurrentPosition(world, boundary ? boundaryBlockId : insideBlockId, 0,
				                            x, y, z, chunkBounds);
			}
		}
	}
}

void StructureComponent::randomlyPlaceBlock(
	World *world, const StructureBoundingBox &chunkBounds,
	Random &random, float chance,
	int_t x, int_t y, int_t z, int_t blockId, int_t metadata) const
{
	if (random.nextFloat() < chance)
		placeBlockAtCurrentPosition(world, blockId, metadata, x, y, z, chunkBounds);
}

void StructureComponent::randomlyRareFillWithBlocks(
	World *world, const StructureBoundingBox &chunkBounds,
	int_t minX, int_t minY, int_t minZ,
	int_t maxX, int_t maxY, int_t maxZ,
	int_t blockId, bool onlyReplaceNonAir) const
{
	const float sizeX = static_cast<float>(maxX - minX + 1);
	const float sizeY = static_cast<float>(maxY - minY + 1);
	const float sizeZ = static_cast<float>(maxZ - minZ + 1);
	const float centerX = static_cast<float>(minX) + sizeX / 2.0f;
	const float centerZ = static_cast<float>(minZ) + sizeZ / 2.0f;

	for (int_t y = minY; y <= maxY; ++y)
	{
		const float normalizedY = static_cast<float>(y - minY) / sizeY;
		for (int_t x = minX; x <= maxX; ++x)
		{
			const float normalizedX = (static_cast<float>(x) - centerX) / (sizeX * 0.5f);
			for (int_t z = minZ; z <= maxZ; ++z)
			{
				const float normalizedZ = (static_cast<float>(z) - centerZ) / (sizeZ * 0.5f);
				if (onlyReplaceNonAir && getBlockIdAtCurrentPosition(world, x, y, z, chunkBounds) == 0)
					continue;
				const float distance = normalizedX * normalizedX + normalizedY * normalizedY + normalizedZ * normalizedZ;
				if (distance <= 1.05f)
					placeBlockAtCurrentPosition(world, blockId, 0, x, y, z, chunkBounds);
			}
		}
	}
}

void StructureComponent::clearCurrentPositionBlocksUpwards(
	World *world, int_t x, int_t y, int_t z,
	const StructureBoundingBox &chunkBounds) const
{
	const int_t worldX = getXWithOffset(x, z);
	int_t worldY = getYWithOffset(y);
	const int_t worldZ = getZWithOffset(x, z);
	if (!chunkBounds.isVecInside(worldX, worldY, worldZ))
		return;

	while (!world->isAirBlock(worldX, worldY, worldZ) && worldY < 255)
	{
		world->setBlockAndMetadata(worldX, worldY, worldZ, 0, 0);
		++worldY;
	}
}

void StructureComponent::fillCurrentPositionBlocksDownwards(
	World *world, int_t blockId, int_t metadata,
	int_t x, int_t y, int_t z, const StructureBoundingBox &chunkBounds) const
{
	const int_t worldX = getXWithOffset(x, z);
	int_t worldY = getYWithOffset(y);
	const int_t worldZ = getZWithOffset(x, z);
	if (!chunkBounds.isVecInside(worldX, worldY, worldZ))
		return;

	while ((world->isAirBlock(worldX, worldY, worldZ) ||
	        world->getBlockMaterial(worldX, worldY, worldZ)->getIsLiquid()) && worldY > 1)
	{
		world->setBlockAndMetadata(worldX, worldY, worldZ, blockId, metadata);
		--worldY;
	}
}

void StructureComponent::createTreasureChestAtCurrentPosition(
	World *world, const StructureBoundingBox &chunkBounds, Random &random,
	int_t x, int_t y, int_t z,
	const std::vector<StructurePieceTreasure> &loot, int_t rolls) const
{
	const int_t worldX = getXWithOffset(x, z);
	const int_t worldY = getYWithOffset(y);
	const int_t worldZ = getZWithOffset(x, z);
	if (!chunkBounds.isVecInside(worldX, worldY, worldZ) ||
	    world->getBlockId(worldX, worldY, worldZ) == Block::chest->blockID)
		return;

	world->setBlockWithNotify(worldX, worldY, worldZ, Block::chest->blockID);
	TileEntityChest *chest = dynamic_cast<TileEntityChest *>(world->getBlockTileEntity(worldX, worldY, worldZ));
	if (chest != nullptr)
		fillTreasureChestWithLoot(random, loot, chest, rolls);
}

void StructureComponent::fillTreasureChestWithLoot(
	Random &random, const std::vector<StructurePieceTreasure> &loot,
	TileEntityChest *chest, int_t rolls)
{
	std::vector<WeightedRandomChoice *> choices;
	choices.reserve(loot.size());
	for (const StructurePieceTreasure &entry : loot)
		choices.push_back(const_cast<StructurePieceTreasure *>(&entry));

	for (int_t roll = 0; roll < rolls; ++roll)
	{
		auto *entry = dynamic_cast<StructurePieceTreasure *>(WeightedRandom::getRandomItem(random, choices));
		if (entry == nullptr)
			continue;
		const int_t count = entry->minItemStack + random.nextInt(entry->maxItemStack - entry->minItemStack + 1);
		if (entry->itemID >= 0 && entry->itemID < Item::ITEM_LIST_SIZE && Item::itemsList[entry->itemID] != nullptr &&
		    Item::itemsList[entry->itemID]->getItemStackLimit() >= count)
		{
			chest->setInventorySlotContents(random.nextInt(chest->getSizeInventory()),
			                                new ItemStack(entry->itemID, count, entry->itemMetadata));
		}
		else
		{
			for (int_t item = 0; item < count; ++item)
			{
				chest->setInventorySlotContents(random.nextInt(chest->getSizeInventory()),
				                                new ItemStack(entry->itemID, 1, entry->itemMetadata));
			}
		}
	}
}

void StructureComponent::placeDoorAtCurrentPosition(
	World *world, const StructureBoundingBox &chunkBounds, Random &,
	int_t x, int_t y, int_t z, int_t direction) const
{
	const int_t worldX = getXWithOffset(x, z);
	const int_t worldY = getYWithOffset(y);
	const int_t worldZ = getZWithOffset(x, z);
	if (chunkBounds.isVecInside(worldX, worldY, worldZ))
		ItemDoor::placeDoorBlock(world, worldX, worldY, worldZ, direction, Block::doorWood);
}
