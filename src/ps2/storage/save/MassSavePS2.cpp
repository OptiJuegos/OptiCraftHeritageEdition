#ifdef PS2_PLATFORM

#include "ps2/storage/save/MassSavePS2.h"
#include "ps2/storage/Ps2Storage.h"

bool ps2_mass_connected()
{
    return Ps2Storage::massAvailable();
}

#endif // PS2_PLATFORM
