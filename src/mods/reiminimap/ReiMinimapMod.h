#pragma once

#include "mods/IMod.h"

class ReiMinimapMod : public IMod
{
public:
    ReiMinimapMod();
    ~ReiMinimapMod() override = default;

    std::string getId() const override { return "reiminimap"; }
    std::string getName() const override { return "Rei's Minimap"; }
    std::string getVersion() const override { return "v3.2_05"; }
    std::string getDescription() const override { return "Minimap radar, coordinates & waypoints. Set waypoint: Triangle + Up."; }
    std::string getAuthor() const override { return "ReiFNSK"; }

    bool isEnabled() const override;
    void setEnabled(bool state) override;

    void onInit(Minecraft *mc) override;
    void onTick() override;
    void onRenderGameOverlay(GuiIngame *gui, int_t screenWidth, int_t screenHeight, float_t partialTick) override;

private:
    bool m_enabled;
};
