#include "BlockEnchantmentTable.h"

#include "Block.h"
#include "EntityPlayer.h"
#include "Material.h"
#include "TileEntityEnchantmentTable.h"
#include "World.h"
#include "java/Random.h"

BlockEnchantmentTable::BlockEnchantmentTable(int_t id)
	: BlockContainer(id, 166, Material::rock)
{
	setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 12.0f / 16.0f, 1.0f);
	setLightOpacity(0);
}

bool BlockEnchantmentTable::renderAsNormalBlock()
{
	return false;
}

void BlockEnchantmentTable::randomDisplayTick(World *world, int_t x, int_t y, int_t z, Random &random)
{
	BlockContainer::randomDisplayTick(world, x, y, z, random);
	if (world == nullptr || Block::bookShelf == nullptr)
		return;

	for (int_t shelfX = x - 2; shelfX <= x + 2; ++shelfX)
	{
		for (int_t shelfZ = z - 2; shelfZ <= z + 2; ++shelfZ)
		{
			if (shelfX > x - 2 && shelfX < x + 2 && shelfZ == z - 1)
				shelfZ = z + 2;
			if (random.nextInt(16) != 0)
				continue;

			for (int_t shelfY = y; shelfY <= y + 1; ++shelfY)
			{
				if (world->getBlockId(shelfX, shelfY, shelfZ) != Block::bookShelf->blockID)
					continue;
				if (!world->isAirBlock((shelfX - x) / 2 + x, shelfY, (shelfZ - z) / 2 + z))
					break;
				const double particleMotionX = (double)((float)(shelfX - x) + random.nextFloat()) - 0.5;
				const double particleMotionY = (double)((float)(shelfY - y) - random.nextFloat() - 1.0f);
				const double particleMotionZ = (double)((float)(shelfZ - z) + random.nextFloat()) - 0.5;
				world->spawnParticle("enchantmenttable", (double)x + 0.5, (double)y + 2.0, (double)z + 0.5,
					particleMotionX, particleMotionY, particleMotionZ);
			}
		}
	}
}

bool BlockEnchantmentTable::isOpaqueCube()
{
	return false;
}

int_t BlockEnchantmentTable::getBlockTextureFromSideAndMetadata(int_t side, int_t metadata)
{
	(void)metadata;
	return getBlockTextureFromSide(side);
}

int_t BlockEnchantmentTable::getBlockTextureFromSide(int_t side)
{
	return side == 0 ? blockIndexInTexture + 17 : (side == 1 ? blockIndexInTexture : blockIndexInTexture + 16);
}

TileEntity *BlockEnchantmentTable::getBlockEntity()
{
	return new TileEntityEnchantmentTable();
}

bool BlockEnchantmentTable::blockActivated(World *world, int_t x, int_t y, int_t z, EntityPlayer *player)
{
	if (world == nullptr || player == nullptr)
		return false;
	if (world->multiplayerWorld)
		return true;
	player->displayGUIEnchantment(x, y, z);
	return true;
}
