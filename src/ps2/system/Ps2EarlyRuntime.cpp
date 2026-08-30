#ifdef PS2_PLATFORM

#include "platform/Log.h"
#include <cstdio>

#include <sifrpc.h>

__attribute__((constructor(101)))
static void ps2EarlyRuntimeInit()
{
    SifInitRpc(0);
    MC_LOG_INFO("platform", "[PS2] early_debug_init: SIF up\n");
}

#endif // PS2_PLATFORM
