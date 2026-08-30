#include "BlockFire.h"

#include "BlockPortal.h"
#include "IBlockAccess.h"
#include "Material.h"
#include "World.h"
#include "WorldProviderEnd.h"
#include "java/Random.h"
#include "BlockLeaves.h"
#include "BlockTallGrass.h"
#include "BlockVine.h"

BlockFire::BlockFire(int_t i, int_t j) :
	Block(i, j, Material::fire)
{
	for (int_t l = 0; l < 256; l++)
	{
		chanceToEncourageFire[l] = 0;
		abilityToCatchFire[l] = 0;
	}
	setTickOnLoad(true);
}

void BlockFire::initializeBlock()
{
	setBurnRate(Block::planks->blockID, 5, 20);
	setBurnRate(Block::fence->blockID, 5, 20);
	setBurnRate(Block::stairCompactPlanks->blockID, 5, 20);
	setBurnRate(Block::wood->blockID, 5, 5);
	setBurnRate(Block::leaves->blockID, 30, 60);
	setBurnRate(Block::bookShelf->blockID, 30, 20);
	setBurnRate(Block::tnt->blockID, 15, 100);
	setBurnRate(Block::tallGrass->blockID, 60, 100);
	setBurnRate(Block::cloth->blockID, 30, 60);
	setBurnRate(Block::vine->blockID, 15, 100);
}

void BlockFire::setBurnRate(int_t i, int_t j, int_t k)
{
	chanceToEncourageFire[i] = j;
	abilityToCatchFire[i] = k;
}

AxisAlignedBB *BlockFire::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	return nullptr;
}

bool BlockFire::isOpaqueCube()
{
	return false;
}

bool BlockFire::renderAsNormalBlock()
{
	return false;
}

int_t BlockFire::getRenderType()
{
	return 3;
}

int_t BlockFire::quantityDropped(Random &random)
{
	return 0;
}

int_t BlockFire::tickRate()
{
	return 30;
}

void BlockFire::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	bool eternalFire = world->getBlockId(i, j - 1, k) == Block::netherrack->blockID;
	if (dynamic_cast<WorldProviderEnd *>(world->worldProvider) != nullptr &&
	    world->getBlockId(i, j - 1, k) == Block::bedrock->blockID)
	{
		eternalFire = true;
	}

	if (!canPlaceBlockAt(world, i, j, k))
	{
		world->setBlockWithNotify(i, j, k, 0);
	}

	if (eternalFire || !world->isRaining() ||
	    (!world->canLightningStrikeAt(i, j, k) &&
	     !world->canLightningStrikeAt(i - 1, j, k) &&
	     !world->canLightningStrikeAt(i + 1, j, k) &&
	     !world->canLightningStrikeAt(i, j, k - 1) &&
	     !world->canLightningStrikeAt(i, j, k + 1)))
	{
		int_t metadata = world->getBlockMetadata(i, j, k);
		if (metadata < 15)
		{
			world->setBlockMetadata(i, j, k, metadata + random.nextInt(3) / 2);
		}

		world->scheduleBlockUpdate(i, j, k, blockID, tickRate() + random.nextInt(10));
		if (!eternalFire && !hasFlammableNeighbor(world, i, j, k))
		{
			if (!world->isBlockNormalCube(i, j - 1, k) || metadata > 3)
			{
				world->setBlockWithNotify(i, j, k, 0);
			}
			return;
		}

		if (!eternalFire && !canBlockCatchFire(world, i, j - 1, k) && metadata == 15 && random.nextInt(4) == 0)
		{
			world->setBlockWithNotify(i, j, k, 0);
			return;
		}

		const bool highHumidity = world->isBlockHighHumidity(i, j, k);
		const int_t humidityModifier = highHumidity ? -50 : 0;

		tryToCatchBlockOnFire(world, i + 1, j, k, 300 + humidityModifier, random, metadata);
		tryToCatchBlockOnFire(world, i - 1, j, k, 300 + humidityModifier, random, metadata);
		tryToCatchBlockOnFire(world, i, j - 1, k, 250 + humidityModifier, random, metadata);
		tryToCatchBlockOnFire(world, i, j + 1, k, 250 + humidityModifier, random, metadata);
		tryToCatchBlockOnFire(world, i, j, k - 1, 300 + humidityModifier, random, metadata);
		tryToCatchBlockOnFire(world, i, j, k + 1, 300 + humidityModifier, random, metadata);

		for (int_t x = i - 1; x <= i + 1; ++x)
		{
			for (int_t z = k - 1; z <= k + 1; ++z)
			{
				for (int_t y = j - 1; y <= j + 4; ++y)
				{
					if (x == i && y == j && z == k)
					{
						continue;
					}

					int_t chanceScale = 100;
					if (y > j + 1)
					{
						chanceScale += (y - (j + 1)) * 100;
					}

					const int_t encouragement = getChanceOfNeighborsEncouragingFire(world, x, y, z);
					if (encouragement <= 0)
					{
						continue;
					}

					int_t chance = (encouragement + 40) / (metadata + 30);
					if (highHumidity)
					{
						chance /= 2;
					}

					if (chance > 0 && random.nextInt(chanceScale) <= chance &&
					    (!world->isRaining() || !world->canLightningStrikeAt(x, y, z)) &&
					    !world->canLightningStrikeAt(x - 1, y, k) &&
					    !world->canLightningStrikeAt(x + 1, y, z) &&
					    !world->canLightningStrikeAt(x, y, z - 1) &&
					    !world->canLightningStrikeAt(x, y, z + 1))
					{
						int_t newMetadata = metadata + random.nextInt(5) / 4;
						if (newMetadata > 15)
						{
							newMetadata = 15;
						}
						world->setBlockAndMetadataWithNotify(x, y, z, blockID, newMetadata);
					}
				}
			}
		}
	}
	else
	{
		world->setBlockWithNotify(i, j, k, 0);
	}
}

