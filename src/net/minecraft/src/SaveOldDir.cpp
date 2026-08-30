#include "SaveOldDir.h"

#include "McRegionChunkLoader.h"
#include "WorldProvider.h"
#include "WorldProviderHell.h"
#include "WorldInfo.h"

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <sys/stat.h>
#define MKDIR(p) mkdir(p, 0755)
#endif

SaveOldDir::SaveOldDir(const std::string &baseDir, const std::string &name, bool flag) :
    SaveHandler(baseDir, name, flag)
{
}

IChunkLoader* SaveOldDir::getChunkLoader(WorldProvider *worldprovider)
{
    const std::string &dir = getSaveDirectory();
    if (dynamic_cast<WorldProviderHell*>(worldprovider))
    {
        std::string dim1 = dir + "/DIM-1";
        MKDIR(dim1.c_str());
        return new McRegionChunkLoader(dim1);
    }
    return new McRegionChunkLoader(dir);
}

void SaveOldDir::saveWorldInfoAndPlayer(WorldInfo *worldinfo, const std::vector<EntityPlayer *> &list)
{
    worldinfo->setSaveVersion(19132);
    SaveHandler::saveWorldInfoAndPlayer(worldinfo, list);
}
