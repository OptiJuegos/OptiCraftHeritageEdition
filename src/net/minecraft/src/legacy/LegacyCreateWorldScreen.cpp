#include "LegacyCreateWorldScreen.h"

#include <algorithm>

#include "LegacyDifficultySlider.h"
#include "LegacyGuiButton.h"
#include "LegacyMainMenuLayout.h"
#include "LegacyOptionLabel.h"
#include "LegacyOptionsLayout.h"
#include "LegacyMenuHints.h"
#include "LegacyPanorama.h"
#include "LegacySceneState.h"
#include "LegacyUiAssets.h"
#include "net/minecraft/src/EnumOptions.h"
#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/FontRenderer.h"
#include "net/minecraft/src/GuiButton.h"
#include "net/minecraft/src/GuiTextField.h"
#include "net/minecraft/src/Minecraft.h"
#include "net/minecraft/src/RenderEngine.h"
#include "net/minecraft/src/SoundManager.h"
#include "net/minecraft/src/StringTranslate.h"
#include "net/minecraft/src/WorldType.h"
#include "pc/lwjgl/Keyboard.h"
#include "platform/Input.h"
#include "platform/PlatformConfig.h"

namespace
{
enum LegacyCreateButtonId
{
    BUTTON_CREATE_WORLD = 0,
    BUTTON_GAME_MODE = 2,
    BUTTON_MORE_OPTIONS = 3,
    BUTTON_GENERATE_STRUCTURES = 4,
    BUTTON_WORLD_TYPE = 5,
    BUTTON_DIFFICULTY = 6,
    BUTTON_WORLD_SIZE = 7
};

constexpr int_t CREATE_SELECTION_COUNT = 6;
bool pointInside(int_t x, int_t y, int_t left, int_t top, int_t width, int_t height)
{
    return x >= left && y >= top && x < left + width && y < top + height;
}

void drawPanelTitle(FontRenderer *font, const std::string &text, int_t centerX, int_t y)
{
    if (font == nullptr)
        return;

    const int_t x = centerX - font->getStringWidth(text) / 2;
    font->drawString(text, x + 1, y + 1, 0xd0d0d0);
    font->drawString(text, x, y, 0x303030);
}
}

LegacyCreateWorldScreen::LegacyCreateWorldScreen(GuiScreen *parent)
    : GuiCreateWorld(parent), difficultySlider(nullptr), legacyWorldSizeButton(nullptr),
      textFieldHeight(0), labelOffsetY(0), selectedControlIndex(0), hoveredControlIndex(-1),
      panoramaAvailable(false)
{
}

void LegacyCreateWorldScreen::configureLayout()
{
    layout = legacyOptionsLayout(width, height, CREATE_SELECTION_COUNT, LegacyOptionsLayoutPreset::Form);
    textFieldHeight = layout.rowHeight;
    labelOffsetY = std::max<int_t>(1, layout.rowHeight / 2);
}