void BlockFire::tryToCatchBlockOnFire(World *world, int_t i, int_t j, int_t k, int_t l, Random &random, int_t i1)
{
	int_t j1 = abilityToCatchFire[world->getBlockId(i, j, k)];
	if (random.nextInt(l) < j1)
	{
		bool flag = world->getBlockId(i, j, k) == Block::tnt->blockID;
		if (random.nextInt(i1 + 10) < 5 && !world->canLightningStrikeAt(i, j, k))
		{
			int_t k1 = i1 + random.nextInt(5) / 4;
			if (k1 > 15)
			{
				k1 = 15;
			}
			world->setBlockAndMetadataWithNotify(i, j, k, blockID, k1);
		}
		else
		{
			world->setBlockWithNotify(i, j, k, 0);
		}
		if (flag)
		{
			Block::tnt->onBlockDestroyedByPlayer(world, i, j, k, 1);
		}
	}
}

bool BlockFire::hasFlammableNeighbor(World *world, int_t i, int_t j, int_t k)
{
	if (canBlockCatchFire(world, i + 1, j, k)) return true;
	if (canBlockCatchFire(world, i - 1, j, k)) return true;
	if (canBlockCatchFire(world, i, j - 1, k)) return true;
	if (canBlockCatchFire(world, i, j + 1, k)) return true;
	if (canBlockCatchFire(world, i, j, k - 1)) return true;
	return canBlockCatchFire(world, i, j, k + 1);
}

int_t BlockFire::getChanceOfNeighborsEncouragingFire(World *world, int_t i, int_t j, int_t k)
{
	int_t l = 0;
	if (!world->isAirBlock(i, j, k))
	{
		return 0;
	}
	else
	{
		l = getChanceToEncourageFire(world, i + 1, j, k, l);
		l = getChanceToEncourageFire(world, i - 1, j, k, l);
		l = getChanceToEncourageFire(world, i, j - 1, k, l);
		l = getChanceToEncourageFire(world, i, j + 1, k, l);
		l = getChanceToEncourageFire(world, i, j, k - 1, l);
		l = getChanceToEncourageFire(world, i, j, k + 1, l);
		return l;
	}
}

bool BlockFire::isCollidable()
{
	return false;
}

bool BlockFire::canBlockCatchFire(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	return chanceToEncourageFire[iblockaccess->getBlockId(i, j, k)] > 0;
}

int_t BlockFire::getChanceToEncourageFire(World *world, int_t i, int_t j, int_t k, int_t l)
{
	int_t i1 = chanceToEncourageFire[world->getBlockId(i, j, k)];
	if (i1 > l)
	{
		return i1;
	}
	else
	{
		return l;
	}
}

