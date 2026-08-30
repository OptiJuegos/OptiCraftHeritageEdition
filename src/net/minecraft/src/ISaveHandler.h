#pragma once

#include <vector>
#include <string>
#include "java/String.h"

class WorldInfo;
class WorldProvider;
class IChunkLoader;
class NBTTagCompound;
class EntityPlayer;

// net.minecraft.src.ISaveHandler
class ISaveHandler
{
public:
	virtual ~ISaveHandler() = default;
	virtual WorldInfo *loadWorldInfo() = 0;
	virtual void checkSessionLock() = 0;
	virtual IChunkLoader *getChunkLoader(WorldProvider *worldprovider) = 0;
	virtual void saveWorldInfoAndPlayer(WorldInfo *worldinfo, const std::vector<EntityPlayer *> &players) = 0;
	virtual void saveWorldInfo(WorldInfo *worldinfo) = 0;
	virtual std::string getMapFile(const jstring &name) = 0;
	virtual std::string getSaveDirectoryName() const = 0;
};