void LegacyCreateWorldScreen::initGui()
{
    StringTranslate *tr = StringTranslate::getInstance();
    WorldType::initialize();
    lwjgl::Keyboard::enableRepeatEvents(true);
    configureLayout();
    hoveredControlIndex = -1;
    panoramaAvailable = mc != nullptr && mc->renderEngine != nullptr &&
        mc->renderEngine->hasResource(legacyPanoramaResourcePath());

    controlList.push_back(new LegacyGuiButton(BUTTON_CREATE_WORLD, layout.contentX, layout.rowY(5),
        layout.contentWidth, layout.rowHeight, tr->translateKey("selectWorld.create")));
    controlList.push_back(gameModeButton = new LegacyGuiButton(BUTTON_GAME_MODE, layout.contentX, layout.rowY(1),
        layout.contentWidth, layout.rowHeight, tr->translateKey("selectWorld.gameMode")));
    controlList.push_back(difficultySlider = new LegacyDifficultySlider(BUTTON_DIFFICULTY,
        layout.contentX, layout.rowY(2), layout.contentWidth, layout.rowHeight, mc->gameSettings));
    controlList.push_back(legacyWorldSizeButton = new LegacyGuiButton(BUTTON_WORLD_SIZE, layout.contentX,
        layout.rowY(3), layout.contentWidth, layout.rowHeight, ""));
    worldSizeButton = legacyWorldSizeButton;
    controlList.push_back(moreWorldOptionsButton = new LegacyGuiButton(BUTTON_MORE_OPTIONS, layout.contentX,
        layout.rowY(4), layout.contentWidth, layout.rowHeight, tr->translateKey("selectWorld.moreWorldOptions")));
    controlList.push_back(generateStructuresButton = new LegacyGuiButton(BUTTON_GENERATE_STRUCTURES,
        layout.contentX, layout.rowY(1), layout.contentWidth, layout.rowHeight,
        tr->translateKey("selectWorld.mapFeatures")));
    controlList.push_back(worldTypeButton = new LegacyGuiButton(BUTTON_WORLD_TYPE, layout.contentX,
        layout.rowY(2), layout.contentWidth, layout.rowHeight, tr->translateKey("selectWorld.mapType")));

    delete textboxWorldName;
    textboxWorldName = new GuiTextField(fontRenderer, layout.contentX, layout.rowY(0), layout.contentWidth, textFieldHeight);
    textboxWorldName->setText(localizedNewWorldText);

    delete textboxSeed;
    textboxSeed = new GuiTextField(fontRenderer, layout.contentX, layout.rowY(0), layout.contentWidth, textFieldHeight);
    textboxSeed->setText(seed);

    updateFolderName();
    updateButtonText();
    updateDifficultyControl();
    updateControlVisibility();
    selectControl(0);
#if PLATFORM_PS2 || PLATFORM_WII
    // Preserve the console's existing initial text-entry behavior. Once the user
    // closes the keyboard, focus stays off until row 0 is activated again.
    textboxWorldName->setFocused(true);
#endif
}

void LegacyCreateWorldScreen::updateControlVisibility()
{
    gameModeButton->enabled2 = !moreOptions;
    difficultySlider->enabled2 = !moreOptions;
    generateStructuresButton->enabled2 = moreOptions;
    worldTypeButton->enabled2 = moreOptions;
    if (worldSizeButton != nullptr)
        worldSizeButton->enabled2 = true;
}

void LegacyCreateWorldScreen::updateDifficultyControl()
{
    if (difficultySlider == nullptr || mc == nullptr || mc->gameSettings == nullptr)
        return;

    // The slider writes its own label every frame, so hardcore is a state it is
    // told about rather than a string pushed into it.
    difficultySlider->enabled = !hardcore;
    difficultySlider->setHardcore(hardcore);
}

GuiButton *LegacyCreateWorldScreen::buttonForSelection(int_t index) const
{
    switch (index)
    {
    case 1: return moreOptions ? generateStructuresButton : gameModeButton;
    case 2: return moreOptions ? worldTypeButton : static_cast<GuiButton *>(difficultySlider);
    case 3: return worldSizeButton;
    case 4: return moreWorldOptionsButton;
    case 5: return !controlList.empty() ? controlList[0] : nullptr;
    default: return nullptr;
    }
}

int_t LegacyCreateWorldScreen::selectionForButton(const GuiButton *button) const
{
    if (button == nullptr)
        return -1;
    if (button == gameModeButton || button == generateStructuresButton)
        return 1;
    if (button == difficultySlider || button == worldTypeButton)
        return 2;
    if (button == worldSizeButton)
        return 3;
    if (button == moreWorldOptionsButton)
        return 4;
    if (!controlList.empty() && button == controlList[0])
        return 5;
    return -1;
}

bool LegacyCreateWorldScreen::isSelectionAvailable(int_t index) const
{
    if (index == 0)
        return true;
    GuiButton *button = buttonForSelection(index);
    return button != nullptr && button->enabled && button->enabled2;
}

