#ifdef PS2_PLATFORM
#include "ps2/storage/save/Ps2SaveSetup.h"
#include "ps2/storage/save/Ps2MemoryCard.h"
#include "ps2/storage/save/Ps2SaveStorage.h"
#include "platform/Storage.h"
#include "platform/Log.h"
#include <vector>

namespace Ps2SaveSetup
{
void selectStorage()
{
    // Never prompt, format a card, or redirect configuration onto USB.
    const bool ready = Ps2MemoryCard::initialize();
    Ps2SaveStorage::setTarget(Ps2SaveStorage::Target::MemoryCard);
    Ps2SaveStorage::reportConfigurationSave(ready);
    if (!ready)
    {
        MC_LOG_WARN("save", "[PS2] Memory Card unavailable; configuration remains on mc0.\n");
        return;
    }
    const std::string config = Ps2SaveStorage::configRoot();
    PlatformStorage::mkdirs(config);
    // Non-destructive compatibility import from the old Memory Card root.
    // Existing destination files always win; USB files and world folders are untouched.
    for (const char *name : {"options.txt", "servers.dat"})
    {
        const std::string destination = PlatformStorage::join(config, name);
        std::vector<unsigned char> bytes;
        if (!PlatformStorage::exists(destination) && !PlatformStorage::exists(destination + ".pending") &&
            PlatformStorage::readFile(PlatformStorage::join("mc0:", name), bytes))
        {
            if (!Ps2SaveStorage::writeConfiguration(destination, bytes.data(), bytes.size()))
                Ps2SaveStorage::reportConfigurationSave(false);
        }
    }
}
}
#endif
