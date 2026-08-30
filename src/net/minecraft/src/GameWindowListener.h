#pragma once

#include <thread>

class Minecraft;

// net.minecraft.src.GameWindowListener
// Java: extends WindowAdapter, responds to window close event.
// SDL2 equivalent: window close is handled via SDL_QUIT in the event loop,
// but this class mirrors the Java structure for completeness.
class GameWindowListener
{
public:
    GameWindowListener(Minecraft *minecraft, std::thread *mcThread);

    void windowClosing();

    Minecraft   *mc;
    std::thread *mcThread;
};
