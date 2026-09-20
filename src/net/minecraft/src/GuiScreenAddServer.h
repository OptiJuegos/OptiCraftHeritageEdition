#pragma once

#include "GuiScreen.h"

class GuiTextField;
class ServerNBTStorage;

// net.minecraft.src.GuiScreenAddServer
class GuiScreenAddServer : public GuiScreen
{
public:
    GuiScreenAddServer(GuiScreen *parent, ServerNBTStorage *server);
    ~GuiScreenAddServer() override;

    void updateScreen() override;
    void initGui() override;
    void onGuiClosed() override;
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
    void actionPerformed(GuiButton *button) override;
    void keyTyped(char_t c, int_t key) override;
    void mouseClicked(int_t x, int_t y, int_t button) override;

private:
    void updateAddButtonState();

    GuiScreen *parentGui;
    GuiTextField *serverAddress;
    GuiTextField *serverName;
    GuiButton *buttonAdd;
    ServerNBTStorage *serverNBTStorage;
};
