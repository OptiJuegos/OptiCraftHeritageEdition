#include "BlockPistonBase.h"

#include "BlockContainer.h"
#include "BlockPistonExtension.h"
#include "BlockPistonMoving.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "IBlockAccess.h"
#include "Material.h"
#include "MathHelper.h"
#include "PistonBlockTextures.h"
#include "TileEntity.h"
#include "TileEntityPiston.h"
#include "World.h"
#include "java/Random.h"

BlockPistonBase::BlockPistonBase(int_t i, int_t j, bool flag) :
	Block(i, j, Material::piston),
	isSticky(flag),
	isMoving(false)
{
	setStepSound(soundStoneFootstep);
	setHardness(0.5f);
}

int_t BlockPistonBase::getPistonHeadTexture()
{
	return !isSticky ? 107 : 106;
}

int_t BlockPistonBase::getPistonExtensionTexture()
{
	return getPistonHeadTexture();
}

int_t BlockPistonBase::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
	int_t k = getPistonOrientation(j);
	if (k > 5)
	{
		return blockIndexInTexture;
	}
	if (i == k)
	{
		if (isPowered(j) || minX > 0.0 || minY > 0.0 || minZ > 0.0 || maxX < 1.0 || maxY < 1.0 || maxZ < 1.0)
		{
			return 110;
		}
		else
		{
			return blockIndexInTexture;
		}
	}
	return i != PistonBlockTextures::oppositeFacing[k] ? 108 : 109;
}

int_t BlockPistonBase::getRenderType()
{
	return 16;
}

bool BlockPistonBase::isOpaqueCube()
{
	return false;
}

bool BlockPistonBase::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
	return false;
}

void BlockPistonBase::onBlockPlacedBy(World *world, int_t i, int_t j, int_t k, EntityLiving *entityliving)
{
	int_t l = getOrientationFromEntity(world, i, j, k, entityliving);
	world->setBlockMetadataWithNotify(i, j, k, l);
	if (!world->multiplayerWorld)
	{
		updatePistonState(world, i, j, k);
	}
}

void BlockPistonBase::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
	if (!world->multiplayerWorld && !isMoving)
	{
		updatePistonState(world, i, j, k);
	}
}

void BlockPistonBase::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
	if (!world->multiplayerWorld && world->getBlockTileEntity(i, j, k) == nullptr)
	{
		updatePistonState(world, i, j, k);
	}
}

void BlockPistonBase::updatePistonState(World *world, int_t i, int_t j, int_t k)
{
	int_t l = world->getBlockMetadata(i, j, k);
	int_t i1 = getPistonOrientation(l);
	bool flag = isIndirectlyPowered(world, i, j, k, i1);
	if (l == 7)
	{
		return;
	}
	if (flag && !isPowered(l))
	{
		if (canExtend(world, i, j, k, i1))
		{
			world->setBlockMetadata(i, j, k, i1 | 8);
			world->playNoteAt(i, j, k, 0, i1);
		}
	}
	else if (!flag && isPowered(l))
	{
		world->setBlockMetadata(i, j, k, i1);
		world->playNoteAt(i, j, k, 1, i1);
	}
}

bool BlockPistonBase::isIndirectlyPowered(World *world, int_t i, int_t j, int_t k, int_t l)
{
	if (l != 0 && world->isBlockIndirectlyProvidingPowerTo(i, j - 1, k, 0)) return true;
	if (l != 1 && world->isBlockIndirectlyProvidingPowerTo(i, j + 1, k, 1)) return true;
	if (l != 2 && world->isBlockIndirectlyProvidingPowerTo(i, j, k - 1, 2)) return true;
	if (l != 3 && world->isBlockIndirectlyProvidingPowerTo(i, j, k + 1, 3)) return true;
	if (l != 5 && world->isBlockIndirectlyProvidingPowerTo(i + 1, j, k, 5)) return true;
	if (l != 4 && world->isBlockIndirectlyProvidingPowerTo(i - 1, j, k, 4)) return true;
	if (world->isBlockIndirectlyProvidingPowerTo(i, j, k, 0)) return true;
	if (world->isBlockIndirectlyProvidingPowerTo(i, j + 2, k, 1)) return true;
	if (world->isBlockIndirectlyProvidingPowerTo(i, j + 1, k - 1, 2)) return true;
	if (world->isBlockIndirectlyProvidingPowerTo(i, j + 1, k + 1, 3)) return true;
	if (world->isBlockIndirectlyProvidingPowerTo(i - 1, j + 1, k, 4)) return true;
	return world->isBlockIndirectlyProvidingPowerTo(i + 1, j + 1, k, 5);
}

