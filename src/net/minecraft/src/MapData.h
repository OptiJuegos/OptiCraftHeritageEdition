#pragma once

#include "MapDataBase.h"
#include "java/Type.h"
#include <vector>
#include <map>

class EntityPlayer;
class ItemStack;
class MapInfo;
class MapCoord;
class NBTTagCompound;

// net.minecraft.src.MapData
class MapData : public MapDataBase
{
public:
	MapData(const std::string &s);
	~MapData() override;

	void readFromNBT(NBTTagCompound *nbttagcompound) override;
	void writeToNBT(NBTTagCompound *nbttagcompound) override;

	void updateMapData(EntityPlayer *entityplayer, ItemStack *itemstack);
	void setColumnDirty(int_t i, int_t j, int_t k);
	void handleMapPacket(const std::vector<byte_t> &data);

	int_t xCenter;          // xCenter
	int_t zCenter;          // zCenter
	byte_t dimension;       // dimension
	byte_t scale;           // scale
	byte_t colors[16384];   // colors
	int_t achievementParent;
	std::vector<MapInfo*> playersOnMap;       // playersOnMap
	std::vector<MapCoord*> playersVisibleOnMap; // playersVisibleOnMap

private:
	std::map<EntityPlayer*, MapInfo*> playerMapInfo; // playerMapInfo
};
