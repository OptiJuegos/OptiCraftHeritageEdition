#include "net/minecraft/src/ControlIcon.h"
#include "net/minecraft/src/UiStrings.h"
#include "GuiSkinSelector.h"
#include "GuiLoadSkinsMenu.h"
#include "SkinManager.h"
#include "GuiButton.h"
#include "Minecraft.h"
#include "FontRenderer.h"
#include "GameSettings.h"
#include "StringTranslate.h"
#include "EntityPlayerSP.h"
#include "RenderEngine.h"
#include "SoundManager.h"
#include "Tessellator.h"
#include "platform/RenderAPI.h"
#include "platform/PlatformCompat.h"

#include "pc/lwjgl/Keyboard.h"

#if PLATFORM_PS2 || PLATFORM_WII
#include "platform/Input.h"
#endif

#ifdef PS2_PLATFORM
#include "ps2/input/Ps2PadState.h"
#include "ps2/input/Ps2PadKeyCodes.h"
#endif

#include <algorithm>
#include <cmath>
#include <string>

namespace
{
constexpr int_t BUTTON_ID_TAB_DEFAULT = 1;
constexpr int_t BUTTON_ID_TAB_CUSTOM = 2;
constexpr int_t BUTTON_ID_PREV_SKIN = 3;
constexpr int_t BUTTON_ID_NEXT_SKIN = 4;
constexpr int_t BUTTON_ID_PLAYER2 = 10;
constexpr int_t BUTTON_ID_LOAD_SKINS = 11;
constexpr int_t BUTTON_ID_DELETE_SKIN = 12;

std::string toUpperString(const std::string &str)
{
    std::string result = str;
    for (char &c : result)
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return result;
}
}

GuiSkinSelector::GuiSkinSelector(GuiScreen *parent, bool isPlayer2)
    : parentScreen(parent)
    , isPlayer2Skin(isPlayer2)
    , initializedSelection(false)
    , currentPackIndex(0)
    , currentSkinIndex(0)
    , scrollOffset(0.0f)
    , dialogLeft(0)
    , dialogTop(0)
    , dialogWidth(0)
    , dialogHeight(0)
    , leftPanelWidth(0)
    , rightPanelX(0)
    , rightPanelWidth(0)
    , carouselCenterX(0)
    , carouselGroundY(0)
    , nameplateY(0)
    , nameplateHeight(0)
    , buttonTabDefault(nullptr)
    , buttonTabCustom(nullptr)
    , buttonPlayer2Skin(nullptr)
    , buttonLoadSkins(nullptr)
    , buttonDeleteSkin(nullptr)
#if PLATFORM_PS2
    , ps2ActionReleaseLatch(true)
    , stickNavLatched(false)
    , dpadRepeatTimer(0)
    , stickRepeatTimer(0)
    , lastPadHeld(0)
#endif
{
#if PLATFORM_PS2
    lastPadHeld = ps2PadGetSnapshot(platformMenuPad()).held;
#endif
    SkinManager::init();
    currentPackIndex = SkinManager::getSelectedPackIndex();
    currentSkinIndex = SkinManager::getSelectedIndex();
}

