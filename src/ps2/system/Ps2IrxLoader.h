#pragma once

#ifdef PS2_PLATFORM

namespace Ps2IrxLoader
{
    // Load an IOP module from the resolved external data/ tree. The asset key
    // is relative to data/ (for example "irx/audsrv.irx"). If it is missing,
    // fallbackPath is passed to SifLoadModule when non-null/non-empty.
    // Returns the module id / loader result (>= 0 on success).
    int load(const char* assetKey, const char* fallbackPath = nullptr);
}

#endif // PS2_PLATFORM
