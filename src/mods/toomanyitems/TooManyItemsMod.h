#pragma once

#include "mods/IMod.h"

class TooManyItemsMod : public IMod
{
public:
    TooManyItemsMod();
    ~TooManyItemsMod() override = default;

    std::string getId() const override { return "toomanyitems"; }
    std::string getName() const override { return "TooManyItems"; }
    std::string getVersion() const override { return "v1.2.5"; }
    std::string getDescription() const override { return "In-game inventory editor and cheat panel"; }
    std::string getAuthor() const override { return "Marglyph"; }

    bool isEnabled() const override { return m_enabled; }
    void setEnabled(bool state) override { m_enabled = state; }

    void onInit(Minecraft *mc) override;
    void onDrawContainer(GuiContainer *container, int_t mouseX, int_t mouseY) override;
    bool onContainerMouseClicked(GuiContainer *container, int_t x, int_t y, int_t button) override;
    bool onContainerKeyTyped(char_t c, int_t key) override;

private:
    bool m_enabled;
};
