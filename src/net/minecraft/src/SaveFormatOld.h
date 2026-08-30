#pragma once

#include "ISaveFormat.h"
#include "java/File.h"
#include <string>
#include <vector>

class SaveFormatComparator;
class WorldInfo;
class ISaveHandler;
class IProgressUpdate;

// net.minecraft.src.SaveFormatOld
class SaveFormatOld : public ISaveFormat
{
public:
    explicit SaveFormatOld(const std::string &baseDir);
    ~SaveFormatOld() override;

    jstring getSaveFormatName() override;
    ISaveHandler* getSaveLoader(const jstring &s, bool flag) override;
    std::vector<SaveFormatComparator *> getSaveList() override;
    void flushCache() override;
    WorldInfo* getWorldInfo(const jstring &s) override;
    void deleteWorldDirectory(const jstring &s) override;
    void renameWorld(const jstring &s, const jstring &s1) override;
    bool isOldMapFormat(const jstring &s) override;
    bool convertMapFormat(const jstring &s, IProgressUpdate *iprogressupdate) override;

protected:
    // Java: protected final File field_22180_a
    File* field_22180_a;
    std::string baseDirectory; // UTF-8 string kept for subclass (SaveConverterMcRegion) use

    WorldInfo* getWorldInfoUTF8(const std::string &s);
};
