#include "BlockEndPortalFrame.h"

#include "EntityLiving.h"
#include "Material.h"
#include "MathHelper.h"
#include "World.h"

BlockEndPortalFrame::BlockEndPortalFrame(int_t id) :
	Block(id, 159, Material::glass)
{
}

int_t BlockEndPortalFrame::getBlockTextureFromSideAndMetadata(int_t side, int_t metadata)
{
	return side == 1 ? blockIndexInTexture - 1 : (side == 0 ? blockIndexInTexture + 16 : blockIndexInTexture);
}

bool BlockEndPortalFrame::isOpaqueCube()
{
	return false;
}

int_t BlockEndPortalFrame::getRenderType()
{
	return 26;
}

void BlockEndPortalFrame::setBlockBoundsForItemRender()
{
	setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 13.0f / 16.0f, 1.0f);
}

void BlockEndPortalFrame::getCollidingBoundingBoxes(World *world, int_t x, int_t y, int_t z, AxisAlignedBB *mask, std::vector<AxisAlignedBB *> &list)
{
	setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 13.0f / 16.0f, 1.0f);
	Block::getCollidingBoundingBoxes(world, x, y, z, mask, list);
	if (isEnderEyeInserted(world->getBlockMetadata(x, y, z)))
	{
		setBlockBounds(5.0f / 16.0f, 13.0f / 16.0f, 5.0f / 16.0f, 11.0f / 16.0f, 1.0f, 11.0f / 16.0f);
		Block::getCollidingBoundingBoxes(world, x, y, z, mask, list);
	}
	setBlockBoundsForItemRender();
}

bool BlockEndPortalFrame::isEnderEyeInserted(int_t metadata)
{
	return (metadata & 4) != 0;
}

int_t BlockEndPortalFrame::idDropped(int_t metadata, Random &random, int_t fortune)
{
	return 0;
}

void BlockEndPortalFrame::onBlockPlacedBy(World *world, int_t x, int_t y, int_t z, EntityLiving *entityliving)
{
	const int_t metadata = ((MathHelper::floor_double((double)(entityliving->rotationYaw * 4.0f / 360.0f) + 0.5) & 3) + 2) % 4;
	world->setBlockMetadataWithNotify(x, y, z, metadata);
}
