#include "LegacyMainMenu.h"

#include "LegacyGuiButton.h"
#include "LegacyMainMenuLayout.h"
#include "net/minecraft/src/GuiButton.h"
#include "net/minecraft/src/StringTranslate.h"

void legacyCreateMainMenuButtons(std::vector<GuiButton *> &controlList, GuiButton *&multiplayerButton,
    int_t screenWidth, int_t screenHeight, bool hideQuitButton)
{
    const int_t buttonCount = legacyMainMenuButtonCount(hideQuitButton);
    const LegacyMainMenuLayout layout = legacyMainMenuLayout(screenWidth, screenHeight, buttonCount);
    const int_t stride = layout.buttonHeight + layout.buttonSpacing;
    int_t row = 0;

    auto addButton = [&](int_t id, const std::string &label) -> GuiButton *
    {
        GuiButton *button = new LegacyGuiButton(id, layout.buttonX, layout.firstButtonY + row * stride,
            layout.buttonWidth, layout.buttonHeight, label);
        controlList.push_back(button);
        ++row;
        return button;
    };

    StringTranslate *tr = StringTranslate::getInstance();
    addButton(1, "Play Game");
    multiplayerButton = addButton(2, tr->translateKey("menu.multiplayer"));
    addButton(3, tr->translateKey("menu.mods"));
    addButton(0, "Help & Options");
    addButton(5, "Credits");
    if (!hideQuitButton)
        addButton(4, tr->translateKey("menu.quit"));
}
