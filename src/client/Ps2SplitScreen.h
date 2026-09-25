#pragma once

class Minecraft;

namespace Ps2SplitScreen
{
#ifdef PS2_PLATFORM
    void joinPlayer2(Minecraft *mc);
    void leavePlayer2(Minecraft *mc);
    void tick(Minecraft *mc);
    void postTick(Minecraft *mc);
#else
    inline void joinPlayer2(Minecraft *mc) { (void)mc; }
    inline void leavePlayer2(Minecraft *mc) { (void)mc; }
    inline void tick(Minecraft *mc) { (void)mc; }
    inline void postTick(Minecraft *mc) { (void)mc; }
#endif
}
