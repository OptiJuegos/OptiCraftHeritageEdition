#include "BlockChest.h"

#include "AxisAlignedBB.h"
#include "EntityItem.h"
#include "EntityLiving.h"
#include "EntityOcelot.h"
#include "EntityPlayer.h"
#include "IBlockAccess.h"
#include "InventoryLargeChest.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "TileEntityChest.h"
#include "World.h"
#include "java/Arithmetic.h"

BlockChest::BlockChest(int_t i)
	: BlockContainer(i, Material::wood)
{
	blockIndexInTexture = 26;
}

bool BlockChest::isOpaqueCube()
{
	return false;
}

bool BlockChest::renderAsNormalBlock()
{
	return false;
}

int_t BlockChest::getRenderType()
{
	return 22;
}

bool BlockChest::isOpaqueBlockId(int_t id)
{
	return id >= 0 && id < Block::BLOCK_REGISTRY_SIZE && Block::opaqueCubeLookup[id];
}

void BlockChest::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
	BlockContainer::onBlockAdded(world, i, j, k);
	unifyAdjacentChests(world, i, j, k);
	if (world->getBlockId(i, j, k - 1) == blockID)
		unifyAdjacentChests(world, i, j, k - 1);
	if (world->getBlockId(i, j, k + 1) == blockID)
		unifyAdjacentChests(world, i, j, k + 1);
	if (world->getBlockId(i - 1, j, k) == blockID)
		unifyAdjacentChests(world, i - 1, j, k);
	if (world->getBlockId(i + 1, j, k) == blockID)
		unifyAdjacentChests(world, i + 1, j, k);
}

void BlockChest::onBlockPlacedBy(World *world, int_t i, int_t j, int_t k, EntityLiving *entityliving)
{
	int_t north = world->getBlockId(i, j, k - 1);
	int_t south = world->getBlockId(i, j, k + 1);
	int_t west = world->getBlockId(i - 1, j, k);
	int_t east = world->getBlockId(i + 1, j, k);
	int_t metadata = 0;
	int_t direction = MathHelper::floor_double((double)(entityliving->rotationYaw * 4.0f / 360.0f) + 0.5) & 3;
	if (direction == 0)
		metadata = 2;
	if (direction == 1)
		metadata = 5;
	if (direction == 2)
		metadata = 3;
	if (direction == 3)
		metadata = 4;

	if (north != blockID && south != blockID && west != blockID && east != blockID)
	{
		world->setBlockMetadataWithNotify(i, j, k, metadata);
		return;
	}

	if ((north == blockID || south == blockID) && (metadata == 4 || metadata == 5))
	{
		world->setBlockMetadataWithNotify(i, j, north == blockID ? k - 1 : k + 1, metadata);
		world->setBlockMetadataWithNotify(i, j, k, metadata);
	}
	if ((west == blockID || east == blockID) && (metadata == 2 || metadata == 3))
	{
		world->setBlockMetadataWithNotify(west == blockID ? i - 1 : i + 1, j, k, metadata);
		world->setBlockMetadataWithNotify(i, j, k, metadata);
	}
}

void BlockChest::unifyAdjacentChests(World *world, int_t i, int_t j, int_t k)
{
	if (world->multiplayerWorld)
		return;

	int_t north = world->getBlockId(i, j, k - 1);
	int_t south = world->getBlockId(i, j, k + 1);
	int_t west = world->getBlockId(i - 1, j, k);
	int_t east = world->getBlockId(i + 1, j, k);
	int_t metadata;

	if (north != blockID && south != blockID)
	{
		if (west != blockID && east != blockID)
		{
			metadata = 3;
			if (isOpaqueBlockId(south) && !isOpaqueBlockId(north))
				metadata = 2;
			if (isOpaqueBlockId(west) && !isOpaqueBlockId(east))
				metadata = 5;
			if (isOpaqueBlockId(east) && !isOpaqueBlockId(west))
				metadata = 4;
		}
		else
		{
			int_t sideNorth = world->getBlockId(west == blockID ? i - 1 : i + 1, j, k - 1);
			int_t sideSouth = world->getBlockId(west == blockID ? i - 1 : i + 1, j, k + 1);
			metadata = 3;
			int_t adjacentMetadata = world->getBlockMetadata(west == blockID ? i - 1 : i + 1, j, k);
			if (adjacentMetadata == 2)
				metadata = 2;
			if ((isOpaqueBlockId(north) || isOpaqueBlockId(sideNorth)) && !isOpaqueBlockId(south) && !isOpaqueBlockId(sideSouth))
				metadata = 3;
			if ((isOpaqueBlockId(south) || isOpaqueBlockId(sideSouth)) && !isOpaqueBlockId(north) && !isOpaqueBlockId(sideNorth))
				metadata = 2;
		}
	}
	else
	{
		int_t sideWest = world->getBlockId(i - 1, j, north == blockID ? k - 1 : k + 1);
		int_t sideEast = world->getBlockId(i + 1, j, north == blockID ? k - 1 : k + 1);
		metadata = 5;
		int_t adjacentMetadata = world->getBlockMetadata(i, j, north == blockID ? k - 1 : k + 1);
		if (adjacentMetadata == 4)
			metadata = 4;
		if ((isOpaqueBlockId(west) || isOpaqueBlockId(sideWest)) && !isOpaqueBlockId(east) && !isOpaqueBlockId(sideEast))
			metadata = 5;
		if ((isOpaqueBlockId(east) || isOpaqueBlockId(sideEast)) && !isOpaqueBlockId(west) && !isOpaqueBlockId(sideWest))
			metadata = 4;
	}

	world->setBlockMetadataWithNotify(i, j, k, metadata);
}

