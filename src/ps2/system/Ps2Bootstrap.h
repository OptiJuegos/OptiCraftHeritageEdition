#pragma once

#ifdef PS2_PLATFORM

namespace Ps2Bootstrap
{
    bool initialize(int argc, char** argv);
    [[noreturn]] void finishGame();
}

#endif // PS2_PLATFORM