void GuiSkinSelector::initGui()
{
    controlList.clear();

    StringTranslate *tr = StringTranslate::getInstance();
    const bool isEs = (tr != nullptr && tr->getCurrentLanguage().rfind("es_", 0) == 0);

    if (!initializedSelection)
    {
        if (isPlayer2Skin)
        {
            std::string p2Skin = "TennisSteve";
            if (mc != nullptr && mc->gameSettings != nullptr && !mc->gameSettings->selectedSkinP2.empty())
                p2Skin = mc->gameSettings->selectedSkinP2;
            else
                p2Skin = SkinManager::getSelectedSkinIdP2();

            const SkinEntry *entry = SkinManager::getSkinById(p2Skin);
            if (entry != nullptr && entry->isCustom)
            {
                currentPackIndex = 1;
                currentSkinIndex = SkinManager::getIndexById(p2Skin);
            }
            else
            {
                currentPackIndex = 0;
                currentSkinIndex = SkinManager::getIndexById(p2Skin);
            }
        }
        else
        {
            if (mc != nullptr && mc->gameSettings != nullptr && !mc->gameSettings->selectedSkin.empty())
            {
                SkinManager::setSelectedSkinId(mc->gameSettings->selectedSkin);
                currentPackIndex = SkinManager::getSelectedPackIndex();
                currentSkinIndex = SkinManager::getSelectedIndex();
            }
        }
        initializedSelection = true;
    }

    if (currentPackIndex >= SkinManager::getPackCount())
        currentPackIndex = 0;

    const int totalInPack = SkinManager::getSkinCountForPack(currentPackIndex);
    if (totalInPack > 0)
    {
        if (currentSkinIndex < 0 || currentSkinIndex >= totalInPack)
            currentSkinIndex = 0;
    }

    // Dialog layout sizing
    dialogWidth = std::min<int_t>(width - 16, 384);
    dialogHeight = std::min<int_t>(height - 48, 172);
    dialogLeft = (width - dialogWidth) / 2;
    dialogTop = 12;

    leftPanelWidth = dialogWidth * 30 / 100;
    rightPanelX = dialogLeft + leftPanelWidth + 4;
    rightPanelWidth = dialogWidth - leftPanelWidth - 4;

    carouselCenterX = rightPanelX + rightPanelWidth / 2;
    nameplateHeight = 20;
    nameplateY = dialogTop + dialogHeight - nameplateHeight - 8;
    carouselGroundY = nameplateY - 6;

    // Tabs in left panel (placed below pack cover art)
    const int_t packBtnX = dialogLeft + 6;
    const int_t packBtnW = leftPanelWidth - 12;
    const int_t tabY = dialogTop + 78;

    std::string defBase = isEs ? "Skins Originales" : "Default Skins";
    std::string defLabel = (currentPackIndex == 0) ? ("> " + defBase + " <") : defBase;
    buttonTabDefault = new GuiButton(BUTTON_ID_TAB_DEFAULT, packBtnX, tabY, packBtnW, 18, defLabel);
    controlList.push_back(buttonTabDefault);

    if (SkinManager::getPackCount() > 1)
    {
        std::string customBase = isEs ? "Personalizado" : "Custom";
        std::string customLabel = customBase + " (" + std::to_string(SkinManager::getSkinCountForPack(1)) + ")";
        if (currentPackIndex == 1)
            customLabel = "> " + customLabel + " <";
        buttonTabCustom = new GuiButton(BUTTON_ID_TAB_CUSTOM, packBtnX, tabY + 22, packBtnW, 18, customLabel);
        controlList.push_back(buttonTabCustom);
    }
    else
    {
        buttonTabCustom = nullptr;
    }

    // Bottom action buttons
    const int_t btnY = dialogTop + dialogHeight + 4;
    const int_t p2BtnWidth = 140;
    const int_t p2BtnHeight = 18;
    const int_t p2BtnX = dialogLeft + dialogWidth - p2BtnWidth;

    if (!isPlayer2Skin)
    {
        std::string p2BtnText = isEs ? "Skin 2do Jugador" : "Choose 2nd Player Skin";
        buttonPlayer2Skin = new GuiButton(BUTTON_ID_PLAYER2, p2BtnX, btnY, p2BtnWidth, p2BtnHeight, p2BtnText);
        buttonPlayer2Skin->enabled = true;
        controlList.push_back(buttonPlayer2Skin);
    }
    else
    {
        buttonPlayer2Skin = nullptr;
    }

    const int_t loadBtnWidth = 85;
    const int_t loadBtnX = (buttonPlayer2Skin != nullptr) ? (p2BtnX - loadBtnWidth - 5) : (dialogLeft + dialogWidth - loadBtnWidth);
    std::string loadBtnText = isEs ? "Cargar Skins" : "Load Skins";
    buttonLoadSkins = new GuiButton(BUTTON_ID_LOAD_SKINS, loadBtnX, btnY, loadBtnWidth, p2BtnHeight, loadBtnText);
    controlList.push_back(buttonLoadSkins);

    if (currentPackIndex == 1 && !SkinManager::getCustomSkins().empty())
    {
        const int_t delBtnWidth = 85;
        std::string delBtnText = isEs ? "Eliminar Skin" : "Delete Skin";
        buttonDeleteSkin = new GuiButton(BUTTON_ID_DELETE_SKIN, dialogLeft, btnY, delBtnWidth, p2BtnHeight, delBtnText);
        controlList.push_back(buttonDeleteSkin);
    }
    else
    {
        buttonDeleteSkin = nullptr;
    }
}

void GuiSkinSelector::switchPack(int newPackIndex)
{
    if (newPackIndex == currentPackIndex)
        return;

    if (newPackIndex == 1 && SkinManager::getCustomSkins().empty())
        return;

    currentPackIndex = newPackIndex;
    if (!isPlayer2Skin)
        SkinManager::setSelectedPackIndex(currentPackIndex);
    currentSkinIndex = 0;
    scrollOffset = 0.0f;

    if (mc != nullptr && mc->sndManager != nullptr)
        mc->sndManager->playSoundFX("random.click", 1.0f, 1.1f);

    initGui();
}