void BlockPistonBase::playBlock(World *world, int_t i, int_t j, int_t k, int_t l, int_t i1)
{
	isMoving = true;
	int_t j1 = i1;
	if (l == 0)
	{
		if (tryExtend(world, i, j, k, j1))
		{
			world->setBlockMetadataWithNotify(i, j, k, j1 | 8);
			world->playSoundEffect((double)i + 0.5, (double)j + 0.5, (double)k + 0.5, "tile.piston.out", 0.5f, world->rand.nextFloat() * 0.25f + 0.6f);
		}
		else
		{
			world->setBlockMetadata(i, j, k, j1);
		}
	}
	else if (l == 1)
	{
		TileEntity *tileentity = world->getBlockTileEntity(i + PistonBlockTextures::deltaX[j1], j + PistonBlockTextures::deltaY[j1], k + PistonBlockTextures::deltaZ[j1]);
		TileEntityPiston *tileentitypiston = dynamic_cast<TileEntityPiston *>(tileentity);
		if (tileentity != nullptr && tileentitypiston != nullptr)
		{
			tileentitypiston->clearPistonTileEntity();
		}
		world->setBlockAndMetadata(i, j, k, Block::pistonMoving->blockID, j1);
		world->setBlockTileEntity(i, j, k, BlockPistonMoving::createMovingPistonTileEntity(blockID, j1, j1, false, true));
		if (isSticky)
		{
			int_t k1 = i + PistonBlockTextures::deltaX[j1] * 2;
			int_t l1 = j + PistonBlockTextures::deltaY[j1] * 2;
			int_t i2 = k + PistonBlockTextures::deltaZ[j1] * 2;
			int_t j2 = world->getBlockId(k1, l1, i2);
			int_t k2 = world->getBlockMetadata(k1, l1, i2);
			bool flag = false;
			if (j2 == Block::pistonMoving->blockID)
			{
				TileEntity *tileentity1 = world->getBlockTileEntity(k1, l1, i2);
				TileEntityPiston *tileentitypiston1 = dynamic_cast<TileEntityPiston *>(tileentity1);
				if (tileentity1 != nullptr && tileentitypiston1 != nullptr)
				{
					if (tileentitypiston1->getOrientation() == j1 && tileentitypiston1->isExtending())
					{
						tileentitypiston1->clearPistonTileEntity();
						j2 = tileentitypiston1->getStoredBlockID();
						k2 = tileentitypiston1->getBlockMetadata();
						flag = true;
					}
				}
			}
			if (!flag && j2 > 0 && canPushBlock(j2, world, k1, l1, i2, false) && (Block::blocksList[j2]->getMobilityFlag() == 0 || j2 == Block::pistonBase->blockID || j2 == Block::pistonStickyBase->blockID))
			{
				i += PistonBlockTextures::deltaX[j1];
				j += PistonBlockTextures::deltaY[j1];
				k += PistonBlockTextures::deltaZ[j1];
				world->setBlockAndMetadata(i, j, k, Block::pistonMoving->blockID, k2);
				world->setBlockTileEntity(i, j, k, BlockPistonMoving::createMovingPistonTileEntity(j2, k2, j1, false, false));
				isMoving = false;
				world->setBlockWithNotify(k1, l1, i2, 0);
				isMoving = true;
			}
			else if (!flag)
			{
				isMoving = false;
				world->setBlockWithNotify(i + PistonBlockTextures::deltaX[j1], j + PistonBlockTextures::deltaY[j1], k + PistonBlockTextures::deltaZ[j1], 0);
				isMoving = true;
			}
		}
		else
		{
			isMoving = false;
			world->setBlockWithNotify(i + PistonBlockTextures::deltaX[j1], j + PistonBlockTextures::deltaY[j1], k + PistonBlockTextures::deltaZ[j1], 0);
			isMoving = true;
		}
		world->playSoundEffect((double)i + 0.5, (double)j + 0.5, (double)k + 0.5, "tile.piston.in", 0.5f, world->rand.nextFloat() * 0.15f + 0.6f);
	}
	isMoving = false;
}