void LegacyCreateWorldScreen::syncSelectedControl()
{
    // setKeyboardSelected is the GuiButton virtual every Legacy control implements.
    // The rows are no longer all LegacyGuiButton, so casting to it would be reading
    // the difficulty slider as the wrong type.
    for (GuiButton *button : controlList)
    {
        if (button != nullptr)
            button->setKeyboardSelected(false);
    }

    // Selecting the text row is not the same as editing it. The virtual keyboard
    // closes by clearing field focus; do not immediately re-focus it on the next
    // selection sync or D-pad navigation becomes trapped on row 0.
    if (selectedControlIndex != 0 || hoveredControlIndex >= 0)
    {
        textboxWorldName->setFocused(false);
        textboxSeed->setFocused(false);
    }

    GuiButton *selected = buttonForSelection(selectedControlIndex);
    if (hoveredControlIndex < 0 && selected != nullptr && selected->enabled2)
        selected->setKeyboardSelected(true);
}

void LegacyCreateWorldScreen::updatePointerHover(int_t mouseX, int_t mouseY)
{
    int_t hover = -1;
#if PLATFORM_PS2
    // Legacy PS2 menus deliberately suppress the software mouse pointer. Treating
    // its stale coordinates as a live hover leaves D-pad navigation blocked after
    // the virtual keyboard closes.
    (void)mouseX;
    (void)mouseY;
#elif PLATFORM_WII
    if (platformMenuPointerActive())
#endif
    {
#if !PLATFORM_PS2
        GuiTextField *field = moreOptions ? textboxSeed : textboxWorldName;
        if (field != nullptr && pointInside(mouseX, mouseY, layout.contentX, layout.rowY(0), layout.contentWidth, textFieldHeight))
        {
            hover = 0;
        }
        else
        {
            for (GuiButton *button : controlList)
            {
                if (button == nullptr || !button->enabled || !button->enabled2)
                    continue;
                if (pointInside(mouseX, mouseY, button->xPosition, button->yPosition,
                    button->getButtonWidth(), button->getButtonHeight()))
                {
                    hover = selectionForButton(button);
                    break;
                }
            }
        }
#endif
    }

    if (hoveredControlIndex != hover)
    {
        hoveredControlIndex = hover;
        if (hoveredControlIndex >= 0)
            selectedControlIndex = hoveredControlIndex;
        syncSelectedControl();
    }
}

bool LegacyCreateWorldScreen::adjustSelection(int_t direction)
{
    if (hoveredControlIndex >= 0)
        return false;
    if (selectedControlIndex <= 0)
        return false;
    if (selectedControlIndex == 3 && worldSizeButton != nullptr)
    {
        limitedWorld = !limitedWorld;
        updateButtonText();
        mc->sndManager->playSoundFX("random.focus", 1.0f, 1.0f);
        return true;
    }
    GuiButton *button = buttonForSelection(selectedControlIndex);
    if (button == nullptr || !button->enabled || !button->enabled2)
        return false;
    if (!button->adjustKeyboard(mc, direction))
        return false;
    mc->gameSettings->saveOptions();
    mc->sndManager->playSoundFX("random.focus", 1.0f, 1.0f);
    return true;
}

void LegacyCreateWorldScreen::activateSelection()
{
    const int_t targetIndex = hoveredControlIndex >= 0 ? hoveredControlIndex : selectedControlIndex;
    if (targetIndex == 0)
    {
        GuiTextField *field = moreOptions ? textboxSeed : textboxWorldName;
        if (field != nullptr)
            field->setFocused(true);
        return;
    }
    if (targetIndex < 0)
        return;
    GuiButton *button = buttonForSelection(targetIndex);
    if (button == nullptr || !button->enabled || !button->enabled2)
        return;
    mc->sndManager->playSoundFX("random.action", 1.0f, 1.0f);
    actionPerformed(button);
}

