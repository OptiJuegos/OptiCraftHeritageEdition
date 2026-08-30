#include "SaveFormatComparator.h"

SaveFormatComparator::SaveFormatComparator(const std::string &fileName, const std::string &displayName,
                                           long_t lastPlayed, long_t sizeOnDisk, bool needsConversion) :
    fileName(fileName),
    displayName(displayName),
    lastTimePlayed(lastPlayed),
    sizeOnDisk_(sizeOnDisk),
    conversionRequired(needsConversion),
    gameType_(0),
    hardcore_(false)
{
}


SaveFormatComparator::SaveFormatComparator(const std::string &fileName, const std::string &displayName,
                                           long_t lastPlayed, long_t sizeOnDisk, int_t gameType,
                                           bool needsConversion, bool hardcore) :
    fileName(fileName),
    displayName(displayName),
    lastTimePlayed(lastPlayed),
    sizeOnDisk_(sizeOnDisk),
    conversionRequired(needsConversion),
    gameType_(gameType),
    hardcore_(hardcore)
{
}

const std::string& SaveFormatComparator::getFileName() const { return fileName; }
const std::string& SaveFormatComparator::getDisplayName() const { return displayName; }
long_t SaveFormatComparator::getSizeOnDisk() const { return sizeOnDisk_; }
bool SaveFormatComparator::requiresConversion() const { return conversionRequired; }
long_t SaveFormatComparator::getLastTimePlayed() const { return lastTimePlayed; }
int_t SaveFormatComparator::getGameType() const { return gameType_; }
bool SaveFormatComparator::isHardcoreModeEnabled() const { return hardcore_; }

int SaveFormatComparator::compareTo(const SaveFormatComparator &other) const
{
    if (lastTimePlayed < other.lastTimePlayed) return 1;
    if (lastTimePlayed > other.lastTimePlayed) return -1;
    return fileName.compare(other.fileName);
}

bool SaveFormatComparator::operator<(const SaveFormatComparator &other) const
{
    return compareTo(other) < 0;
}