void GuiSkinSelector::deleteCurrentCustomSkin()
{
    if (currentPackIndex != 1)
        return;

    const auto &customs = SkinManager::getCustomSkins();
    if (customs.empty() || currentSkinIndex < 0 || currentSkinIndex >= static_cast<int>(customs.size()))
        return;

    std::string skinId = customs[currentSkinIndex].id;
    bool deleted = SkinManager::deleteCustomSkin(skinId);
    if (!deleted)
        return;

    if (mc != nullptr && mc->sndManager != nullptr)
        mc->sndManager->playSoundFX("random.break", 1.0f, 1.0f);

    // If no custom skins are left, return immediately to the main menu as requested
    if (SkinManager::getCustomSkins().empty())
    {
        cancelAndReturn();
        return;
    }

    if (currentSkinIndex >= static_cast<int>(SkinManager::getCustomSkins().size()))
        currentSkinIndex = static_cast<int>(SkinManager::getCustomSkins().size()) - 1;

    initGui();
}

void GuiSkinSelector::handleSpecializedMenuInput()
{
#if PLATFORM_PS2 || PLATFORM_WII
    const PlatformTextInputSnapshot pad = platformTextInputSnapshot(platformMenuPad());
    if (!pad.connected)
        return;

#if PLATFORM_PS2
    const Ps2PadSnapshot &ps2Pad = ps2PadGetSnapshot(platformMenuPad());
    const unsigned short held = ps2Pad.held;
    unsigned short pressed = held & ~lastPadHeld;
    lastPadHeld = held;

    if (ps2ActionReleaseLatch)
    {
        pressed &= ~PS2_PAD_CROSS;
        if ((held & PS2_PAD_CROSS) == 0)
            ps2ActionReleaseLatch = false;
    }

    // Circle: Cancel / Return
    if ((pressed & PS2_PAD_CIRCLE) != 0)
    {
        cancelAndReturn();
        return;
    }

    // Triangle: Open Load Skins screen
    if ((pressed & PS2_PAD_TRIANGLE) != 0)
    {
        if (mc != nullptr && mc->sndManager != nullptr)
            mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
        mc->displayGuiScreen(new GuiLoadSkinsMenu(this));
        return;
    }

    // Square: Delete current custom skin (when on custom pack)
    if ((pressed & PS2_PAD_SQUARE) != 0)
    {
        if (currentPackIndex == 1)
        {
            deleteCurrentCustomSkin();
            return;
        }
    }

    // L2 / R2: Switch between skin packs (tabs)
    if ((pressed & (PS2_PAD_L2 | PS2_PAD_R2)) != 0)
    {
        if (SkinManager::getPackCount() > 1)
        {
            switchPack(1 - currentPackIndex);
            return;
        }
    }

    // L1 / R1: Navigate carousel skins (D-Pad disabled for carousel to avoid conflict with pointer)
    bool movedLeft = false;
    bool movedRight = false;

    if ((pressed & PS2_PAD_L1) != 0)
    {
        movedLeft = true;
        dpadRepeatTimer = 0;
    }
    else if ((held & PS2_PAD_L1) != 0)
    {
        dpadRepeatTimer++;
        if (dpadRepeatTimer > 15 && (dpadRepeatTimer % 5) == 0)
            movedLeft = true;
    }

    if ((pressed & PS2_PAD_R1) != 0)
    {
        movedRight = true;
        dpadRepeatTimer = 0;
    }
    else if ((held & PS2_PAD_R1) != 0)
    {
        dpadRepeatTimer++;
        if (dpadRepeatTimer > 15 && (dpadRepeatTimer % 5) == 0)
            movedRight = true;
    }

    if (movedLeft) prevSkin();
    if (movedRight) nextSkin();

    // Confirm selection (Cross) - ONLY when pointer is NOT hovering over an active GUI button
    if ((pressed & PS2_PAD_CROSS) != 0)
    {
        int mx = 0, my = 0;
        PlatformCompat::getMouseState(&mx, &my);
        int mouseX = (mc != nullptr && mc->displayWidth > 0) ? (mx * width) / mc->displayWidth : 0;
        int mouseY = (mc != nullptr && mc->displayHeight > 0) ? (my * height) / mc->displayHeight : 0;
        bool overButton = false;
        for (GuiButton *btn : controlList)
        {
            if (btn != nullptr && btn->enabled && btn->mousePressed(mc, mouseX, mouseY))
            {
                overButton = true;
                break;
            }
        }
        if (!overButton)
        {
            while (lwjgl::Mouse::next()) {}
            selectAndConfirm();
            return;
        }
    }

#elif PLATFORM_WII
    // Wii Classic Controller / GameCube / Wiimote D-Pad
    if ((pad.pressed & (PLATFORM_TEXT_BACK | PLATFORM_TEXT_CLOSE)) != 0)
    {
        cancelAndReturn();
        return;
    }

    if ((pad.pressed & PLATFORM_TEXT_LEFT) != 0)
        prevSkin();
    else if ((pad.pressed & PLATFORM_TEXT_RIGHT) != 0)
        nextSkin();

    if ((pad.pressed & (PLATFORM_TEXT_UP | PLATFORM_TEXT_DOWN)) != 0 && SkinManager::getPackCount() > 1)
    {
        switchPack(1 - currentPackIndex);
        return;
    }

    if ((pad.pressed & PLATFORM_TEXT_TYPE) != 0)
    {
        selectAndConfirm();
        return;
    }
#endif
#endif
}

