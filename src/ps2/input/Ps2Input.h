#pragma once

#ifdef PS2_PLATFORM

namespace Ps2Input
{
    void initialize();
    bool waitUntilReady();
    void poll(bool inMenu);

    // Place the simulated menu cursor at an absolute framebuffer position.
    // Used by the container slot navigation (ContainerSlotNavigator) to step
    // between inventory slots; the D-pad and the left stick keep driving the
    // cursor normally on every other screen.
    void setMenuCursor(int x, int y);
}

#endif // PS2_PLATFORM
