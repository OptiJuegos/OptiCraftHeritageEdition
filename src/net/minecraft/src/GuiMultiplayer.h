#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "GuiScreen.h"

class GuiButton;
class GuiSlotServer;
class Minecraft;
class FontRenderer;
class ServerNBTStorage;

// net.minecraft.src.GuiMultiplayer
class GuiMultiplayer : public GuiScreen
{
public:
    explicit GuiMultiplayer(GuiScreen *parent);
    ~GuiMultiplayer() override;

    void updateScreen() override;
    void initGui() override;
    void onGuiClosed() override;
    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;
    void confirmClicked(bool confirmed, int_t id) override;
    bool suppressesPlatformPointerInput() const override;

    void initGuiControls();
    void joinServer(int_t index);
    void joinServer(const std::shared_ptr<ServerNBTStorage> &server);

    const std::vector<std::shared_ptr<ServerNBTStorage>> &getServerList() const;
    int_t getSelectedServer() const;
    void setSelectedServer(int_t index);
    GuiButton *getButtonSelect() const;
    GuiButton *getButtonEdit() const;
    GuiButton *getButtonDelete() const;
    void setTooltipText(const std::string &text);
    Minecraft *getMinecraft() const { return mc; }
    FontRenderer *getFontRenderer() const { return fontRenderer; }

    static int_t getThreadsPending();
    static void incrementThreadsPending();
    static void decrementThreadsPending();
    static void pollServer(const std::shared_ptr<ServerNBTStorage> &server);

protected:
    void actionPerformed(GuiButton *button) override;
    void keyTyped(char_t c, int_t key) override;
    void mouseClicked(int_t x, int_t y, int_t button) override;
    bool usesSpecializedMenuNavigation() const override;
    void handleSpecializedMenuInput() override;

private:
    void loadServerList();
    void saveServerList();
    void updateSelectionButtons();
    void syncControllerFocus();
    void focusButton(int_t buttonId);
    void moveControllerFocusHorizontal(int_t direction);
    void moveControllerFocusVertical(int_t direction);
    void activateControllerFocus();
    GuiButton *findButton(int_t buttonId) const;
    int_t parseIntWithDefault(const std::string &value, int_t defaultValue) const;
    void drawTooltip(const std::string &text, int_t mouseX, int_t mouseY);
    static void splitServerAddress(const std::string &address, std::string &host, int_t &port);

    static std::atomic<int_t> threadsPending;

    GuiScreen *parentScreen;
    GuiSlotServer *serverSlotContainer;
    std::vector<std::shared_ptr<ServerNBTStorage>> serverList;
    int_t selectedServer;
    GuiButton *buttonEdit;
    GuiButton *buttonSelect;
    GuiButton *buttonDelete;
    bool deleteClicked;
    bool addClicked;
    bool editClicked;
    bool directClicked;
    std::string lagTooltip;
    std::shared_ptr<ServerNBTStorage> tempServer;
    int_t controllerFocus;
};