void GuiSkinSelector::updateScreen()
{
    GuiScreen::updateScreen();

    // Smoothly dampen carousel scrolling transition
    if (std::fabs(scrollOffset) > 0.001f)
    {
        scrollOffset *= 0.65f;
        if (std::fabs(scrollOffset) < 0.002f)
            scrollOffset = 0.0f;
    }
}

void GuiSkinSelector::keyTyped(char_t c, int_t key)
{
    if (key == 1 || key == lwjgl::Keyboard::KEY_ESCAPE) // ESC
    {
        cancelAndReturn();
        return;
    }
    if (key == lwjgl::Keyboard::KEY_RETURN || key == lwjgl::Keyboard::KEY_NUMPADENTER)
    {
        selectAndConfirm();
        return;
    }

    // Left arrow or A: Previous skin
    if (key == lwjgl::Keyboard::KEY_LEFT || key == lwjgl::Keyboard::KEY_A)
    {
        prevSkin();
        return;
    }

    // Right arrow or D: Next skin
    if (key == lwjgl::Keyboard::KEY_RIGHT || key == lwjgl::Keyboard::KEY_D)
    {
        nextSkin();
        return;
    }

    // Tab: Switch pack/tab
    if (key == lwjgl::Keyboard::KEY_TAB)
    {
        if (SkinManager::getPackCount() > 1)
            switchPack(1 - currentPackIndex);
        return;
    }

    // Delete: Delete current custom skin
    if (key == lwjgl::Keyboard::KEY_DELETE)
    {
        deleteCurrentCustomSkin();
        return;
    }

    GuiScreen::keyTyped(c, key);
}

void GuiSkinSelector::nextSkin()
{
    const int total = SkinManager::getSkinCountForPack(currentPackIndex);
    if (total <= 0)
        return;

    currentSkinIndex = (currentSkinIndex + 1) % total;
    scrollOffset -= 1.0f;
    if (scrollOffset < -1.5f)
        scrollOffset = -1.5f;

    if (mc != nullptr && mc->sndManager != nullptr)
        mc->sndManager->playSoundFX("random.click", 1.0f, 1.2f);
}

void GuiSkinSelector::prevSkin()
{
    const int total = SkinManager::getSkinCountForPack(currentPackIndex);
    if (total <= 0)
        return;

    currentSkinIndex = ((currentSkinIndex - 1) % total + total) % total;
    scrollOffset += 1.0f;
    if (scrollOffset > 1.5f)
        scrollOffset = 1.5f;

    if (mc != nullptr && mc->sndManager != nullptr)
        mc->sndManager->playSoundFX("random.click", 1.0f, 1.2f);
}

void GuiSkinSelector::selectAndConfirm()
{
    const SkinEntry *skin = SkinManager::getSkin(currentPackIndex, currentSkinIndex);
    if (skin != nullptr)
    {
        if (isPlayer2Skin)
        {
            SkinManager::setSelectedSkinIdP2(skin->id);
            if (mc != nullptr && mc->gameSettings != nullptr)
            {
                mc->gameSettings->selectedSkinP2 = skin->id;
                mc->gameSettings->saveOptions();
            }
        }
        else
        {
            SkinManager::setSelectedPackIndex(currentPackIndex);
            SkinManager::setSelectedSkinId(skin->id);
            if (mc != nullptr && mc->gameSettings != nullptr)
            {
                mc->gameSettings->selectedSkin = skin->id;
                mc->gameSettings->saveOptions();
            }
            if (mc != nullptr && mc->thePlayer != nullptr)
            {
                mc->thePlayer->setEntityTexture(skin->modelPath);
                mc->thePlayer->skinUrl = "";
            }
        }
    }

    if (mc != nullptr && mc->sndManager != nullptr)
        mc->sndManager->playSoundFX("random.action", 1.0f, 1.0f);

    mc->displayGuiScreen(parentScreen);
}

