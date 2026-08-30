#pragma once

// net.minecraft.src.CanvasMinecraftApplet
// Java: extends Canvas, calls mcApplet.startMainThread() when added to a
// window (addNotify) and mcApplet.shutdown() when removed (removeNotify).
// C++: thin wrapper around the Minecraft pointer; call startGame / shutdown
// explicitly from the SDL main loop instead of relying on AWT callbacks.
class Minecraft;

class CanvasMinecraftApplet
{
public:
    explicit CanvasMinecraftApplet(Minecraft *minecraftapplet);

    // Called when the canvas is attached to a window (Java: addNotify).
    void addNotify();

    // Called when the canvas is detached from a window (Java: removeNotify).
    void removeNotify();

    Minecraft *mcApplet;
};
