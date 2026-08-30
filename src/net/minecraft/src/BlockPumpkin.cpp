#include "BlockPumpkin.h"
#include "Material.h"
#include "World.h"
#include "Block.h"
#include "EntityLiving.h"
#include "MathHelper.h"
#include "EntitySnowman.h"
#include "EntityIronGolem.h"

BlockPumpkin::BlockPumpkin(int_t i, int_t j, bool flag) : Block(i, Material::pumpkin)
{
	blockIndexInTexture = j;
	setTickOnLoad(true);
	blockType = flag;
}

int_t BlockPumpkin::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
	if (i == 1) return blockIndexInTexture;
	if (i == 0) return blockIndexInTexture;
	int_t k = blockIndexInTexture + 1 + 16;
	if (blockType) k++;
	if (j == 2 && i == 2) return k;
	if (j == 3 && i == 5) return k;
	if (j == 0 && i == 3) return k;
	if (j == 1 && i == 4) return k;
	return blockIndexInTexture + 16;
}

int_t BlockPumpkin::getBlockTextureFromSide(int_t i)
{
	if (i == 1) return blockIndexInTexture;
	if (i == 0) return blockIndexInTexture;
	if (i == 3) return blockIndexInTexture + 1 + 16;
	return blockIndexInTexture + 16;
}

void BlockPumpkin::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
	Block::onBlockAdded(world, i, j, k);

	if (world->getBlockId(i, j - 1, k) == Block::blockSnow->blockID &&
	    world->getBlockId(i, j - 2, k) == Block::blockSnow->blockID)
	{
		if (!world->multiplayerWorld)
		{
			world->setBlock(i, j, k, 0);
			world->setBlock(i, j - 1, k, 0);
			world->setBlock(i, j - 2, k, 0);

			EntitySnowman *snowman = new EntitySnowman(world);
			snowman->setLocationAndAngles((double)i + 0.5, (double)j - 1.95, (double)k + 0.5, 0.0f, 0.0f);
			if (!world->spawnEntityInWorld(snowman))
				delete snowman;

			world->notifyBlockChange(i, j, k, 0);
			world->notifyBlockChange(i, j - 1, k, 0);
			world->notifyBlockChange(i, j - 2, k, 0);
		}

		for (int_t n = 0; n < 120; ++n)
		{
			const double particleX = (double)i + world->rand.nextDouble();
			const double particleY = (double)(j - 2) + world->rand.nextDouble() * 2.5;
			const double particleZ = (double)k + world->rand.nextDouble();
			world->spawnParticle("snowshovel", particleX, particleY, particleZ, 0.0, 0.0, 0.0);
		}
	}
	else if (world->getBlockId(i, j - 1, k) == Block::blockSteel->blockID &&
	         world->getBlockId(i, j - 2, k) == Block::blockSteel->blockID)
	{
		const bool xArms = world->getBlockId(i - 1, j - 1, k) == Block::blockSteel->blockID &&
		                   world->getBlockId(i + 1, j - 1, k) == Block::blockSteel->blockID;
		const bool zArms = world->getBlockId(i, j - 1, k - 1) == Block::blockSteel->blockID &&
		                   world->getBlockId(i, j - 1, k + 1) == Block::blockSteel->blockID;

		if (xArms || zArms)
		{
			world->setBlock(i, j, k, 0);
			world->setBlock(i, j - 1, k, 0);
			world->setBlock(i, j - 2, k, 0);
			if (xArms)
			{
				world->setBlock(i - 1, j - 1, k, 0);
				world->setBlock(i + 1, j - 1, k, 0);
			}
			else
			{
				world->setBlock(i, j - 1, k - 1, 0);
				world->setBlock(i, j - 1, k + 1, 0);
			}

			EntityIronGolem *golem = new EntityIronGolem(world);
			golem->setPlayerCreated(true);
			golem->setLocationAndAngles((double)i + 0.5, (double)j - 1.95, (double)k + 0.5, 0.0f, 0.0f);
			if (!world->spawnEntityInWorld(golem))
				delete golem;

			for (int_t n = 0; n < 120; ++n)
			{
				const double particleX = (double)i + world->rand.nextDouble();
				const double particleY = (double)(j - 2) + world->rand.nextDouble() * 3.9;
				const double particleZ = (double)k + world->rand.nextDouble();
				world->spawnParticle("snowballpoof", particleX, particleY, particleZ, 0.0, 0.0, 0.0);
			}

			world->notifyBlockChange(i, j, k, 0);
			world->notifyBlockChange(i, j - 1, k, 0);
			world->notifyBlockChange(i, j - 2, k, 0);
			if (xArms)
			{
				world->notifyBlockChange(i - 1, j - 1, k, 0);
				world->notifyBlockChange(i + 1, j - 1, k, 0);
			}
			else
			{
				world->notifyBlockChange(i, j - 1, k - 1, 0);
				world->notifyBlockChange(i, j - 1, k + 1, 0);
			}
		}
	}
}

bool BlockPumpkin::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
	int_t l = world->getBlockId(i, j, k);
	return (l == 0 || Block::blocksList[l]->blockMaterial->getIsGroundCover()) &&
	       world->isBlockNormalCube(i, j - 1, k);
}

void BlockPumpkin::onBlockPlacedBy(World *world, int_t i, int_t j, int_t k, EntityLiving *entityliving)
{
	int_t l = MathHelper::floor_double((double)(entityliving->rotationYaw * 4.0f / 360.0f) + 2.5) & 3;
	world->setBlockMetadataWithNotify(i, j, k, l);
}
