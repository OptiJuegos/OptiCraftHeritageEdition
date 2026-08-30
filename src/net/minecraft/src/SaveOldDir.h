#pragma once

#include "SaveHandler.h"
#include <vector>

class WorldProvider;
class WorldInfo;
class IChunkLoader;
class NBTTagCompound;
class EntityPlayer;

// net.minecraft.src.SaveOldDir
class SaveOldDir : public SaveHandler
{
public:
    SaveOldDir(const std::string &baseDir, const std::string &name, bool flag);

    IChunkLoader* getChunkLoader(WorldProvider *worldprovider) override;
    void saveWorldInfoAndPlayer(WorldInfo *worldinfo, const std::vector<EntityPlayer *> &list) override;
};
