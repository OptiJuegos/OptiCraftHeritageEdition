#include "GuiSkinSelector.h"

#include "SkinManager.h"
#include "net/minecraft/src/FontRenderer.h"
#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/GuiButton.h"
#include "net/minecraft/src/Minecraft.h"
#include "net/minecraft/src/RenderEngine.h"
#include "net/minecraft/src/SoundManager.h"
#include "net/minecraft/src/Tessellator.h"
#include "net/minecraft/src/EntityPlayerSP.h"
#include "platform/Input.h"
#include "platform/RenderAPI.h"
#include "pc/lwjgl/Keyboard.h"

#include <cmath>
#include <algorithm>

namespace
{
constexpr int BUTTON_ID_PLAYER2 = 1001;

// Transform text to uppercase for Legacy Console style banners/nameplates
std::string toUpperString(const std::string &str)
{
    std::string out = str;
    for (char &c : out)
    {
        if (c >= 'a' && c <= 'z')
            c -= ('a' - 'A');
    }
    return out;
}
} // namespace

GuiSkinSelector::GuiSkinSelector(GuiScreen *parent)
    : parentScreen(parent)
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
    , buttonPlayer2Skin(nullptr)
#if defined(PS2_PLATFORM) || defined(PLATFORM_PS2)
    , ps2ActionReleaseLatch(true)
    , stickNavLatched(false)
    , dpadRepeatTimer(0)
    , stickRepeatTimer(0)
#endif
{
    SkinManager::init();
    currentSkinIndex = SkinManager::getSelectedIndex();
}

void GuiSkinSelector::initGui()
{
    controlList.clear();

    if (mc != nullptr && mc->gameSettings != nullptr && !mc->gameSettings->selectedSkin.empty())
    {
        SkinManager::setSelectedSkinId(mc->gameSettings->selectedSkin);
        currentSkinIndex = SkinManager::getSelectedIndex();
    }

    // Dialog layout sizing
    dialogWidth = std::min<int_t>(width - 24, 380);
    dialogHeight = std::min<int_t>(height - 40, 206);
    dialogLeft = (width - dialogWidth) / 2;
    dialogTop = (height - 20 - dialogHeight) / 2;

    leftPanelWidth = dialogWidth * 29 / 100;
    rightPanelX = dialogLeft + leftPanelWidth + 4;
    rightPanelWidth = dialogWidth - leftPanelWidth - 4;

    carouselCenterX = rightPanelX + rightPanelWidth / 2;
    nameplateHeight = 28;
    nameplateY = dialogTop + dialogHeight - nameplateHeight - 10;
    carouselGroundY = nameplateY - 8;

    // Disabled "Choose 2nd Player Skin" button in the bottom-right corner as requested
    const int_t p2BtnWidth = 140;
    const int_t p2BtnHeight = 18;
    const int_t p2BtnX = width - p2BtnWidth - 8;
    const int_t p2BtnY = height - p2BtnHeight - 4;

    buttonPlayer2Skin = new GuiButton(BUTTON_ID_PLAYER2, p2BtnX, p2BtnY, p2BtnWidth, p2BtnHeight, "Choose 2nd Player Skin");
    buttonPlayer2Skin->enabled = false; // Disabled (grayed out) temporarily
    controlList.push_back(buttonPlayer2Skin);
}

