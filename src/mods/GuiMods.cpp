#include "GuiMods.h"
#include "GuiSlot.h"
#include "GuiButton.h"
#include "Minecraft.h"
#include "FontRenderer.h"
#include "SoundManager.h"
#include "StringTranslate.h"
#include "GuiTexturePacks.h"
#include "mods/ModManager.h"

#if PLATFORM_PS2 || PLATFORM_WII
#include "platform/Input.h"
#endif

class GuiSlotMods : public GuiSlot
{
public:
    GuiSlotMods(GuiMods *parentScreen)
        : GuiSlot(parentScreen->mc, parentScreen->width, parentScreen->height, 38, parentScreen->height - 42, 36)
        , parent(parentScreen)
    {
    }

    int_t getSize() override
    {
        return static_cast<int_t>(ModManager::getInstance().getMods().size());
    }

    void elementClicked(int_t index, bool doubleClicked) override
    {
        auto &mods = ModManager::getInstance().getMods();
        if (index >= 0 && index < static_cast<int_t>(mods.size()))
        {
            bool newState = !mods[index]->isEnabled();
            mods[index]->setEnabled(newState);
            ModManager::getInstance().save();

            if (parent->mc != nullptr && parent->mc->sndManager != nullptr)
                parent->mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
        }
    }

    bool isSelected(int_t index) override
    {
        return false;
    }

    int_t getContentHeight() override
    {
        return getSize() * 36;
    }

    void drawBackground() override
    {
        parent->drawDefaultBackground();
    }

    void drawSlot(int_t index, int_t x, int_t y, int_t height, Tessellator *tess) override
    {
        auto &mods = ModManager::getInstance().getMods();
        if (index < 0 || index >= static_cast<int_t>(mods.size()))
            return;

        const auto &mod = mods[index];
        FontRenderer *fr = parent->getFont();
        if (fr == nullptr)
            return;

        // Mod Title & Version
        std::string title = mod->getName() + " " + std::string("\xc2\xa7") + "7" + mod->getVersion();
        parent->drawString(fr, title, x + 4, y + 4, 0xFFFFFF);

        // Mod Description & Author
        std::string desc = mod->getDescription();
        if (!mod->getAuthor().empty())
            desc += " " + std::string("\xc2\xa7") + "8(" + mod->getAuthor() + ")";
        parent->drawString(fr, desc, x + 4, y + 18, 0x888888);

        // Toggle button on right side of slot
        int_t btnW = 76;
        int_t btnH = 18;
        int_t btnX = x + 216 - btnW;
        int_t btnY = y + 6;

        bool enabled = mod->isEnabled();
        int_t bgColor = enabled ? 0x80104510 : 0x80451010;
        int_t borderColor = enabled ? 0xFF28A828 : 0xFFA82828;

        parent->drawRect(btnX, btnY, btnX + btnW, btnY + btnH, bgColor);
        parent->drawRect(btnX, btnY, btnX + btnW, btnY + 1, borderColor);
        parent->drawRect(btnX, btnY + btnH - 1, btnX + btnW, btnY + btnH, borderColor);
        parent->drawRect(btnX, btnY, btnX + 1, btnY + btnH, borderColor);
        parent->drawRect(btnX + btnW - 1, btnY, btnX + btnW, btnY + btnH, borderColor);

        std::string statusText = enabled ? (std::string("\xc2\xa7") + "a[ ENABLED ]") : (std::string("\xc2\xa7") + "c[ DISABLED ]");
        parent->drawCenteredString(fr, statusText, btnX + btnW / 2, btnY + 5, 0xFFFFFF);
    }

private:
    GuiMods *parent;
};

GuiMods::GuiMods(GuiScreen *parent)
    : parentScreen(parent)
    , slotList(nullptr)
    , screenTitle("Mod Manager")
{
}

GuiMods::~GuiMods()
{
    delete slotList;
    slotList = nullptr;
}

void GuiMods::initGui()
{
    StringTranslate *tr = StringTranslate::getInstance();
    screenTitle = "Mod Manager";

    delete slotList;
    slotList = new GuiSlotMods(this);
    slotList->registerScrollButtons(controlList, 7, 8);

    // Bottom buttons
    controlList.push_back(new GuiButton(100, width / 2 - 155, height - 32, 150, 20, "Texture Packs"));
    controlList.push_back(new GuiButton(200, width / 2 + 5, height - 32, 150, 20, tr->translateKey("gui.done")));
}

void GuiMods::actionPerformed(GuiButton *button)
{
    if (!button->enabled)
        return;

    if (button->id == 200) // Done
    {
        ModManager::getInstance().save();
        mc->displayGuiScreen(parentScreen);
    }
    else if (button->id == 100) // Texture Packs
    {
        mc->displayGuiScreen(new GuiTexturePacks(this));
    }
    else if (slotList != nullptr)
    {
        slotList->actionPerformed(button);
    }
}

void GuiMods::keyTyped(char_t c, int_t key)
{
    if (key == 1) // ESC
    {
        ModManager::getInstance().save();
        mc->displayGuiScreen(parentScreen);
        return;
    }
    GuiScreen::keyTyped(c, key);
}

void GuiMods::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
#if PLATFORM_PS2 || PLATFORM_WII
    const PlatformTextInputSnapshot pad = platformTextInputSnapshot(platformMenuPad());
    if ((pad.pressed & (PLATFORM_TEXT_BACK | PLATFORM_TEXT_CLOSE)) != 0)
    {
        ModManager::getInstance().save();
        mc->displayGuiScreen(parentScreen);
        return;
    }
#endif

    if (slotList != nullptr)
        slotList->drawScreen(mouseX, mouseY, partialTick);

    drawCenteredString(fontRenderer, screenTitle, width / 2, 10, 0xFFFFFF);
    drawCenteredString(fontRenderer, std::string("\xc2\xa7") + "7Mods are experimental & subject to community development", width / 2, 23, 0x888888);

    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
