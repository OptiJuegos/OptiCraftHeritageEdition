#include "BlockRedstoneLight.h"

#include "Material.h"
#include "World.h"

BlockRedstoneLight::BlockRedstoneLight(int_t id, bool isPowered) :
	Block(id, 211, Material::redstoneLight),
	powered(isPowered)
{
	if (powered)
	{
		setLightValue(1.0f);
		++blockIndexInTexture;
	}
}

void BlockRedstoneLight::onBlockAdded(World *world, int_t x, int_t y, int_t z)
{
	if (world->multiplayerWorld)
	{
		return;
	}
	if (powered && !world->isBlockIndirectlyGettingPowered(x, y, z))
	{
		world->scheduleBlockUpdate(x, y, z, blockID, 4);
	}
	else if (!powered && world->isBlockIndirectlyGettingPowered(x, y, z) && redstoneLampActive != nullptr)
	{
		world->setBlockWithNotify(x, y, z, redstoneLampActive->blockID);
	}
}

void BlockRedstoneLight::onNeighborBlockChange(World *world, int_t x, int_t y, int_t z, int_t neighborId)
{
	if (world->multiplayerWorld)
	{
		return;
	}
	if (powered && !world->isBlockIndirectlyGettingPowered(x, y, z))
	{
		world->scheduleBlockUpdate(x, y, z, blockID, 4);
	}
	else if (!powered && world->isBlockIndirectlyGettingPowered(x, y, z) && redstoneLampActive != nullptr)
	{
		world->setBlockWithNotify(x, y, z, redstoneLampActive->blockID);
	}
}

void BlockRedstoneLight::updateTick(World *world, int_t x, int_t y, int_t z, Random &random)
{
	if (!world->multiplayerWorld && powered && !world->isBlockIndirectlyGettingPowered(x, y, z) && redstoneLampIdle != nullptr)
	{
		world->setBlockWithNotify(x, y, z, redstoneLampIdle->blockID);
	}
}

int_t BlockRedstoneLight::idDropped(int_t metadata, Random &random, int_t fortune)
{
	return redstoneLampIdle != nullptr ? redstoneLampIdle->blockID : blockID;
}
