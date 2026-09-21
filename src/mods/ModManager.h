#pragma once

#include "java/Type.h"
#include "IMod.h"
#include <vector>
#include <string>
#include <memory>

class Minecraft;
class GuiContainer;
class GuiIngame;

class ModManager
{
public:
    static ModManager &getInstance();

    void init(Minecraft *mc);
    void shutdown();

    void registerMod(std::unique_ptr<IMod> mod);
    const std::vector<std::unique_ptr<IMod>> &getMods() const { return mods; }
    std::vector<std::unique_ptr<IMod>> &getMods() { return mods; }

    IMod *getMod(const std::string &id);
    bool isModEnabled(const std::string &id);
    void setModEnabled(const std::string &id, bool enabled);

    // Persistence: saves/loads enabled states to/from mods.txt
    void load();
    void save();

    // Dispatched events to all enabled mods
    void onTick();
    void onRenderGameOverlay(GuiIngame *gui, int_t screenWidth, int_t screenHeight, float_t partialTick);
    void onDrawContainer(GuiContainer *container, int_t mouseX, int_t mouseY);
    bool onContainerMouseClicked(GuiContainer *container, int_t x, int_t y, int_t button);
    bool onContainerKeyTyped(char_t c, int_t key);

private:
    ModManager();
    ~ModManager();
    ModManager(const ModManager &) = delete;
    ModManager &operator=(const ModManager &) = delete;

    std::string getConfigPath() const;

    Minecraft *mc;
    std::vector<std::unique_ptr<IMod>> mods;
    bool initialized;
};