int_t BlockChest::getBlockTexture(IBlockAccess *access, int_t i, int_t j, int_t k, int_t side)
{
	if (side == 0 || side == 1)
		return blockIndexInTexture - 1;

	int_t north = access->getBlockId(i, j, k - 1);
	int_t south = access->getBlockId(i, j, k + 1);
	int_t west = access->getBlockId(i - 1, j, k);
	int_t east = access->getBlockId(i + 1, j, k);

	if (north != blockID && south != blockID)
	{
		if (west != blockID && east != blockID)
		{
			int_t front = 3;
			if (isOpaqueBlockId(south) && !isOpaqueBlockId(north))
				front = 2;
			if (isOpaqueBlockId(west) && !isOpaqueBlockId(east))
				front = 5;
			if (isOpaqueBlockId(east) && !isOpaqueBlockId(west))
				front = 4;
			return side == front ? blockIndexInTexture + 1 : blockIndexInTexture;
		}
		if (side == 4 || side == 5)
			return blockIndexInTexture;

		int_t offset = west == blockID ? -1 : 0;
		int_t sideNorth = access->getBlockId(west == blockID ? i - 1 : i + 1, j, k - 1);
		int_t sideSouth = access->getBlockId(west == blockID ? i - 1 : i + 1, j, k + 1);
		if (side == 3)
			offset = -1 - offset;
		int_t front = 3;
		if ((isOpaqueBlockId(south) || isOpaqueBlockId(sideSouth)) && !isOpaqueBlockId(north) && !isOpaqueBlockId(sideNorth))
			front = 2;
		return (side == front ? blockIndexInTexture + 16 : blockIndexInTexture + 32) + offset;
	}

	if (side == 2 || side == 3)
		return blockIndexInTexture;
	int_t offset = north == blockID ? -1 : 0;
	int_t sideWest = access->getBlockId(i - 1, j, north == blockID ? k - 1 : k + 1);
	int_t sideEast = access->getBlockId(i + 1, j, north == blockID ? k - 1 : k + 1);
	if (side == 4)
		offset = -1 - offset;
	int_t front = 5;
	if ((isOpaqueBlockId(east) || isOpaqueBlockId(sideEast)) && !isOpaqueBlockId(west) && !isOpaqueBlockId(sideWest))
		front = 4;
	return (side == front ? blockIndexInTexture + 16 : blockIndexInTexture + 32) + offset;
}

int_t BlockChest::getBlockTextureFromSide(int_t side)
{
	if (side == 0 || side == 1)
		return blockIndexInTexture - 1;
	return side == 3 ? blockIndexInTexture + 1 : blockIndexInTexture;
}

bool BlockChest::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
	int_t adjacent = 0;
	if (world->getBlockId(i - 1, j, k) == blockID)
		++adjacent;
	if (world->getBlockId(i + 1, j, k) == blockID)
		++adjacent;
	if (world->getBlockId(i, j, k - 1) == blockID)
		++adjacent;
	if (world->getBlockId(i, j, k + 1) == blockID)
		++adjacent;
	if (adjacent > 1)
		return false;
	if (isThereANeighborChest(world, i - 1, j, k) || isThereANeighborChest(world, i + 1, j, k))
		return false;
	return !isThereANeighborChest(world, i, j, k - 1) && !isThereANeighborChest(world, i, j, k + 1);
}

bool BlockChest::isThereANeighborChest(World *world, int_t i, int_t j, int_t k)
{
	if (world->getBlockId(i, j, k) != blockID)
		return false;
	return world->getBlockId(i - 1, j, k) == blockID || world->getBlockId(i + 1, j, k) == blockID ||
	       world->getBlockId(i, j, k - 1) == blockID || world->getBlockId(i, j, k + 1) == blockID;
}

