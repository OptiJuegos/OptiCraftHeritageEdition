#pragma once

#include <string>
#include "java/Type.h"

// net.minecraft.src.SaveFormatComparator
class SaveFormatComparator
{
public:
    SaveFormatComparator(const std::string &fileName, const std::string &displayName,
                         long_t lastPlayed, long_t sizeOnDisk, bool needsConversion);
    SaveFormatComparator(const std::string &fileName, const std::string &displayName,
                         long_t lastPlayed, long_t sizeOnDisk, int_t gameType,
                         bool needsConversion, bool hardcore);

    const std::string& getFileName() const;
    const std::string& getDisplayName() const;
    long_t getSizeOnDisk() const;
    bool requiresConversion() const;
    long_t getLastTimePlayed() const;
    int_t getGameType() const;
    bool isHardcoreModeEnabled() const;

    int compareTo(const SaveFormatComparator &other) const;
    bool operator<(const SaveFormatComparator &other) const;

private:
    std::string fileName;
    std::string displayName;
    long_t lastTimePlayed;
    long_t sizeOnDisk_;
    bool conversionRequired;
    int_t gameType_;
    bool hardcore_;
};