void GuiSkinSelector::handleSpecializedMenuInput()
{
#if defined(PS2_PLATFORM) || defined(PLATFORM_PS2) || defined(PLATFORM_WII)
    const PlatformTextInputSnapshot pad = platformTextInputSnapshot(platformMenuPad());
    if (!pad.connected)
        return;

#if defined(PS2_PLATFORM) || defined(PLATFORM_PS2)
    std::uint32_t pressed = pad.pressed;
    if (ps2ActionReleaseLatch)
    {
        pressed &= ~PLATFORM_TEXT_TYPE;
        if ((pad.held & PLATFORM_TEXT_TYPE) == 0)
            ps2ActionReleaseLatch = false;
    }

    // Cancel / Return (Circle, Triangle, Square)
    if ((pressed & (PLATFORM_TEXT_CLOSE | PLATFORM_TEXT_SHIFT | PLATFORM_TEXT_BACK)) != 0)
    {
        cancelAndReturn();
        return;
    }

    // D-Pad navigation with initial edge + repeat on hold
    bool movedLeft = false;
    bool movedRight = false;

    if ((pressed & PLATFORM_TEXT_LEFT) != 0)
    {
        movedLeft = true;
        dpadRepeatTimer = 0;
    }
    else if ((pad.held & PLATFORM_TEXT_LEFT) != 0)
    {
        dpadRepeatTimer++;
        if (dpadRepeatTimer >= 16 && (dpadRepeatTimer % 5 == 0))
            movedLeft = true;
    }
    else if ((pressed & PLATFORM_TEXT_RIGHT) != 0)
    {
        movedRight = true;
        dpadRepeatTimer = 0;
    }
    else if ((pad.held & PLATFORM_TEXT_RIGHT) != 0)
    {
        dpadRepeatTimer++;
        if (dpadRepeatTimer >= 16 && (dpadRepeatTimer % 5 == 0))
            movedRight = true;
    }
    else
    {
        dpadRepeatTimer = 0;
    }

    // Left analog stick navigation (deadzone + latch + repeat)
    if (!movedLeft && !movedRight)
    {
        const PlatformGamepadSnapshot stick = platformGamepadSnapshot(platformMenuPad());
        if (stick.connected)
        {
            if (stick.leftX < -0.55f)
            {
                if (!stickNavLatched)
                {
                    movedLeft = true;
                    stickNavLatched = true;
                    stickRepeatTimer = 0;
                }
                else
                {
                    stickRepeatTimer++;
                    if (stickRepeatTimer >= 16 && (stickRepeatTimer % 5 == 0))
                        movedLeft = true;
                }
            }
            else if (stick.leftX > 0.55f)
            {
                if (!stickNavLatched)
                {
                    movedRight = true;
                    stickNavLatched = true;
                    stickRepeatTimer = 0;
                }
                else
                {
                    stickRepeatTimer++;
                    if (stickRepeatTimer >= 16 && (stickRepeatTimer % 5 == 0))
                        movedRight = true;
                }
            }
            else if (std::fabs(stick.leftX) < 0.25f)
            {
                stickNavLatched = false;
                stickRepeatTimer = 0;
            }
        }
    }

    if (movedLeft)
        prevSkin();
    else if (movedRight)
        nextSkin();

    // Cross / Confirm
    if ((pressed & PLATFORM_TEXT_TYPE) != 0)
    {
        selectAndConfirm();
    }
#elif PLATFORM_WII
    if ((pad.pressed & PLATFORM_TEXT_LEFT) != 0)
    {
        prevSkin();
    }
    else if ((pad.pressed & PLATFORM_TEXT_RIGHT) != 0)
    {
        nextSkin();
    }
    if ((pad.pressed & PLATFORM_TEXT_TYPE) != 0)
    {
        selectAndConfirm();
    }
    if ((pad.pressed & (PLATFORM_TEXT_BACK | PLATFORM_TEXT_CLOSE)) != 0)
    {
        cancelAndReturn();
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

void GuiSkinSelector::keyTyped(char_t, int_t key)
{
    if (key == lwjgl::Keyboard::KEY_ESCAPE)
    {
        cancelAndReturn();
        return;
    }
    if (key == lwjgl::Keyboard::KEY_LEFT || key == lwjgl::Keyboard::KEY_A)
    {
        prevSkin();
        return;
    }
    if (key == lwjgl::Keyboard::KEY_RIGHT || key == lwjgl::Keyboard::KEY_D)
    {
        nextSkin();
        return;
    }
    if (key == lwjgl::Keyboard::KEY_RETURN || key == lwjgl::Keyboard::KEY_SPACE)
    {
        selectAndConfirm();
        return;
    }
}

void GuiSkinSelector::nextSkin()
{
    const int total = SkinManager::getSkinCount();
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
    const int total = SkinManager::getSkinCount();
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
    const SkinEntry *skin = SkinManager::getSkin(currentSkinIndex);
    if (skin != nullptr)
    {
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

    // Check click inside carousel area
    if (mouseY >= dialogTop + 24 && mouseY < nameplateY)
    {
        if (mouseX >= rightPanelX && mouseX < carouselCenterX - 30)
        {
            prevSkin();
            return;
        }
        if (mouseX > carouselCenterX + 30 && mouseX <= rightPanelX + rightPanelWidth)
        {
            nextSkin();
            return;
        }
        if (mouseX >= carouselCenterX - 30 && mouseX <= carouselCenterX + 30)
        {
            selectAndConfirm();
            return;
        }
    }

    // Check click on nameplate
    if (mouseY >= nameplateY && mouseY <= nameplateY + nameplateHeight &&
        mouseX >= rightPanelX && mouseX <= rightPanelX + rightPanelWidth)
    {
        selectAndConfirm();
        return;
    }
}

void GuiSkinSelector::actionPerformed(GuiButton *button)
{
    (void)button;
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
    // Outer drop shadow (subtle dark outline)
    drawRect(left - 1, top - 1, right + 1, bottom + 1, 0xFF181818);

    // Inner main fill
    drawRect(left + 2, top + 2, right - 2, bottom - 2, fillColor);

    // Top and Left light highlight
    drawRect(left + 1, top + 1, right - 1, top + 2, 0xFFFFFFFF);
    drawRect(left + 1, top + 2, left + 2, bottom - 1, 0xFFFFFFFF);

    // Bottom and Right dark bevel shadow
    drawRect(left + 2, bottom - 2, right - 1, bottom - 1, 0xFF7A7A7A);
    drawRect(right - 2, top + 2, right - 1, bottom - 2, 0xFF7A7A7A);
}

void GuiSkinSelector::drawInsetPanel(int_t left, int_t top, int_t right, int_t bottom, int_t fillColor)
{
    // Dark outer border
    drawRect(left, top, right, bottom, 0xFF222222);

    // Inner fill
    drawRect(left + 2, top + 2, right - 2, bottom - 2, fillColor);

    // Top and Left inset shadow
    drawRect(left + 1, top + 1, right - 1, top + 2, 0xFF666666);
    drawRect(left + 1, top + 2, left + 2, bottom - 1, 0xFF666666);

    // Bottom and Right inset highlight
    drawRect(left + 2, bottom - 2, right - 1, bottom - 1, 0xFFE8E8E8);
    drawRect(right - 2, top + 2, right - 1, bottom - 2, 0xFFE8E8E8);
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
    drawRect(x1, y1 + 1, x2, y2 - 1, color);

    renderEnable(RenderCapability::Texture2D);
}

void GuiSkinSelector::drawFrontPreview(const std::string &path, float x, float y, float w, float h, float alpha)
{
    if (mc == nullptr || mc->renderEngine == nullptr)
        return;

    int texId = mc->renderEngine->getTexture(path);
    if (texId < 0)
        return;

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
}

// ----------------------------------------------------------------------------
// Screen Rendering
// ----------------------------------------------------------------------------

void GuiSkinSelector::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    (void)partialTick;

    // 1. Darkened background
    drawDefaultBackground();

    const int_t panelColor = 0xFFC6C6C6;

    // 2. Left Panel (Skin Pack Selector)
    const int_t leftX1 = dialogLeft;
    const int_t leftY1 = dialogTop;
    const int_t leftX2 = dialogLeft + leftPanelWidth;
    const int_t leftY2 = dialogTop + dialogHeight;
    drawBeveledPanel(leftX1, leftY1, leftX2, leftY2, panelColor);

    // Pack artwork container (square)
    const int_t artPadding = 8;
    const int_t artSize = std::min<int_t>(leftPanelWidth - artPadding * 2, 72);
    const int_t artX = leftX1 + (leftPanelWidth - artSize) / 2;
    const int_t artY = leftY1 + 10;
    drawInsetPanel(artX - 2, artY - 2, artX + artSize + 2, artY + artSize + 2, 0xFF101010);

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

    // Pack list button ("Default Skins")
    const int_t packBtnX = leftX1 + 6;
    const int_t packBtnY = artY + artSize + 10;
    const int_t packBtnW = leftPanelWidth - 12;
    const int_t packBtnH = 20;
    drawInsetPanel(packBtnX, packBtnY, packBtnX + packBtnW, packBtnY + packBtnH, 0xFF9E9E9E);
    drawCenteredString(fontRenderer, "Default Skins", packBtnX + packBtnW / 2, packBtnY + 6, 0xFFFFFF);

    // 3. Right Panel (Skin Carousel and Details)
    const int_t rightX1 = rightPanelX;
    const int_t rightY1 = dialogTop;
    const int_t rightX2 = rightPanelX + rightPanelWidth;
    const int_t rightY2 = dialogTop + dialogHeight;
    drawBeveledPanel(rightX1, rightY1, rightX2, rightY2, panelColor);

    // Top Header Banner in Right Panel
    const int_t headerH = 18;
    const int_t headerY = rightY1 + 5;
    drawRect(rightX1 + 4, headerY, rightX2 - 4, headerY + headerH, 0x88242424);
    fontRenderer->drawStringWithShadow("Default Skins", rightX1 + 14, headerY + 5, 0xFFFFFF);

    // 4. Infinite Carousel Area
    const int totalSkins = SkinManager::getSkinCount();
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
            const SkinEntry *skin = SkinManager::getSkin(skinIndex);
            if (skin == nullptr)
                continue;

            // Scale factor based on distance from center
            // Center (dist=0): ~3.4x (height 108)
            // Near (dist=1): ~2.35x (height 75)
            // Far (dist=2): ~1.65x (height 52)
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
            drawFrontPreview(skin->frontPath, skinX, skinY, skinW, skinH, alpha);
        }
    }

    // 5. Bottom Nameplate Banner
    const int_t npPadding = 8;
    const int_t npW = rightPanelWidth - 44;
    const int_t npX = rightX1 + npPadding;
    const int_t npY = nameplateY;
    drawInsetPanel(npX, npY, npX + npW, npY + nameplateHeight, 0xFF383838);

    // Selected skin name
    const SkinEntry *selectedSkin = SkinManager::getSkin(currentSkinIndex);
    if (selectedSkin != nullptr)
    {
        std::string displayName = toUpperString(selectedSkin->name);
        drawCenteredString(fontRenderer, displayName, npX + npW / 2, npY + 10, 0xFFFFFF);
    }

    // Two small decorative indicator slots to the right of the nameplate (matching image.png)
    const int_t slotX = npX + npW + 6;
    const int_t slotW = 14;
    const int_t slotH = 11;
    drawInsetPanel(slotX, npY, slotX + slotW, npY + slotH, 0xFF383838);
    drawInsetPanel(slotX, npY + 14, slotX + slotW, npY + 14 + slotH, 0xFF383838);

    // 6. Navigation arrows (clickable on PC, visual cue)
    fontRenderer->drawStringWithShadow("<", rightX1 + 10, carouselGroundY - 48, 0xEEEEEE);
    fontRenderer->drawStringWithShadow(">", rightX2 - 16, carouselGroundY - 48, 0xEEEEEE);

    // 7. Footer / Controller Legend
    const int_t footerY = height - 14;
#if defined(PS2_PLATFORM) || defined(PLATFORM_PS2)
    const std::string hint = "[X] Select Skin   [O] Cancel   [D-Pad] Navigate";
#elif PLATFORM_WII
    const std::string hint = "[A] Select Skin   [B] Cancel   [D-Pad] Navigate";
#else
    const std::string hint = "[Enter] Select Skin   [Esc] Cancel   [< / >] Navigate";
#endif
    fontRenderer->drawStringWithShadow(hint, dialogLeft + 4, footerY, 0xF0F0F0);

    // 8. Draw standard GUI controls (like the disabled Player 2 button)
    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