void LegacyCreateWorldScreen::selectControl(int_t index)
{
    const int_t clamped = std::max<int_t>(0, std::min<int_t>(index, CREATE_SELECTION_COUNT - 1));
    if (!isSelectionAvailable(clamped))
        return;
    selectedControlIndex = clamped;
    syncSelectedControl();
}

void LegacyCreateWorldScreen::moveSelection(int_t direction)
{
    if (hoveredControlIndex >= 0)
        return;
    const int_t previous = selectedControlIndex;
    int_t candidate = selectedControlIndex + direction;
    while (candidate >= 0 && candidate < CREATE_SELECTION_COUNT)
    {
        if (isSelectionAvailable(candidate))
        {
            selectControl(candidate);
            if (selectedControlIndex != previous)
                mc->sndManager->playSoundFX("random.focus", 1.0f, 1.0f);
            return;
        }
        candidate += direction;
    }
}

void LegacyCreateWorldScreen::updateScreen()
{
    GuiCreateWorld::updateScreen();
#if PLATFORM_PS2 || PLATFORM_WII
    // The virtual keyboard owns the console text-input snapshot while a field is
    // focused. Do not let menu navigation consume the same presses underneath it.
    if (platformTextInputExclusive())
        return;
    const PlatformTextInputSnapshot pad = platformTextInputSnapshot(platformMenuPad());
#if PLATFORM_PS2
    if ((pad.pressed & (PLATFORM_TEXT_CLOSE | PLATFORM_TEXT_SHIFT)) != 0)
    {
        if (mc->sndManager != nullptr)
            mc->sndManager->playSoundFX("random.back", 1.0f, 1.0f);
        mc->displayGuiScreen(parentScreen);
        return;
    }
#endif
    if ((pad.pressed & PLATFORM_TEXT_UP) != 0)
        moveSelection(-1);
    else if ((pad.pressed & PLATFORM_TEXT_DOWN) != 0)
        moveSelection(1);
    else if ((pad.pressed & PLATFORM_TEXT_LEFT) != 0)
        adjustSelection(-1);
    else if ((pad.pressed & PLATFORM_TEXT_RIGHT) != 0)
        adjustSelection(1);
#if PLATFORM_PS2
    if ((pad.pressed & PLATFORM_TEXT_TYPE) != 0)
        activateSelection();
#elif PLATFORM_WII
    if (!platformMenuPointerActive() && (pad.pressed & PLATFORM_TEXT_TYPE) != 0)
        activateSelection();
#endif
#if PLATFORM_WII
    if ((pad.pressed & PLATFORM_TEXT_BACK) != 0)
    {
        mc->sndManager->playSoundFX("random.back", 1.0f, 1.0f);
        mc->displayGuiScreen(parentScreen);
    }
#endif
#endif
}

void LegacyCreateWorldScreen::actionPerformed(GuiButton *button)
{
    if (button == nullptr || !button->enabled)
        return;

    const int_t selection = selectionForButton(button);
    if (selection >= 0)
        selectControl(selection);

    // The slider is driven with left and right, but activating it still steps to the
    // next difficulty. That keeps one button working on a pad, the way it did when
    // this row was a cycling button.
    if (button->id == BUTTON_DIFFICULTY)
    {
        if (mc != nullptr && mc->gameSettings != nullptr)
        {
            mc->gameSettings->setOptionValue(EnumOptions::DIFFICULTY, 1);
            mc->gameSettings->saveOptions();
            difficultySlider->refreshFromSettings();
        }
        return;
    }

    const int_t id = button->id;
    GuiCreateWorld::actionPerformed(button);
    if (id == BUTTON_MORE_OPTIONS)
    {
        updateControlVisibility();
        selectControl(4);
    }
    else if (id == BUTTON_WORLD_SIZE)
    {
        updateButtonText();
    }
    else if (id == BUTTON_GAME_MODE)
    {
        updateDifficultyControl();
        if (!isSelectionAvailable(selectedControlIndex))
            moveSelection(1);
    }
}

