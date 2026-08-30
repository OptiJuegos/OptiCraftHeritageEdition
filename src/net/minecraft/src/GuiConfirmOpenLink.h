#pragma once

#include "GuiYesNo.h"
#include <string>

// net.minecraft.src.GuiConfirmOpenLink
class GuiConfirmOpenLink : public GuiYesNo
{
public:
    GuiConfirmOpenLink(GuiScreen *parent, const std::string &link, int_t id);
    void initGui() override;
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
    void actionPerformed(GuiButton *button) override;
    virtual void copyLinkToClipboard() = 0;

private:
    std::string warningText;
    std::string copyText;
};
