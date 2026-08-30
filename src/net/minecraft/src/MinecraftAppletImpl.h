#pragma once

#include "Minecraft.h"

class UnexpectedThrowable;

// net.minecraft.src.MinecraftAppletImpl
// Applet-based Minecraft instance.
// Java: extends Minecraft, holds the MinecraftApplet so that the crash
// panel can be injected into the applet container on fatal error.
// C++: the applet container is stored as void* (same as base class mcApplet).
class MinecraftAppletImpl : public Minecraft
{
public:
    MinecraftAppletImpl(void *minecraftAppletFrame, void *component, void *canvas,
                        void *minecraftapplet, int_t width, int_t height, bool fullscreen);

    void displayUnexpectedThrowable(UnexpectedThrowable *unexpectedthrowable) override;

private:
    void *mainFrame;
};