void LegacyCreateWorldScreen::keyTyped(char_t c, int_t key)
{
    if (key == 1)
    {
        GuiTextField *field = moreOptions ? textboxSeed : textboxWorldName;
        if (field != nullptr && field->getFocused())
        {
            field->setFocused(false);
            hoveredControlIndex = -1;
            syncSelectedControl();
            return;
        }
        mc->sndManager->playSoundFX("random.back", 1.0f, 1.0f);
        mc->displayGuiScreen(parentScreen);
        return;
    }
#if !PLATFORM_PS2 && !PLATFORM_WII
    if (key == 200)
    {
        moveSelection(-1);
        return;
    }
    if (key == 208 || key == 15)
    {
        moveSelection(1);
        return;
    }
    // Left and right drive the difficulty slider. The name field keeps them for its
    // caret, so they only reach a control once the selection has left the field.
    if ((key == 203 || key == 205) && selectedControlIndex != 0)
    {
        adjustSelection(key == 203 ? -1 : 1);
        return;
    }
    if (key == 28 || c == '\r')
    {
        if (selectedControlIndex == 0)
            moveSelection(1);
        else
            activateSelection();
        return;
    }
#endif

    if (selectedControlIndex == 0)
    {
        if (!moreOptions)
        {
            textboxWorldName->textboxKeyTyped(c, key);
            localizedNewWorldText = textboxWorldName->getText();
        }
        else
        {
            textboxSeed->textboxKeyTyped(c, key);
            seed = textboxSeed->getText();
        }
        controlList[0]->enabled = !textboxWorldName->getText().empty();
        updateFolderName();
    }
}

void LegacyCreateWorldScreen::mouseClicked(int_t x, int_t y, int_t button)
{
    GuiCreateWorld::mouseClicked(x, y, button);
    if (button != 0)
        return;

    GuiTextField *field = moreOptions ? textboxSeed : textboxWorldName;
    if (pointInside(x, y, layout.contentX, layout.rowY(0), layout.contentWidth, textFieldHeight))
        selectControl(0);
    else if (field->getFocused())
        field->setFocused(false);
}

void LegacyCreateWorldScreen::drawLegacyScene(float_t partialTick)
{
    const bool drewPanorama = panoramaAvailable &&
        legacyDrawPanorama(mc, width, height, legacyScenePanoramaTimer(), partialTick, zLevel);
    if (!drewPanorama)
        drawDefaultBackground();
    else
        drawGradientRect(0, 0, width, height, static_cast<int_t>(0x14000000u), static_cast<int_t>(0x3c000000u));

    LegacyMainMenuLayout titleLayout{};
    titleLayout.titleY = layout.titleY;
    titleLayout.titleMaxWidth = layout.titleMaxWidth;
    titleLayout.titleMaxHeight = layout.titleMaxHeight;
    legacyDrawTitleTexture(mc, titleLayout, width, zLevel, nullptr);
    panelRenderer.draw(layout);
}

void LegacyCreateWorldScreen::drawMenuControlHints()
{
    drawLegacyMenuHints(mc, width, height, true);
}

void LegacyCreateWorldScreen::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    StringTranslate *tr = StringTranslate::getInstance();
    drawLegacyScene(partialTick);
    const std::string title = tr->translateKey(moreOptions ? "selectWorld.moreWorldOptions" : "selectWorld.create");
    drawPanelTitle(fontRenderer, title, width / 2, layout.panelY + 8);

    if (!moreOptions)
    {
        legacyDrawOptionLabel(fontRenderer, tr->translateKey("selectWorld.enterName"),
            layout.contentX, layout.rowY(0) - labelOffsetY);
        textboxWorldName->drawTextBox();
    }
    else
    {
        legacyDrawOptionLabel(fontRenderer, tr->translateKey("selectWorld.enterSeed"),
            layout.contentX, layout.rowY(0) - labelOffsetY);
        textboxSeed->drawTextBox();
    }

    updatePointerHover(mouseX, mouseY);
    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
    drawMenuControlHints();
}
