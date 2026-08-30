#pragma once

#include <string>

class NBTTagCompound;

// net.minecraft.src.MapDataBase
class MapDataBase
{
public:
	MapDataBase(const std::string &s);
	virtual ~MapDataBase() = default;

	virtual void readFromNBT(NBTTagCompound *nbttagcompound) = 0;
	virtual void writeToNBT(NBTTagCompound *nbttagcompound) = 0;

	void markDirty();
	void setDirty(bool flag);
	bool isDirty();

	std::string mapName; // mapName

private:
	bool dirty;
};
