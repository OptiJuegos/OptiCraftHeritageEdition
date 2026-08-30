#pragma once

#include "SaveHandler.h"

// net.minecraft.src.AnvilSaveHandler
class AnvilSaveHandler : public SaveHandler
{
public:
    AnvilSaveHandler(const std::string &baseDir, const std::string &name, bool createPlayers);

    IChunkLoader *getChunkLoader(WorldProvider *worldprovider) override;
    void saveWorldInfoAndPlayer(WorldInfo *worldinfo,
                                const std::vector<EntityPlayer *> &players) override;
};
