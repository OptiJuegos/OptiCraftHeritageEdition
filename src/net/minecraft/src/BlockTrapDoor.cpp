#include "BlockTrapDoor.h"
#include "Material.h"
#include "IBlockAccess.h"
#include "World.h"
#include "AxisAlignedBB.h"
#include "EntityPlayer.h"
#include "Vec3D.h"
#include "MovingObjectPosition.h"
#include "Block.h"

BlockTrapDoor::BlockTrapDoor(int_t i, Material *material) : Block(i, material)
{
	blockIndexInTexture = 84;
	if (material == Material::iron) blockIndexInTexture++;
	float f  = 0.5f;
	float f1 = 1.0f;
	setBlockBounds(0.5f - f, 0.0f, 0.5f - f, 0.5f + f, f1, 0.5f + f);
}

bool  BlockTrapDoor::isOpaqueCube()        { return false; }
bool  BlockTrapDoor::renderAsNormalBlock() { return false; }
int_t BlockTrapDoor::getRenderType()       { return 0; }

AxisAlignedBB *BlockTrapDoor::getSelectedBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	setBlockBoundsBasedOnState(world, i, j, k);
	return Block::getSelectedBoundingBoxFromPool(world, i, j, k);
}

AxisAlignedBB *BlockTrapDoor::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	setBlockBoundsBasedOnState(world, i, j, k);
	return Block::getCollisionBoundingBoxFromPool(world, i, j, k);
}

void BlockTrapDoor::setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	setBlockBoundsForBlockRender(iblockaccess->getBlockMetadata(i, j, k));
}

void BlockTrapDoor::setBlockBoundsForItemRender()
{
	float f = 0.1875f;
	setBlockBounds(0.0f, 0.5f - f / 2.0f, 0.0f, 1.0f, 0.5f + f / 2.0f, 1.0f);
}

void BlockTrapDoor::setBlockBoundsForBlockRender(int_t i)
{
	float f = 0.1875f;
	setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, f, 1.0f);
	if (isTrapdoorOpen(i))
	{
		if ((i & 3) == 0) setBlockBounds(0.0f, 0.0f, 1.0f - f, 1.0f, 1.0f, 1.0f);
		if ((i & 3) == 1) setBlockBounds(0.0f, 0.0f, 0.0f,     1.0f, 1.0f, f);
		if ((i & 3) == 2) setBlockBounds(1.0f - f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
		if ((i & 3) == 3) setBlockBounds(0.0f, 0.0f, 0.0f,     f, 1.0f, 1.0f);
	}
}

void BlockTrapDoor::onBlockClicked(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
	blockActivated(world, i, j, k, entityplayer);
}

bool BlockTrapDoor::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
	if (blockMaterial == Material::iron) return true;
	int_t l = world->getBlockMetadata(i, j, k);
	world->setBlockMetadataWithNotify(i, j, k, l ^ 4);
	world->getTrapdoorFacing(entityplayer, 1003, i, j, k, 0);
	return true;
}

void BlockTrapDoor::onPoweredBlockChange(World *world, int_t i, int_t j, int_t k, bool flag)
{
	int_t l = world->getBlockMetadata(i, j, k);
	bool flag1 = (l & 4) > 0;
	if (flag1 == flag) return;
	world->setBlockMetadataWithNotify(i, j, k, l ^ 4);
	world->getTrapdoorFacing(nullptr, 1003, i, j, k, 0);
}

void BlockTrapDoor::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
	if (world->multiplayerWorld) return;
	int_t i1 = world->getBlockMetadata(i, j, k);
	int_t j1 = i;
	int_t k1 = k;
	if ((i1 & 3) == 0) k1++;
	if ((i1 & 3) == 1) k1--;
	if ((i1 & 3) == 2) j1++;
	if ((i1 & 3) == 3) j1--;
	if (!isValidSupportBlock(world->getBlockId(j1, j, k1)))
	{
		world->setBlockWithNotify(i, j, k, 0);
		dropBlockAsItem(world, i, j, k, i1);
	}
	bool powered = world->isBlockIndirectlyGettingPowered(i, j, k);
	if (powered || l == 0 || (l > 0 && Block::blocksList[l] != nullptr && Block::blocksList[l]->canProvidePower()))
	{
		onPoweredBlockChange(world, i, j, k, powered);
	}
}

MovingObjectPosition *BlockTrapDoor::collisionRayTrace(World *world, int_t i, int_t j, int_t k, Vec3D *vec3d, Vec3D *vec3d1)
{
	setBlockBoundsBasedOnState(world, i, j, k);
	return Block::collisionRayTrace(world, i, j, k, vec3d, vec3d1);
}

void BlockTrapDoor::onBlockPlaced(World *world, int_t i, int_t j, int_t k, int_t l)
{
	byte_t byte0 = 0;
	if (l == 2) byte0 = 0;
	if (l == 3) byte0 = 1;
	if (l == 4) byte0 = 2;
	if (l == 5) byte0 = 3;
	world->setBlockMetadataWithNotify(i, j, k, byte0);
}

bool BlockTrapDoor::canPlaceBlockOnSide(World *world, int_t i, int_t j, int_t k, int_t l)
{
	if (l == 0) return false;
	if (l == 1) return false;
	if (l == 2) k++;
	if (l == 3) k--;
	if (l == 4) i++;
	if (l == 5) i--;
	return isValidSupportBlock(world->getBlockId(i, j, k));
}

bool BlockTrapDoor::isTrapdoorOpen(int_t i)
{
	return (i & 4) != 0;
}

bool BlockTrapDoor::getBlocksMovement(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	return !isTrapdoorOpen(iblockaccess->getBlockMetadata(i, j, k));
}


bool BlockTrapDoor::isValidSupportBlock(int_t blockId)
{
	if (blockId <= 0 || blockId >= BLOCK_REGISTRY_SIZE)
		return false;
	Block *block = Block::blocksList[blockId];
	return (block != nullptr && block->blockMaterial->isOpaque() && block->renderAsNormalBlock()) || block == Block::glowStone;
}
