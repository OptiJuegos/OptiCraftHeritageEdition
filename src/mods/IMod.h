#pragma once

#include "java/Type.h"
#include <string>

class Minecraft;
class GuiContainer;
class GuiIngame;

// Base interface for all OptiCraft mods
class IMod
{
public:
    virtual ~IMod() = default;

    // Unique identifier for configuration persistence (e.g. "toomanyitems")
    virtual std::string getId() const = 0;

    // Mod display info in the Mods menu
    virtual std::string getName() const = 0;
    virtual std::string getVersion() const = 0;
    virtual std::string getDescription() const = 0;
    virtual std::string getAuthor() const = 0;

    // Toggle state
    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;

    // Package path & removability
    virtual std::string getPackPath() const { return packPath; }
    virtual void setPackPath(const std::string &path) { packPath = path; }
    virtual bool isRemovable() const { return !packPath.empty(); }

    // Mod lifecycle
    virtual void onInit(Minecraft *mc) {}
    virtual void onTick() {}

    // In-game HUD overlay hook (rendered while playing)
    virtual void onRenderGameOverlay(GuiIngame *gui, int_t screenWidth, int_t screenHeight, float_t partialTick) {}

    // GUI Container hooks (inventory, chests, crafting tables, etc.)
    virtual void onDrawContainer(GuiContainer *container, int_t mouseX, int_t mouseY) {}
    virtual bool onContainerMouseClicked(GuiContainer *container, int_t x, int_t y, int_t button) { return false; }
    virtual bool onContainerKeyTyped(char_t c, int_t key) { return false; }

protected:
    std::string packPath;
};
