#include "MapData.h"
#include "java/Arithmetic.h"

#include <algorithm>
#include <cstdint>
#include "MapInfo.h"
#include "MapCoord.h"
#include "EntityPlayer.h"
#include "ItemStack.h"
#include "InventoryPlayer.h"
#include "NBTTagCompound.h"
#include "MathHelper.h"

MapData::MapData(const std::string &s) :
	MapDataBase(s),
	xCenter(0),
	zCenter(0),
	dimension(0),
	scale(0),
	achievementParent(0)
{
	for (int_t i = 0; i < 16384; i++)
		colors[i] = 0;
}

MapData::~MapData()
{
	for (MapCoord *coord : playersVisibleOnMap)
		delete coord;
	playersVisibleOnMap.clear();

	for (MapInfo *info : playersOnMap)
		delete info;
	playersOnMap.clear();
	playerMapInfo.clear();
}

void MapData::readFromNBT(NBTTagCompound *nbttagcompound)
{
	dimension = nbttagcompound->getByte("dimension");
	xCenter = nbttagcompound->getInteger("xCenter");
	zCenter = nbttagcompound->getInteger("zCenter");
	scale = nbttagcompound->getByte("scale");
	if (scale < 0) scale = 0;
	if (scale > 4) scale = 4;

	short_t width = nbttagcompound->getShort("width");
	short_t height = nbttagcompound->getShort("height");

	if (width == 128 && height == 128)
	{
		std::vector<byte_t> storedColors = nbttagcompound->getByteArray("colors");
		std::copy_n(storedColors.begin(), std::min<size_t>(storedColors.size(), 16384), colors);
	}
	else
	{
		std::vector<byte_t> storedColors = nbttagcompound->getByteArray("colors");
		for (int_t i = 0; i < 16384; i++)
			colors[i] = 0;

		int_t i = (128 - width) / 2;
		int_t j = (128 - height) / 2;
		for (int_t k = 0; k < height; k++)
		{
			int_t l = k + j;
			if (l < 0 || l >= 128) continue;
			for (int_t i1 = 0; i1 < width; i1++)
			{
				int_t j1 = i1 + i;
				if (j1 >= 0 && j1 < 128)
				{
					size_t sourceIndex = (size_t)i1 + (size_t)k * (size_t)width;
					if (sourceIndex < storedColors.size())
						colors[j1 + l * 128] = storedColors[sourceIndex];
				}
			}
		}
	}
}

void MapData::writeToNBT(NBTTagCompound *nbttagcompound)
{
	nbttagcompound->setByte("dimension", dimension);
	nbttagcompound->setInteger("xCenter", xCenter);
	nbttagcompound->setInteger("zCenter", zCenter);
	nbttagcompound->setByte("scale", scale);
	nbttagcompound->setShort("width", (short_t)128);
	nbttagcompound->setShort("height", (short_t)128);
	nbttagcompound->setByteArray("colors", std::vector<byte_t>(colors, colors + 16384));
}

void MapData::updateMapData(EntityPlayer *entityplayer, ItemStack *itemstack)
{
	auto it = playerMapInfo.find(entityplayer);
	if (it == playerMapInfo.end())
	{
		MapInfo *mapinfo = new MapInfo(this, entityplayer);
		playerMapInfo[entityplayer] = mapinfo;
		playersOnMap.push_back(mapinfo);
	}
	for (MapCoord *coord : playersVisibleOnMap)
		delete coord;
	playersVisibleOnMap.clear();
	for (auto itInfo = playersOnMap.begin(); itInfo != playersOnMap.end(); )
	{
		MapInfo *mapinfo1 = *itInfo;
		if (mapinfo1 == nullptr || mapinfo1->entityplayerObj == nullptr ||
			mapinfo1->entityplayerObj->isDead ||
			!mapinfo1->entityplayerObj->inventory->doesXZShowFog(itemstack))
		{
			if (mapinfo1 != nullptr && mapinfo1->entityplayerObj != nullptr)
				playerMapInfo.erase(mapinfo1->entityplayerObj);
			delete mapinfo1;
			itInfo = playersOnMap.erase(itInfo);
			continue;
		}
		++itInfo;
		float f = (float)(mapinfo1->entityplayerObj->posX - (double)xCenter) / (float)JavaArithmetic::intShl(1, scale);
		float f1 = (float)(mapinfo1->entityplayerObj->posZ - (double)zCenter) / (float)JavaArithmetic::intShl(1, scale);
		int_t j = 64;
		int_t k = 64;
		if (f < (float)(-j) || f1 < (float)(-k) || f > (float)j || f1 > (float)k)
			continue;

		byte_t byte0 = 0;
		byte_t byte1 = JavaArithmetic::byteFromBits(static_cast<ubyte_t>(JavaArithmetic::doubleToInt(static_cast<double>(f * 2.0f) + 0.5)));
		byte_t byte2 = JavaArithmetic::byteFromBits(static_cast<ubyte_t>(JavaArithmetic::doubleToInt(static_cast<double>(f1 * 2.0f) + 0.5)));
		byte_t byte3 = JavaArithmetic::byteFromBits(static_cast<ubyte_t>(JavaArithmetic::doubleToInt(static_cast<double>((entityplayer->rotationYaw * 16.0f) / 360.0f) + 0.5)));
		if (dimension < 0)
		{
			int32_t l = achievementParent / 10;
			// Java int multiplication wraps at 32 bits. Do it explicitly here so
			// long-running map updates cannot trigger signed-overflow UB in C++.
			uint32_t lu = static_cast<uint32_t>(l);
			uint32_t mixed = lu * lu * 34187121u + lu * 121u;
			byte3 = (byte_t)((JavaArithmetic::intFromBits(mixed) >> 15) & 0xf);
		}
		if (mapinfo1->entityplayerObj->dimension == dimension)
		{
			playersVisibleOnMap.push_back(new MapCoord(this, byte0, byte1, byte2, byte3));
		}
	}
}

void MapData::setColumnDirty(int_t i, int_t j, int_t k)
{
	MapDataBase::markDirty();
	for (auto *mapinfo : playersOnMap)
	{
		if (mapinfo->minVisibleY[i] < 0 || mapinfo->minVisibleY[i] > j)
			mapinfo->minVisibleY[i] = j;
		if (mapinfo->maxVisibleY[i] < 0 || mapinfo->maxVisibleY[i] < k)
			mapinfo->maxVisibleY[i] = k;
	}
}

void MapData::handleMapPacket(const std::vector<byte_t> &data)
{
	if (data.empty())
		return;

	if (data[0] == 0 && data.size() >= 3)
	{
		int_t column = data[1] & 0xff;
		int_t firstRow = data[2] & 0xff;
		for (size_t offset = 3; offset < data.size(); offset++)
		{
			size_t row = (size_t)firstRow + offset - 3;
			if (column < 128 && row < 128)
				colors[row * 128 + (size_t)column] = data[offset];
		}
		markDirty();
	}
	else if (data[0] == 1)
	{
		for (MapCoord *coord : playersVisibleOnMap)
			delete coord;
		playersVisibleOnMap.clear();
		for (size_t offset = 1; offset + 2 < data.size(); offset += 3)
		{
			byte_t byte0 = (byte_t)(data[offset] % 16);
			byte_t byte1 = data[offset + 1];
			byte_t byte2 = data[offset + 2];
			byte_t byte3 = (byte_t)(data[offset] / 16);
			playersVisibleOnMap.push_back(new MapCoord(this, byte0, byte1, byte2, byte3));
		}
	}
}
