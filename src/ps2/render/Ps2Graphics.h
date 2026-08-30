#pragma once

#ifdef PS2_PLATFORM

#include <gsKit.h>

// Renderer internals still use this context directly. Platform/bootstrap code
// should use the Ps2Graphics accessors instead.
extern GSGLOBAL* gsGlobal;

namespace Ps2Graphics
{
    void initialize();
    GSGLOBAL* context();
    int width();
    int height();
    bool isPal();
}

extern "C" long ps2_dbg_gs_queue_ram_bytes();

#endif // PS2_PLATFORM