void GuiSkinSelector::cancelAndReturn()
{
    if (mc != nullptr && mc->sndManager != nullptr)
        mc->sndManager->playSoundFX("random.back", 1.0f, 1.0f);

    mc->displayGuiScreen(parentScreen);
}

void GuiSkinSelector::mouseClicked(int_t mouseX, int_t mouseY, int_t button)
{
    GuiScreen::mouseClicked(mouseX, mouseY, button);

    if (button != 0)
        return;

    // If an active button was clicked, don't trigger carousel actions
    for (GuiButton *btn : controlList)
    {
        if (btn != nullptr && btn->enabled && btn->mousePressed(mc, mouseX, mouseY))
            return;
    }

    // Click on nameplate confirms
    if (mouseY >= nameplateY && mouseY <= nameplateY + nameplateHeight &&
        mouseX >= rightPanelX && mouseX <= rightPanelX + rightPanelWidth)
    {
        selectAndConfirm();
        return;
    }

    // Click on center character confirms
    if (mouseX >= carouselCenterX - 25 && mouseX <= carouselCenterX + 25 &&
        mouseY >= carouselGroundY - 75 && mouseY <= carouselGroundY)
    {
        selectAndConfirm();
        return;
    }

    // Click on left skin in carousel advances to left
    if (mouseX >= carouselCenterX - 85 && mouseX < carouselCenterX - 25 &&
        mouseY >= carouselGroundY - 65 && mouseY <= carouselGroundY)
    {
        prevSkin();
        return;
    }

    // Click on right skin in carousel advances to right
    if (mouseX > carouselCenterX + 25 && mouseX <= carouselCenterX + 85 &&
        mouseY >= carouselGroundY - 65 && mouseY <= carouselGroundY)
    {
        nextSkin();
        return;
    }
}

void GuiSkinSelector::actionPerformed(GuiButton *button)
{
    if (!button->enabled)
        return;

    if (button->id == BUTTON_ID_TAB_DEFAULT)
    {
        switchPack(0);
    }
    else if (button->id == BUTTON_ID_TAB_CUSTOM)
    {
        switchPack(1);
    }
    else if (button->id == BUTTON_ID_PLAYER2)
    {
        if (mc != nullptr && mc->sndManager != nullptr)
            mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
        mc->displayGuiScreen(new GuiSkinSelector(this, true));
    }
    else if (button->id == BUTTON_ID_LOAD_SKINS)
    {
        if (mc != nullptr && mc->sndManager != nullptr)
            mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
        mc->displayGuiScreen(new GuiLoadSkinsMenu(this));
    }
    else if (button->id == BUTTON_ID_DELETE_SKIN)
    {
        deleteCurrentCustomSkin();
    }
}

bool GuiSkinSelector::doesGuiPauseGame()
{
    return true;
}

// ----------------------------------------------------------------------------
// Custom Visual Drawing Helpers
// ----------------------------------------------------------------------------

void GuiSkinSelector::drawBeveledPanel(int_t left, int_t top, int_t right, int_t bottom, int_t fillColor)
{
    drawRect(left, top, right, bottom, fillColor);
    drawRect(left, top, right, top + 1, 0x40FFFFFF);
    drawRect(left, top, left + 1, bottom, 0x40FFFFFF);
    drawRect(left, bottom - 1, right, bottom, 0xFF000000);
    drawRect(right - 1, top, right, bottom, 0xFF000000);
}

void GuiSkinSelector::drawInsetPanel(int_t left, int_t top, int_t right, int_t bottom, int_t fillColor)
{
    drawRect(left, top, right, bottom, fillColor);
    drawRect(left, top, right, top + 1, 0xFF000000);
    drawRect(left, top, left + 1, bottom, 0xFF000000);
    drawRect(left, bottom - 1, right, bottom, 0x30FFFFFF);
    drawRect(right - 1, top, right, bottom, 0x30FFFFFF);
}

void GuiSkinSelector::drawFeetShadow(float centerX, float groundY, float radiusX, float radiusY, float alpha)
{
    renderEnable(RenderCapability::Blend);
    renderDisable(RenderCapability::Texture2D);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);

    int a = static_cast<int>(alpha * 90.0f);
    if (a < 0) a = 0;
    if (a > 255) a = 255;
    int color = (a << 24) | 0x000000;

    int_t x1 = static_cast<int_t>(centerX - radiusX);
    int_t x2 = static_cast<int_t>(centerX + radiusX);
    int_t y1 = static_cast<int_t>(groundY - radiusY * 0.5f);
    int_t y2 = static_cast<int_t>(groundY + radiusY * 0.5f);

    drawRect(x1 + 2, y1, x2 - 2, y2, color);
    renderDisable(RenderCapability::Blend);
}

