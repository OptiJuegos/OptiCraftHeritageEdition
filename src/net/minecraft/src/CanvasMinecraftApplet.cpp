#include "CanvasMinecraftApplet.h"

#include "Minecraft.h"

CanvasMinecraftApplet::CanvasMinecraftApplet(Minecraft *minecraftapplet)
    : mcApplet(minecraftapplet)
{
}

void CanvasMinecraftApplet::addNotify()
{
    // Java: super.addNotify(); mcApplet.startMainThread();
    // In C++ the main thread is started explicitly by the caller;
    // mcApplet->startGame() is the nearest equivalent.
    if (mcApplet != nullptr)
        mcApplet->startGame();
}

void CanvasMinecraftApplet::removeNotify()
{
    // Java: mcApplet.shutdown(); super.removeNotify();
    if (mcApplet != nullptr)
        mcApplet->shutdown();
}