void BlockPistonBase::setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	int_t l = iblockaccess->getBlockMetadata(i, j, k);
	if (isPowered(l))
	{
		switch (getPistonOrientation(l))
		{
		case 0: setBlockBounds(0.0f, 0.25f, 0.0f, 1.0f, 1.0f, 1.0f); break;
		case 1: setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 0.75f, 1.0f); break;
		case 2: setBlockBounds(0.0f, 0.0f, 0.25f, 1.0f, 1.0f, 1.0f); break;
		case 3: setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.75f); break;
		case 4: setBlockBounds(0.25f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f); break;
		case 5: setBlockBounds(0.0f, 0.0f, 0.0f, 0.75f, 1.0f, 1.0f); break;
		}
	}
	else
	{
		setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	}
}

void BlockPistonBase::setBlockBoundsForItemRender()
{
	setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}

void BlockPistonBase::getCollidingBoundingBoxes(World *world, int_t i, int_t j, int_t k, AxisAlignedBB *axisalignedbb, std::vector<AxisAlignedBB *> &arraylist)
{
	setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	Block::getCollidingBoundingBoxes(world, i, j, k, axisalignedbb, arraylist);
}

AxisAlignedBB *BlockPistonBase::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	setBlockBoundsBasedOnState(world, i, j, k);
	return Block::getCollisionBoundingBoxFromPool(world, i, j, k);
}

bool BlockPistonBase::renderAsNormalBlock()
{
	return false;
}

int_t BlockPistonBase::getPistonOrientation(int_t i)
{
	return i & 7;
}

bool BlockPistonBase::isPowered(int_t i)
{
	return (i & 8) != 0;
}

bool BlockPistonBase::isExtended(int_t i)
{
	return isPowered(i);
}

int_t BlockPistonBase::getOrientationFromEntity(World *world, int_t i, int_t j, int_t k, EntityLiving *entityliving)
{
	if (MathHelper::abs((float)entityliving->posX - (float)i) < 2.0f && MathHelper::abs((float)entityliving->posZ - (float)k) < 2.0f)
	{
		double d = (entityliving->posY + 1.8200000000000001) - (double)entityliving->yOffset;
		if (d - (double)j > 2.0)
		{
			return 1;
		}
		if ((double)j - d > 0.0)
		{
			return 0;
		}
	}
	int_t l = MathHelper::floor_double((double)((entityliving->rotationYaw * 4.0f) / 360.0f) + 0.5) & 3;
	if (l == 0) return 2;
	if (l == 1) return 5;
	if (l == 2) return 3;
	return l != 3 ? 0 : 4;
}

bool BlockPistonBase::canPushBlock(int_t i, World *world, int_t j, int_t k, int_t l, bool flag)
{
	if (i == Block::obsidian->blockID)
	{
		return false;
	}
	if (i == Block::pistonBase->blockID || i == Block::pistonStickyBase->blockID)
	{
		if (isPowered(world->getBlockMetadata(j, k, l)))
		{
			return false;
		}
	}
	else
	{
		if (Block::blocksList[i]->getHardness() == -1.0f)
		{
			return false;
		}
		if (Block::blocksList[i]->getMobilityFlag() == 2)
		{
			return false;
		}
		if (!flag && Block::blocksList[i]->getMobilityFlag() == 1)
		{
			return false;
		}
	}
	return dynamic_cast<BlockContainer *>(Block::blocksList[i]) == nullptr;
}