void GuiSkinSelector::drawFrontPreview(const SkinEntry *skin, float x, float y, float w, float h, float alpha)
{
    if (skin == nullptr || mc == nullptr || mc->renderEngine == nullptr)
        return;

    // If pre-rendered 16x32 front preview is present, draw single quad
    if (!skin->isCustom && !skin->frontPath.empty())
    {
        int texId = mc->renderEngine->getTexture(skin->frontPath);
        if (texId >= 0)
        {
            mc->renderEngine->bindTexture(texId);
            renderEnable(RenderCapability::Texture2D);
            renderEnable(RenderCapability::Blend);
            renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
            renderColor4f(alpha, alpha, alpha, alpha);

            Tessellator &tess = Tessellator::instance;
            tess.startDrawingQuads();
            tess.addVertexWithUV(x,     y + h, zLevel, 0.0f, 1.0f);
            tess.addVertexWithUV(x + w, y + h, zLevel, 1.0f, 1.0f);
            tess.addVertexWithUV(x + w, y,     zLevel, 1.0f, 0.0f);
            tess.addVertexWithUV(x,     y,     zLevel, 0.0f, 0.0f);
            tess.draw();
            renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            return;
        }
    }

    // Custom skin (or fallback): Assemble the 2D front character dynamically
    int texId = -1;
    if (!skin->frontPath.empty())
        texId = mc->renderEngine->getTexture(skin->frontPath);

    if (texId < 0 && !skin->skinPath.empty())
        texId = mc->renderEngine->getTexture(skin->skinPath);

    if (texId < 0 && !skin->modelPath.empty())
        texId = mc->renderEngine->getTexture(skin->modelPath);

    if (texId < 0)
        return;

    mc->renderEngine->bindTexture(texId);
    renderEnable(RenderCapability::Texture2D);
    renderEnable(RenderCapability::Blend);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
    renderColor4f(alpha, alpha, alpha, alpha);

    int_t tw = 64, th = 32;
    mc->renderEngine->getTextureDimensions(texId, &tw, &th);
    if (th <= 0) th = 32;

    // If the texture loaded is already a 16x32 front preview, draw single quad
    if (tw == 16 && th == 32)
    {
        Tessellator &tess = Tessellator::instance;
        tess.startDrawingQuads();
        tess.addVertexWithUV(x,     y + h, zLevel, 0.0f, 1.0f);
        tess.addVertexWithUV(x + w, y + h, zLevel, 1.0f, 1.0f);
        tess.addVertexWithUV(x + w, y,     zLevel, 1.0f, 0.0f);
        tess.addVertexWithUV(x,     y,     zLevel, 0.0f, 0.0f);
        tess.draw();
        renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        return;
    }

    // Assemble standing body parts from standard skin sheet
    const float uScale = 1.0f / 64.0f;
    const float vScale = 1.0f / static_cast<float>(th);
    const float unitW = w / 16.0f;
    const float unitH = h / 32.0f;

    Tessellator &tess = Tessellator::instance;
    tess.startDrawingQuads();

    auto drawQuad = [&](float qx, float qy, float qw, float qh, float su0, float sv0, float su1, float sv1) {
        tess.addVertexWithUV(qx,      qy + qh, zLevel, su0, sv1);
        tess.addVertexWithUV(qx + qw, qy + qh, zLevel, su1, sv1);
        tess.addVertexWithUV(qx + qw, qy,      zLevel, su1, sv0);
        tess.addVertexWithUV(qx,      qy,      zLevel, su0, sv0);
    };

    // 1. Head front (8,8 to 16,16)
    drawQuad(x + 4.0f * unitW, y, 8.0f * unitW, 8.0f * unitH, 8.0f * uScale, 8.0f * vScale, 16.0f * uScale, 16.0f * vScale);

    // 2. Hat overlay (40,8 to 48,16)
    drawQuad(x + 3.5f * unitW, y - 0.5f * unitH, 9.0f * unitW, 9.0f * unitH, 40.0f * uScale, 8.0f * vScale, 48.0f * uScale, 16.0f * vScale);

    // 3. Torso front (20,20 to 28,32)
    drawQuad(x + 4.0f * unitW, y + 8.0f * unitH, 8.0f * unitW, 12.0f * unitH, 20.0f * uScale, 20.0f * vScale, 28.0f * uScale, 32.0f * vScale);

    // 4. Right Arm front (44,20 to 48,32)
    drawQuad(x, y + 8.0f * unitH, 4.0f * unitW, 12.0f * unitH, 44.0f * uScale, 20.0f * vScale, 48.0f * uScale, 32.0f * vScale);

    // 5. Left Arm front
    if (th == 64)
        drawQuad(x + 12.0f * unitW, y + 8.0f * unitH, 4.0f * unitW, 12.0f * unitH, 36.0f * uScale, 52.0f * vScale, 40.0f * uScale, 64.0f * vScale);
    else
        drawQuad(x + 12.0f * unitW, y + 8.0f * unitH, 4.0f * unitW, 12.0f * unitH, 48.0f * uScale, 20.0f * vScale, 44.0f * uScale, 32.0f * vScale);

    // 6. Right Leg front (4,20 to 8,32)
    drawQuad(x + 4.0f * unitW, y + 20.0f * unitH, 4.0f * unitW, 12.0f * unitH, 4.0f * uScale, 20.0f * vScale, 8.0f * uScale, 32.0f * vScale);

    // 7. Left Leg front
    if (th == 64)
        drawQuad(x + 8.0f * unitW, y + 20.0f * unitH, 4.0f * unitW, 12.0f * unitH, 20.0f * uScale, 52.0f * vScale, 24.0f * uScale, 64.0f * vScale);
    else
        drawQuad(x + 8.0f * unitW, y + 20.0f * unitH, 4.0f * unitW, 12.0f * unitH, 8.0f * uScale, 20.0f * vScale, 4.0f * uScale, 32.0f * vScale);

    tess.draw();
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void GuiSkinSelector::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    (void)partialTick;

    // 1. Darkened background
    drawDefaultBackground();

    // 2. Left Panel
    const int_t leftX1 = dialogLeft;
    const int_t leftY1 = dialogTop;
    const int_t leftX2 = dialogLeft + leftPanelWidth;
    const int_t leftY2 = dialogTop + dialogHeight;
    drawInsetPanel(leftX1, leftY1, leftX2, leftY2, 0xD0101010);

    // Pack artwork container (square)
    const int_t artSize = std::min<int_t>(leftPanelWidth - 20, 60);
    const int_t artX = leftX1 + (leftPanelWidth - artSize) / 2;
    const int_t artY = leftY1 + 8;
    drawInsetPanel(artX - 1, artY - 1, artX + artSize + 1, artY + artSize + 1, 0xFF000000);

    // Draw pack cover art
    if (mc != nullptr && mc->renderEngine != nullptr)
    {
        int coverTex = mc->renderEngine->getTexture("/skins/default_pack.png");
        if (coverTex >= 0)
        {
            mc->renderEngine->bindTexture(coverTex);
            renderEnable(RenderCapability::Texture2D);
            renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            Tessellator &tess = Tessellator::instance;
            tess.startDrawingQuads();
            tess.addVertexWithUV(artX,           artY + artSize, zLevel, 0.0f, 1.0f);
            tess.addVertexWithUV(artX + artSize, artY + artSize, zLevel, 1.0f, 1.0f);
            tess.addVertexWithUV(artX + artSize, artY,           zLevel, 1.0f, 0.0f);
            tess.addVertexWithUV(artX,           artY,           zLevel, 0.0f, 0.0f);
            tess.draw();
        }
    }

    // 3. Right Panel (Skin Carousel and Details)
    const int_t rightX1 = rightPanelX;
    const int_t rightY1 = dialogTop;
    const int_t rightX2 = rightPanelX + rightPanelWidth;
    const int_t rightY2 = dialogTop + dialogHeight;
    drawInsetPanel(rightX1, rightY1, rightX2, rightY2, 0xD0101010);

    // Header bar inside right panel
    drawRect(rightX1 + 1, rightY1 + 1, rightX2 - 1, rightY1 + 18, 0x80282828);
    StringTranslate *tr = StringTranslate::getInstance();
    const bool isEs = (tr != nullptr && tr->getCurrentLanguage().rfind("es_", 0) == 0);

    std::string headerName = SkinManager::getPackName(currentPackIndex);
    if (isPlayer2Skin)
        headerName += isEs ? " (Jugador 2)" : " (Player 2)";
    fontRenderer->drawStringWithShadow(headerName, rightX1 + 10, rightY1 + 5, isPlayer2Skin ? 0x55FFFF : 0xFFFFFF);

    if (isPlayer2Skin)
    {
        std::string p2Title = isEs ? "[ SELECTOR DE SKIN: JUGADOR 2 ]" : "[ PLAYER 2 SKIN SELECTOR ]";
        drawCenteredString(fontRenderer, p2Title, width / 2, dialogTop - 11, 0x55FFFF);
    }

    const int totalSkins = SkinManager::getSkinCountForPack(currentPackIndex);
    if (totalSkins > 0)
    {
        std::string countStr = std::to_string(currentSkinIndex + 1) + " / " + std::to_string(totalSkins);
        int_t countW = fontRenderer->getStringWidth(countStr);
        fontRenderer->drawStringWithShadow(countStr, rightX2 - countW - 10, rightY1 + 5, 0xAAAAAA);
    }

    // 4. Infinite Carousel Area
    if (totalSkins > 0)
    {
        const float spacing = static_cast<float>(rightPanelWidth) * 0.22f;

        // Render slots from k = 3 down to -3 (so center skin renders on top of flanking ones)
        const int slotOrder[] = { -3, 3, -2, 2, -1, 1, 0 };
        for (int k : slotOrder)
        {
            float t = static_cast<float>(k) - scrollOffset;
            float dist = std::fabs(t);
            if (dist > 2.6f)
                continue;

            int skinIndex = ((currentSkinIndex + k) % totalSkins + totalSkins) % totalSkins;
            const SkinEntry *skin = SkinManager::getSkin(currentPackIndex, skinIndex);
            if (skin == nullptr)
                continue;

            // Scale factor based on distance from center
            float scale = 3.4f - 1.05f * std::min(dist, 2.0f);
            if (scale < 1.0f) scale = 1.0f;

            float alpha = 1.0f - 0.20f * std::min(dist, 2.0f);
            if (alpha < 0.35f) alpha = 0.35f;

            float skinW = 16.0f * scale;
            float skinH = 32.0f * scale;
            float skinX = static_cast<float>(carouselCenterX) + t * spacing - skinW * 0.5f;
            float skinY = static_cast<float>(carouselGroundY) - skinH;

            // Draw soft ground shadow
            drawFeetShadow(static_cast<float>(carouselCenterX) + t * spacing, static_cast<float>(carouselGroundY),
                           skinW * 0.45f, 4.0f, alpha);

            // Draw front preview
            drawFrontPreview(skin, skinX, skinY, skinW, skinH, alpha);
        }
    }

    // 5. Nameplate banner
    const int_t npPadding = 8;
    const int_t npW = rightPanelWidth - npPadding * 2;
    const int_t npX = rightX1 + npPadding;
    const int_t npY = nameplateY;
    drawInsetPanel(npX, npY, npX + npW, npY + nameplateHeight, 0xFF181818);

    const SkinEntry *selectedSkin = SkinManager::getSkin(currentPackIndex, currentSkinIndex);
    if (selectedSkin != nullptr)
    {
        std::string displayName = toUpperString(selectedSkin->name);
        if (isPlayer2Skin)
            displayName += " [P2]";
        drawCenteredString(fontRenderer, displayName, npX + npW / 2, npY + 6, isPlayer2Skin ? 0xAAFFFF : 0xFFFFAA);
    }

    // Keep the upstream skin actions while sharing the platform icon/fallback renderer.
    const int_t footerY = height - 13;
#if PLATFORM_PS2
    const std::string buttons[] = {"Cross", "Circle", "Triangle", "Square"};
    const std::string actions[] = {uiText("Select"), uiText("Back"), uiText("Load"), uiText("Delete")};
    drawControlHintRow(mc, width, footerY - 14, buttons, actions, currentPackIndex == 1 ? 4 : 3);
    const std::string navigation[] = {"L1/R1", "L2/R2"};
    const std::string navigationActions[] = {uiText("Skin"), uiText("Tab")};
    drawControlHintRow(mc, width, footerY, navigation, navigationActions, 2);
#elif PLATFORM_WII
    const std::string buttons[] = {"A", "B", "L/R", "ZL/ZR"};
    const std::string actions[] = {uiText("Select"), uiText("Back"), uiText("Skin"), uiText("Tab")};
    drawControlHintRow(mc, width, footerY, buttons, actions, 4);
#else
    const std::string buttons[] = {"Enter", "Esc", "Left/Right", "Tab"};
    const std::string actions[] = {uiText("Select"), uiText("Back"), uiText("Skin"), uiText("Tab")};
    drawControlHintRow(mc, width, footerY, buttons, actions, 4);
#endif

    // 7. Draw standard GUI controls (buttons, tabs, arrows) AND the software cursor
    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
