#pragma once

#include "SaveFormatOld.h"
#include <vector>
#include <string>

class SaveFormatComparator;
class IProgressUpdate;
class ISaveHandler;
class ChunkFile;

// net.minecraft.src.SaveConverterMcRegion
class SaveConverterMcRegion : public SaveFormatOld
{
public:
    explicit SaveConverterMcRegion(const std::string &baseDir);

    jstring getSaveFormatName() override;
    std::vector<SaveFormatComparator *> getSaveList() override;
    void flushCache() override;
    ISaveHandler* getSaveLoader(const jstring &s, bool flag) override;
    bool isOldMapFormat(const jstring &s) override;
    bool convertMapFormat(const jstring &s, IProgressUpdate *iprogressupdate) override;

private:
    void scanChunkFiles(const std::string &dir, std::vector<ChunkFile> &chunks, std::vector<std::string> &folders);
    void convertChunkFiles(const std::string &worldDir, std::vector<ChunkFile> &chunks,
                           int_t doneCount, int_t totalCount, IProgressUpdate *progress);
    void deleteFolders(std::vector<std::string> &folders, int_t doneCount, int_t totalCount, IProgressUpdate *progress);
};
