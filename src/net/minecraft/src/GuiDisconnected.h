#pragma once

#include "GuiScreen.h"
#include <string>
#include <vector>

// net.minecraft.src.GuiDisconnected
class GuiDisconnected : public GuiScreen
{
public:
    GuiDisconnected(const std::string &messageKey, const std::string &detailKey,
                    const std::vector<std::string> &args = {});

    void updateScreen() override;
    void initGui() override;
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
    void keyTyped(char_t c, int_t key) override;
    void actionPerformed(GuiButton *button) override;

private:
    std::string errorMessage;
    std::string errorDetail;
};