bool BlockFire::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
	return world->isBlockNormalCube(i, j - 1, k) || hasFlammableNeighbor(world, i, j, k);
}

void BlockFire::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
	if (!world->isBlockNormalCube(i, j - 1, k) && !hasFlammableNeighbor(world, i, j, k))
	{
		world->setBlockWithNotify(i, j, k, 0);
		return;
	}
	else
	{
		return;
	}
}

void BlockFire::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
	if (world->worldProvider->worldType <= 0 &&
	    world->getBlockId(i, j - 1, k) == Block::obsidian->blockID &&
	    Block::portal->tryToCreatePortal(world, i, j, k))
	{
		return;
	}

	if (!world->isBlockNormalCube(i, j - 1, k) && !hasFlammableNeighbor(world, i, j, k))
	{
		world->setBlockWithNotify(i, j, k, 0);
		return;
	}

	world->scheduleBlockUpdate(i, j, k, blockID, tickRate() + world->rand.nextInt(10));
}

void BlockFire::randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	if (random.nextInt(24) == 0)
	{
		const float volume = 1.0f + random.nextFloat();
		const float pitch = random.nextFloat() * 0.7f + 0.3f;
		world->playSoundEffect((float)i + 0.5f, (float)j + 0.5f, (float)k + 0.5f, "fire.fire", volume, pitch);
	}
	if (world->isBlockNormalCube(i, j - 1, k) || Block::fire->canBlockCatchFire(world, i, j - 1, k))
	{
		for (int_t l = 0; l < 3; l++)
		{
			float f = (float)i + random.nextFloat();
			float f6 = (float)j + random.nextFloat() * 0.5f + 0.5f;
			float f12 = (float)k + random.nextFloat();
			world->spawnParticle("largesmoke", f, f6, f12, 0.0, 0.0, 0.0);
		}
	}
	else
	{
		if (Block::fire->canBlockCatchFire(world, i - 1, j, k))
		{
			for (int_t i1 = 0; i1 < 2; i1++)
			{
				float f1 = (float)i + random.nextFloat() * 0.1f;
				float f7 = (float)j + random.nextFloat();
				float f13 = (float)k + random.nextFloat();
				world->spawnParticle("largesmoke", f1, f7, f13, 0.0, 0.0, 0.0);
			}
		}
		if (Block::fire->canBlockCatchFire(world, i + 1, j, k))
		{
			for (int_t j1 = 0; j1 < 2; j1++)
			{
				float f2 = (float)(i + 1) - random.nextFloat() * 0.1f;
				float f8 = (float)j + random.nextFloat();
				float f14 = (float)k + random.nextFloat();
				world->spawnParticle("largesmoke", f2, f8, f14, 0.0, 0.0, 0.0);
			}
		}
		if (Block::fire->canBlockCatchFire(world, i, j, k - 1))
		{
			for (int_t k1 = 0; k1 < 2; k1++)
			{
				float f3 = (float)i + random.nextFloat();
				float f9 = (float)j + random.nextFloat();
				float f15 = (float)k + random.nextFloat() * 0.1f;
				world->spawnParticle("largesmoke", f3, f9, f15, 0.0, 0.0, 0.0);
			}
		}
		if (Block::fire->canBlockCatchFire(world, i, j, k + 1))
		{
			for (int_t l1 = 0; l1 < 2; l1++)
			{
				float f4 = (float)i + random.nextFloat();
				float f10 = (float)j + random.nextFloat();
				float f16 = (float)(k + 1) - random.nextFloat() * 0.1f;
				world->spawnParticle("largesmoke", f4, f10, f16, 0.0, 0.0, 0.0);
			}
		}
		if (Block::fire->canBlockCatchFire(world, i, j + 1, k))
		{
			for (int_t i2 = 0; i2 < 2; i2++)
			{
				float f5 = (float)i + random.nextFloat();
				float f11 = (float)(j + 1) - random.nextFloat() * 0.1f;
				float f17 = (float)k + random.nextFloat();
				world->spawnParticle("largesmoke", f5, f11, f17, 0.0, 0.0, 0.0);
			}
		}
	}
}
