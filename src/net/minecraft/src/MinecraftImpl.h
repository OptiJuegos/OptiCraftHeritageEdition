#pragma once

#include "Minecraft.h"

class UnexpectedThrowable;

// net.minecraft.src.MinecraftImpl
// Standalone (non-applet) Minecraft instance.
// Java: extends Minecraft, holds a reference to the AWT Frame so that
// the crash report panel can be injected when a fatal error occurs.
// C++: holds an SDL_Window* (as void*) to represent the game window.
class MinecraftImpl : public Minecraft
{
public:
    MinecraftImpl(void *component, void *canvas, void *minecraftapplet,
                  int_t width, int_t height, bool fullscreen, void *frame);

    void displayUnexpectedThrowable(UnexpectedThrowable *unexpectedthrowable) override;

private:
    void *mcFrame;
};
