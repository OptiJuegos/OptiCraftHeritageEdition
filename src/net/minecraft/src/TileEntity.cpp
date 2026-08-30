#include "TileEntity.h"

#include "platform/Log.h"
#include <iostream>
#include <stdexcept>
#include <typeinfo>

#include "Block.h"
#include "NBTTagCompound.h"
#include "World.h"

#include "TileEntityFurnace.h"
#include "TileEntityChest.h"
#include "TileEntityRecordPlayer.h"
#include "TileEntityDispenser.h"
#include "TileEntitySign.h"
#include "TileEntityMobSpawner.h"
#include "TileEntityNote.h"
#include "TileEntityPiston.h"
#include "TileEntityEnchantmentTable.h"
#include "TileEntityBrewingStand.h"
#include "TileEntityEndPortal.h"

std::map<std::string, std::function<TileEntity *()>> TileEntity::nameToClassMap;
std::map<std::string, std::string>                   TileEntity::classToNameMap;

// Java: static { addMapping(...); } block at the bottom of TileEntity.
// Registered the first time any TileEntity machinery is touched.
namespace
{
	struct TileEntityMappingInit
	{
		TileEntityMappingInit()
		{
			TileEntity::addMapping<TileEntityFurnace>("Furnace");
			TileEntity::addMapping<TileEntityChest>("Chest");
			TileEntity::addMapping<TileEntityRecordPlayer>("RecordPlayer");
			TileEntity::addMapping<TileEntityDispenser>("Trap");
			TileEntity::addMapping<TileEntitySign>("Sign");
			TileEntity::addMapping<TileEntityMobSpawner>("MobSpawner");
			TileEntity::addMapping<TileEntityNote>("Music");
			TileEntity::addMapping<TileEntityPiston>("Piston");
			TileEntity::addMapping<TileEntityEnchantmentTable>("EnchantTable");
			TileEntity::addMapping<TileEntityBrewingStand>("Cauldron");
			TileEntity::addMapping<TileEntityEndPortal>("Airportal");
		}
	};
	TileEntityMappingInit tileEntityMappingInit;
}

TileEntity::TileEntity()
	: worldObj(nullptr),
	  xCoord(0), yCoord(0), zCoord(0),
	  blockMetadata(-1), blockType(nullptr),
	  invalid(false)
{
}

void TileEntity::readFromNBT(NBTTagCompound *nbttagcompound)
{
	xCoord = nbttagcompound->getInteger("x");
	yCoord = nbttagcompound->getInteger("y");
	zCoord = nbttagcompound->getInteger("z");
}

void TileEntity::writeToNBT(NBTTagCompound *nbttagcompound)
{
	auto it = classToNameMap.find(typeid(*this).name());
	if (it == classToNameMap.end())
		throw std::runtime_error(std::string(typeid(*this).name()) + " is missing a mapping! This is a bug!");
	nbttagcompound->setString("id", it->second);
	nbttagcompound->setInteger("x", xCoord);
	nbttagcompound->setInteger("y", yCoord);
	nbttagcompound->setInteger("z", zCoord);
}

void TileEntity::updateEntity()
{
}

TileEntity *TileEntity::createAndLoadEntity(NBTTagCompound *nbttagcompound)
{
	TileEntity *tileentity = nullptr;
	try
	{
		auto it = nameToClassMap.find(nbttagcompound->getString("id"));
		if (it != nameToClassMap.end())
			tileentity = it->second();
	}
	catch (std::exception &ex)
	{
		MC_LOG_ERROR("game", "%s\n", ex.what());
	}
	if (tileentity != nullptr)
		tileentity->readFromNBT(nbttagcompound);
	else
		MC_LOG_WARN("entity", "Skipping TileEntity with id %s\n", nbttagcompound->getString("id").c_str());
	return tileentity;
}

int_t TileEntity::getBlockMetadata()
{
	if (blockMetadata == -1 && worldObj != nullptr)
		blockMetadata = worldObj->getBlockMetadata(xCoord, yCoord, zCoord);
	return blockMetadata;
}

void TileEntity::onInventoryChanged()
{
	if (worldObj != nullptr)
	{
		blockMetadata = worldObj->getBlockMetadata(xCoord, yCoord, zCoord);
		worldObj->markTileEntityChunkModified(xCoord, yCoord, zCoord, this);
	}
}

double TileEntity::getDistanceFrom(double d, double d1, double d2)
{
	double d3 = ((double)xCoord + 0.5) - d;
	double d4 = ((double)yCoord + 0.5) - d1;
	double d5 = ((double)zCoord + 0.5) - d2;
	return d3 * d3 + d4 * d4 + d5 * d5;
}

Block *TileEntity::getBlockType()
{
	if (blockType == nullptr && worldObj != nullptr)
	{
		int_t id = worldObj->getBlockId(xCoord, yCoord, zCoord);
		if (id >= 0 && id < Block::BLOCK_REGISTRY_SIZE)
			blockType = Block::blocksList[id];
	}
	return blockType;
}

void TileEntity::invalidate()
{
	invalid = true;
}

void TileEntity::validate()
{
	invalid = false;
}

void TileEntity::onTileEntityPowered(int_t, int_t)
{
}

void TileEntity::updateContainingBlockInfo()
{
	blockType = nullptr;
	blockMetadata = -1;
}
