#include "BlockContainer.h"

#include "World.h"
#include "TileEntity.h"

BlockContainer::BlockContainer(int_t i, Material *material) :
	Block(i, material)
{
	isBlockContainer[i] = true;
}

BlockContainer::BlockContainer(int_t i, int_t j, Material *material) :
	Block(i, j, material)
{
	isBlockContainer[i] = true;
}

void BlockContainer::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
	Block::onBlockAdded(world, i, j, k);
	world->setBlockTileEntity(i, j, k, getBlockEntity());
}

void BlockContainer::onBlockRemoval(World *world, int_t i, int_t j, int_t k)
{
	Block::onBlockRemoval(world, i, j, k);
	world->removeBlockTileEntity(i, j, k);
}

void BlockContainer::playBlock(World *world, int_t i, int_t j, int_t k, int_t eventId, int_t eventData)
{
	Block::playBlock(world, i, j, k, eventId, eventData);
	TileEntity *tile = world->getBlockTileEntity(i, j, k);
	if (tile != nullptr)
		tile->onTileEntityPowered(eventId, eventData);
}
