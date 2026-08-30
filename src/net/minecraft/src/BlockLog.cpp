#include "BlockLog.h"
#include "Material.h"
#include "World.h"
#include "Block.h"
#include "EntityPlayer.h"
#include "BlockLeaves.h"

BlockLog::BlockLog(int_t i) : Block(i, Material::wood)
{
	blockIndexInTexture = 20;
}

int_t BlockLog::quantityDropped(Random &random) { return 1; }

int_t BlockLog::idDropped(int_t i, Random &random)
{
	return Block::wood->blockID;
}

void BlockLog::harvestBlock(World *world, EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t l)
{
	Block::harvestBlock(world, entityplayer, i, j, k, l);
}

void BlockLog::onBlockRemoval(World *world, int_t i, int_t j, int_t k)
{
	byte_t byte0 = 4;
	int_t l = byte0 + 1;
	if (world->checkChunksExist(i - l, j - l, k - l, i + l, j + l, k + l))
	{
		for (int_t i1 = -byte0; i1 <= byte0; i1++)
		{
			for (int_t j1 = -byte0; j1 <= byte0; j1++)
			{
				for (int_t k1 = -byte0; k1 <= byte0; k1++)
				{
					int_t l1 = world->getBlockId(i + i1, j + j1, k + k1);
					if (l1 != Block::leaves->blockID) continue;
					int_t i2 = world->getBlockMetadata(i + i1, j + j1, k + k1);
					if ((i2 & 8) == 0)
					{
						world->setBlockMetadata(i + i1, j + j1, k + k1, i2 | 8);
					}
				}
			}
		}
	}
}

int_t BlockLog::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
	if (i == 1) return 21;
	if (i == 0) return 21;
	if (j == 1) return 116;
	if (j == 2) return 117;
	if (j == 3) return 153;
	return 20;
}

int_t BlockLog::damageDropped(int_t i) { return i; }
