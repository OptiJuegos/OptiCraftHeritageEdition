#pragma once

#include "GuiScreen.h"

class GuiTextField;
class ServerNBTStorage;

// net.minecraft.src.GuiScreenServerList
class GuiScreenServerList : public GuiScreen
{
public:
    GuiScreenServerList(GuiScreen *parent, ServerNBTStorage *server);
    ~GuiScreenServerList() override;

    void updateScreen() override;
    void initGui() override;
    void onGuiClosed() override;
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
    void actionPerformed(GuiButton *button) override;
    void keyTyped(char_t c, int_t key) override;
    void mouseClicked(int_t x, int_t y, int_t button) override;

private:
    void updateSelectButtonState();

    static jstring lastAddress;
    GuiScreen *parentGui;
    ServerNBTStorage *serverListStorage;
    GuiTextField *serverTextField;
    GuiButton *buttonSelect;
};
