#pragma once

#include <string>
#include <vector>

#include "SaveFormatOld.h"

class IProgressUpdate;
class WorldChunkManager;

// net.minecraft.src.AnvilSaveConverter
class AnvilSaveConverter : public SaveFormatOld
{
public:
    explicit AnvilSaveConverter(const std::string &baseDir);

    jstring getSaveFormatName() override;
    std::vector<SaveFormatComparator *> getSaveList() override;
    void flushCache() override;
    ISaveHandler *getSaveLoader(const jstring &s, bool flag) override;
    bool isOldMapFormat(const jstring &s) override;
    bool convertMapFormat(const jstring &s, IProgressUpdate *progress) override;

protected:
    int_t getCurrentSaveVersion() const;

private:
    void backupLevelDat(const std::string &worldName);
    void scanRegionFiles(const std::string &dimensionDir, std::vector<std::string> &files) const;
    void convertRegionFiles(const std::vector<std::string> &files, WorldChunkManager *worldChunkManager,
                            int_t completedRegions, int_t totalRegions, IProgressUpdate *progress);
    void convertRegionFile(const std::string &inputPath, WorldChunkManager *worldChunkManager,
                           int_t completedRegions, int_t totalRegions, IProgressUpdate *progress);
};
