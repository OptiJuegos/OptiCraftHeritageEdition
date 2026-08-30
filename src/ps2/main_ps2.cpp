#ifdef PS2_PLATFORM

#include "platform/Log.h"
#include "client/Minecraft.h"
#include "ps2/system/Ps2Bootstrap.h"

#include <cstdio>

int main(int argc, char* argv[])
{
    if (!Ps2Bootstrap::initialize(argc, argv))
        return 1;

    MC_LOG_INFO("game", "[PS2] Minecraft::start()\n");
    MC_LOG_DEBUG("ps2.boot", "[PS2] main: calling Minecraft::start\n");

    jstring username = "OptiPlayer";
    jstring auth = "-";
    Minecraft::start(&username, &auth);

    Ps2Bootstrap::finishGame();
}

#endif // PS2_PLATFORM
