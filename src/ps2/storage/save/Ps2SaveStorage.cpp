#ifdef PS2_PLATFORM

#include "ps2/storage/save/Ps2SaveStorage.h"
#include "ps2/storage/Ps2Storage.h"
#include "platform/storage/PathUtils.h"

namespace
{

struct SaveStorageState
{
    Ps2SaveStorage::Target target = Ps2SaveStorage::Target::Disabled;
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
        state().displayRoot = PlatformStorage::join(state().massRoot, "MCBETA");
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