void BlockChest::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
	BlockContainer::onNeighborBlockChange(world, i, j, k, l);
	TileEntityChest *chest = dynamic_cast<TileEntityChest *>(world->getBlockTileEntity(i, j, k));
	if (chest != nullptr)
		chest->updateContainingBlockInfo();
}

void BlockChest::onBlockRemoval(World *world, int_t i, int_t j, int_t k)
{
	TileEntityChest *chest = dynamic_cast<TileEntityChest *>(world->getBlockTileEntity(i, j, k));
	if (chest != nullptr)
	{
		for (int_t slot = 0; slot < chest->getSizeInventory(); ++slot)
		{
			ItemStack *stack = chest->getStackInSlot(slot);
			if (stack == nullptr)
				continue;

			float offsetX = random.nextFloat() * 0.8f + 0.1f;
			float offsetY = random.nextFloat() * 0.8f + 0.1f;
			float offsetZ = random.nextFloat() * 0.8f + 0.1f;
			while (stack->stackSize > 0)
			{
				int_t count = random.nextInt(21) + 10;
				if (count > stack->stackSize)
					count = stack->stackSize;
				stack->stackSize -= count;
				ItemStack *droppedStack = new ItemStack(stack->itemID, count, stack->getItemDamage());
				if (stack->hasTagCompound())
				{
					NBTTagCompound *copy = dynamic_cast<NBTTagCompound *>(stack->getTagCompound()->copy());
					droppedStack->setTagCompound(copy);
				}
				EntityItem *item = new EntityItem(world, (double)((float)i + offsetX), (double)((float)j + offsetY),
				                                  (double)((float)k + offsetZ), droppedStack);
				const float velocity = 0.05f;
				item->motionX = (float)random.nextGaussian() * velocity;
				item->motionY = (float)random.nextGaussian() * velocity + 0.2f;
				item->motionZ = (float)random.nextGaussian() * velocity;
				if (!world->entityJoinedWorld(item))
					delete item;
			}
		}
	}
	BlockContainer::onBlockRemoval(world, i, j, k);
}

bool BlockChest::isOcelotBlockingChest(World *world, int_t i, int_t j, int_t k)
{
	AxisAlignedBB *box = AxisAlignedBB::getBoundingBoxFromPool(
		static_cast<double>(i), static_cast<double>(JavaArithmetic::intAdd(j, 1)), static_cast<double>(k),
		static_cast<double>(JavaArithmetic::intAdd(i, 1)), static_cast<double>(JavaArithmetic::intAdd(j, 2)),
		static_cast<double>(JavaArithmetic::intAdd(k, 1)));
	const std::vector<Entity *> &entities = world->getEntitiesWithinAABB(typeid(EntityOcelot), box);
	for (Entity *entity : entities)
	{
		EntityOcelot *ocelot = dynamic_cast<EntityOcelot *>(entity);
		if (ocelot != nullptr && ocelot->isSitting())
			return true;
	}
	return false;
}

bool BlockChest::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *player)
{
	IInventory *inventory = dynamic_cast<TileEntityChest *>(world->getBlockTileEntity(i, j, k));
	if (inventory == nullptr)
		return true;
	if (world->isBlockNormalCube(i, j + 1, k) || isOcelotBlockingChest(world, i, j, k))
		return true;

	auto blockedAdjacent = [this, world, j](int_t x, int_t z) -> bool
	{
		return world->getBlockId(x, j, z) == blockID &&
		       (world->isBlockNormalCube(x, j + 1, z) || isOcelotBlockingChest(world, x, j, z));
	};
	if (blockedAdjacent(i - 1, k) || blockedAdjacent(i + 1, k) || blockedAdjacent(i, k - 1) || blockedAdjacent(i, k + 1))
		return true;

	if (world->multiplayerWorld)
		return true;

	if (world->getBlockId(i - 1, j, k) == blockID)
		inventory = new InventoryLargeChest("Large chest", dynamic_cast<TileEntityChest *>(world->getBlockTileEntity(i - 1, j, k)), inventory);
	if (world->getBlockId(i + 1, j, k) == blockID)
		inventory = new InventoryLargeChest("Large chest", inventory, dynamic_cast<TileEntityChest *>(world->getBlockTileEntity(i + 1, j, k)));
	if (world->getBlockId(i, j, k - 1) == blockID)
		inventory = new InventoryLargeChest("Large chest", dynamic_cast<TileEntityChest *>(world->getBlockTileEntity(i, j, k - 1)), inventory);
	if (world->getBlockId(i, j, k + 1) == blockID)
		inventory = new InventoryLargeChest("Large chest", inventory, dynamic_cast<TileEntityChest *>(world->getBlockTileEntity(i, j, k + 1)));

	player->displayGUIChest(inventory);
	return true;
}

TileEntity *BlockChest::getBlockEntity()
{
	return new TileEntityChest();
}