bool BlockPistonBase::canExtend(World *world, int_t i, int_t j, int_t k, int_t l)
{
	int_t i1 = i + PistonBlockTextures::deltaX[l];
	int_t j1 = j + PistonBlockTextures::deltaY[l];
	int_t k1 = k + PistonBlockTextures::deltaZ[l];
	int_t l1 = 0;
	do
	{
		if (l1 >= 13)
		{
			break;
		}
		if (j1 <= 0 || j1 >= 255)
		{
			return false;
		}
		int_t i2 = world->getBlockId(i1, j1, k1);
		if (i2 == 0)
		{
			break;
		}
		if (!canPushBlock(i2, world, i1, j1, k1, true))
		{
			return false;
		}
		if (Block::blocksList[i2]->getMobilityFlag() == 1)
		{
			break;
		}
		if (l1 == 12)
		{
			return false;
		}
		i1 += PistonBlockTextures::deltaX[l];
		j1 += PistonBlockTextures::deltaY[l];
		k1 += PistonBlockTextures::deltaZ[l];
		l1++;
	} while (true);
	return true;
}

bool BlockPistonBase::tryExtend(World *world, int_t i, int_t j, int_t k, int_t l)
{
	int_t i1 = i + PistonBlockTextures::deltaX[l];
	int_t j1 = j + PistonBlockTextures::deltaY[l];
	int_t k1 = k + PistonBlockTextures::deltaZ[l];
	int_t l1 = 0;
	do
	{
		if (l1 >= 13)
		{
			break;
		}
		if (j1 <= 0 || j1 >= 255)
		{
			return false;
		}
		int_t j2 = world->getBlockId(i1, j1, k1);
		if (j2 == 0)
		{
			break;
		}
		if (!canPushBlock(j2, world, i1, j1, k1, true))
		{
			return false;
		}
		if (Block::blocksList[j2]->getMobilityFlag() == 1)
		{
			Block::blocksList[j2]->dropBlockAsItem(world, i1, j1, k1, world->getBlockMetadata(i1, j1, k1));
			world->setBlockWithNotify(i1, j1, k1, 0);
			break;
		}
		if (l1 == 12)
		{
			return false;
		}
		i1 += PistonBlockTextures::deltaX[l];
		j1 += PistonBlockTextures::deltaY[l];
		k1 += PistonBlockTextures::deltaZ[l];
		l1++;
	} while (true);
	int_t l2;
	for (; i1 != i || j1 != j || k1 != k; k1 = l2)
	{
		int_t i2 = i1 - PistonBlockTextures::deltaX[l];
		int_t k2 = j1 - PistonBlockTextures::deltaY[l];
		l2 = k1 - PistonBlockTextures::deltaZ[l];
		int_t i3 = world->getBlockId(i2, k2, l2);
		int_t j3 = world->getBlockMetadata(i2, k2, l2);
		if (i3 == blockID && i2 == i && k2 == j && l2 == k)
		{
			world->setBlockAndMetadata(i1, j1, k1, Block::pistonMoving->blockID, l | (isSticky ? 8 : 0));
			world->setBlockTileEntity(i1, j1, k1, BlockPistonMoving::createMovingPistonTileEntity(Block::pistonExtension->blockID, l | (isSticky ? 8 : 0), l, true, false));
		}
		else
		{
			world->setBlockAndMetadata(i1, j1, k1, Block::pistonMoving->blockID, j3);
			world->setBlockTileEntity(i1, j1, k1, BlockPistonMoving::createMovingPistonTileEntity(i3, j3, l, true, false));
		}
		i1 = i2;
		j1 = k2;
	}

	return true;
}
