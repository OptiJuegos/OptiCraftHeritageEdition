#ifdef PS2_PLATFORM

#include "ps2/storage/save/Ps2SaveStorage.h"
#include "ps2/storage/Ps2Storage.h"
#include "ps2/storage/assets/Ps2Assets.h"
#include "platform/storage/PathUtils.h"
#include "platform/Storage.h"
#include <cstring>
#include "ps2/storage/save/Ps2MemoryCard.h"

namespace
{

struct SaveStorageState
{
    Ps2SaveStorage::Target target = Ps2SaveStorage::Target::MemoryCard;
    bool configurationSaveFailed = false;
    std::string massRoot;
    std::string displayRoot;
};

SaveStorageState& state()
{
    static SaveStorageState value;
    return value;
}

} // namespace

namespace Ps2SaveStorage
{

std::string configRoot() { return "mc0:/OPTICRAFT_CFG"; }

bool available(Target targetValue)
{
    if (targetValue == Target::MemoryCard)
        return Ps2MemoryCard::available();
    if (targetValue == Target::MassStorage)
        return Ps2Storage::massAvailable();
    return false;
}

bool readConfiguration(const std::string &path, std::vector<unsigned char> &out)
{
    const auto readBounded = [](const std::string &file, std::vector<unsigned char> &bytes) {
        const auto size = PlatformStorage::getFileSize(file);
        return size >= 0 && size <= 1024 * 1024 && PlatformStorage::readFile(file, bytes);
    };
    // A pending file means the last replacement did not complete. Keep using
    // the previous verified copy, never a partially overwritten options file.
    if (PlatformStorage::exists(path + ".pending"))
    {
        if (readBounded(path + ".bak", out)) return true;
        std::vector<unsigned char> pending;
        return readBounded(path, out) &&
            readBounded(path + ".pending", pending) && out == pending;
    }
    return readBounded(path, out);
}

bool writeConfiguration(const std::string &path, const void *data, std::size_t length)
{
    if (length > 1024 * 1024 || (length != 0 && data == nullptr) ||
        path.compare(0, configRoot().size() + 1, configRoot() + "/") != 0 ||
        !available(Target::MemoryCard)) return false;
    PlatformStorage::mkdirs(configRoot());
    std::vector<unsigned char> previous;
    const bool hadPrevious = readConfiguration(path, previous);
    const auto *bytes = static_cast<const unsigned char *>(data);
    const auto writeVerified = [&](const std::string &destination, const void *source, std::size_t size) {
        if (!PlatformStorage::writeFile(destination, source, size)) return false;
        std::vector<unsigned char> check;
        return PlatformStorage::readFile(destination, check) && check.size() == size &&
            (size == 0 || std::memcmp(check.data(), source, size) == 0);
    };
    // Resolve an interrupted replacement before reusing its journal files.
    if (PlatformStorage::exists(path + ".pending"))
    {
        if (hadPrevious && !writeVerified(path, previous.data(), previous.size())) return false;
        if (!PlatformStorage::removeFile(path + ".pending")) return false;
    }
    else if (!hadPrevious && PlatformStorage::exists(path))
        return false; // An unreadable existing file must not be overwritten.
    // Allocate and verify the new copy BEFORE touching either previous copy.
    if (!writeVerified(path + ".pending", bytes, length))
    {
        PlatformStorage::removeFile(path + ".pending");
        return false;
    }
    if (hadPrevious && !writeVerified(path + ".bak", previous.data(), previous.size()))
    {
        PlatformStorage::removeFile(path + ".pending");
        return false;
    }
    if (!writeVerified(path, bytes, length)) return false;
    return PlatformStorage::removeFile(path + ".pending");
}

bool configurationSaveFailed() { return state().configurationSaveFailed; }
void reportConfigurationSave(bool success) { state().configurationSaveFailed = !success; }

void setTarget(Target targetValue)
{
    state().target = targetValue;
    state().massRoot.clear();
    state().displayRoot.clear();

    if (targetValue == Target::MassStorage)
    {
        state().massRoot = Ps2Storage::massRoot();
        if (state().massRoot.empty())
            state().massRoot = "mass:/";

        // Saves live inside the install folder, next to data/. Following the
        // resolved install directory rather than a fixed name is what lets the
        // folder be called anything: rename it and the saves inside travel with
        // it, so there is nothing to migrate.
        //
        // Only when the install cannot hold them -- a read-only disc, or a host:
        // launch -- do saves need a home of their own, and then the conventional
        // name on mass storage is the one place both a disc build and a USB
        // build will agree on.
        const bool installIsWritable = Ps2Assets::source() == Ps2Assets::Source::UsbMass;
        const std::string installDir = Ps2Assets::installDir();
        if (installIsWritable && !installDir.empty())
            state().displayRoot = installDir;
        else
            state().displayRoot = PlatformStorage::join(state().massRoot, Ps2AssetLocator::INSTALL_FOLDER);
    }
}

Target target()
{
    return state().target;
}

bool enabled()
{
    return state().target != Target::Disabled;
}

std::string root()
{
    switch (state().target)
    {
        case Target::MemoryCard:
            return "mc0:";
        case Target::MassStorage:
            return state().displayRoot;
        case Target::Disabled:
        default:
            return "nosave:";
    }
}

const char* displayRoot()
{
    switch (state().target)
    {
        case Target::MemoryCard:
            return "mc0:";
        case Target::MassStorage:
            return state().displayRoot.c_str();
        case Target::Disabled:
        default:
            return "(disabled)";
    }
}

} // namespace Ps2SaveStorage

#endif // PS2_PLATFORM
