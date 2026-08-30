#pragma once

#include "java/String.h"

class NBTTagCompound;

// net.minecraft.src.WorldSavedData
class WorldSavedData
{
public:
	explicit WorldSavedData(const jstring &mapName);
	virtual ~WorldSavedData() = default;

	virtual void readFromNBT(NBTTagCompound *tag) = 0;
	virtual void writeToNBT(NBTTagCompound *tag) = 0;

	void markDirty();
	void setDirty(bool dirty);
	bool isDirty() const;

	const jstring mapName;

private:
	bool dirty;
};
