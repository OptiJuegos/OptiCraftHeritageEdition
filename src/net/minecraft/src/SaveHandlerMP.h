#pragma once

#include "ISaveHandler.h"

// net.minecraft.src.SaveHandlerMP
class SaveHandlerMP : public ISaveHandler
{
public:
    SaveHandlerMP() = default;

    WorldInfo* loadWorldInfo() override { return nullptr; }
    void checkSessionLock() override {}
    IChunkLoader* getChunkLoader(WorldProvider *worldprovider) override { return nullptr; }
    void saveWorldInfoAndPlayer(WorldInfo *worldinfo, const std::vector<EntityPlayer *> &players) override {}
    void saveWorldInfo(WorldInfo *worldinfo) override {}
    std::string getMapFile(const jstring &s) override { return ""; }
    std::string getSaveDirectoryName() const override { return "none"; }
};
