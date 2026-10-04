#include "net/minecraft/src/UiStrings.h"
#include "GuiMainMenu.h"
#include "platform/Log.h"
#include "platform/PlatformConfig.h"
#include "java/String.h"
#include "java/BufferedImage.h"
#include "ControlIcon.h"
#include "GuiButton.h"
#include "GuiButtonLanguage.h"
#include "GuiLanguage.h"
#include "GuiOptions.h"
#include "legacy/LegacyHelpOptions.h"
#include "legacy/LegacyGuiButton.h"
#include "legacy/LegacyLanguageOptions.h"
#include "GuiSelectWorld.h"
#include "GuiMultiplayer.h"
#include "GuiTexturePacks.h"
#include "mods/GuiMods.h"
#include "StringTranslate.h"
#include "Tessellator.h"
#include "MathHelper.h"
#include "FontRenderer.h"
#include "RenderEngine.h"
#include "Minecraft.h"
#include "GameSettings.h"
#include "SoundManager.h"
#include "pc/lwjgl/Keyboard.h"
#include "platform/Input.h"
#include "net/minecraft/src/legacy/LegacyMainMenu.h"
#include "net/minecraft/src/legacy/LegacyPlayGameScreen.h"
#include "net/minecraft/src/legacy/LegacyMainMenuLayout.h"
#include "net/minecraft/src/legacy/LegacyMenuHints.h"
#include "net/minecraft/src/legacy/LegacyMenuNavigation.h"
#include "skin/GuiSkinSelector.h"
#include "Session.h"
#include "net/minecraft/src/legacy/LegacyUiAssets.h"
#include "net/minecraft/src/legacy/LegacyPanorama.h"
#include "net/minecraft/src/legacy/LegacySceneLayout.h"
#include "net/minecraft/src/legacy/LegacySceneState.h"
#include "GameResources.h"
#include "java/System.h"
#include "java/Random.h"
#include "platform/RenderAPI.h"
#include "platform/PlatformTuning.h"
#include "platform/ClientPlatformPolicy.h"
#ifdef PS2_PLATFORM
#include "java/Resource.h"
#endif
#if PLATFORM_3DS
// The 3DS-only "Descarga QR" entry: back camera + httpc downloader
// (src/3ds/qr). The class itself is 3DS-only (globbed with src/3ds), so the
// include and every reference below stay inside the same platform gate.
#include "3ds/qr/GuiQrDownload.h"
#endif
#include <algorithm>
#include <fstream>
#include <memory>
#include <vector>
#include <ctime>
#include <cmath>
#include <cstdint>

namespace
{
Random g_mainMenuRand;

#if PLATFORM_3DS
// The 3DS title's two text moments share one size: the splash under the
// banner and the "Press START Button" gate on the touch panel both draw at
// 1.5x the 8 px font -- the 12 px height the control-hint row's icons use.
constexpr float_t TITLE_TEXT_SCALE = 1.5f;
#endif

int32_t javaStringHash(const std::string &value)
{
    uint32_t hash = 0;
    for (unsigned char c : value)
        hash = hash * 31u + static_cast<uint32_t>(c);
    return static_cast<int32_t>(hash);
}

void setPerspective(float_t fovY, float_t aspectRatio, float_t nearPlane, float_t farPlane)
{
#if PLATFORM_FLOAT_VERTEX_MATH
    const float_t radians = fovY * 3.14159265358979323846f / 360.0f;
    const float_t top = nearPlane * std::tan(radians);
    const float_t right = top * aspectRatio;
#else
    const double radians = static_cast<double>(fovY) * 3.14159265358979323846 / 360.0;
    const double top = static_cast<double>(nearPlane) * std::tan(radians);
    const double right = top * static_cast<double>(aspectRatio);
#endif
    renderFrustum(-right, right, -top, top, nearPlane, farPlane);
}
}

GuiMainMenu::GuiMainMenu()
    : updateCounter(0.0f)
    , splashText("missingno")
    , multiplayerButton(nullptr)
    , panoramaTimer(0)
    , viewportTexture(-1)
    , legacyPanoramaAvailable(false)
    , selectedControlIndex(-1)
    , hoveredControlIndex(-1)
{
    try
    {
        std::vector<std::string> lines;
        std::unique_ptr<std::istream> splashStream;
#ifndef PS2_PLATFORM
        splashStream = GameResources::open("/title/splashes.txt");
#else
        const char *ps2SplashPaths[] = {
            "/title/splashes.txt",
            "/assets/title/splashes.txt",
            "/minecraft/title/splashes.txt",
            "/resources/title/splashes.txt"
        };
        for (const char *path : ps2SplashPaths)
        {
            try
            {
                splashStream.reset(Resource::getResource(path));
                if (splashStream && *splashStream)
                {
                    MC_LOG_DEBUG("ps2", "splash resource loaded: %s\n", path);
                    break;
                }
            }
            catch (...)
            {
                splashStream.reset();
            }
        }
#endif

        if (splashStream && *splashStream)
        {
            std::string line;
            while (std::getline(*splashStream, line))
            {
                line = String::trimJava(line);
                if (!line.empty())
                    lines.push_back(line);
            }
        }

        if (!lines.empty())
        {
            do
            {
                splashText = lines[g_mainMenuRand.nextInt(static_cast<int_t>(lines.size()))];
            }
            while (lines.size() > 1 && javaStringHash(splashText) == 125780783);
        }
#ifdef PS2_PLATFORM
        MC_LOG_DEBUG("ps2", "splash lines=%u selected='%s'\n",
            static_cast<unsigned>(lines.size()), splashText.c_str());
#endif
    }
    catch (...)
    {
    }

    // Java draws the splash index before the logo easter-egg counter. Keeping the
    // same order keeps both values on the Java RNG stream.
    updateCounter = g_mainMenuRand.nextFloat();
}

GuiMainMenu::~GuiMainMenu()
{
    if (viewportTexture >= 0 && mc != nullptr && mc->renderEngine != nullptr)
        mc->renderEngine->deleteTexture(viewportTexture);
    viewportTexture = -1;

}

void GuiMainMenu::updateScreen()
{
    ++panoramaTimer;
    if (mc == nullptr || mc->gameSettings == nullptr || !mc->gameSettings->legacyUI)
        return;

    syncLegacySelection();
#if PLATFORM_PS2 || PLATFORM_WII
    const PlatformTextInputSnapshot pad = platformTextInputSnapshot(platformMenuPad());
    if ((pad.pressed & PLATFORM_TEXT_UP) != 0)
        moveLegacySelection(-1);
    else if ((pad.pressed & PLATFORM_TEXT_DOWN) != 0)
        moveLegacySelection(1);
#if PLATFORM_PS2
    if ((pad.pressed & PLATFORM_TEXT_TYPE) != 0)
        activateLegacySelection();
#elif PLATFORM_WII
    if (!platformMenuPointerActive() && (pad.pressed & PLATFORM_TEXT_TYPE) != 0)
        activateLegacySelection();
#endif
#endif
}

bool GuiMainMenu::doesGuiPauseGame()
{
    return false;
}

bool GuiMainMenu::usesSpecializedMenuNavigation() const
{
    return mc != nullptr && mc->gameSettings != nullptr && mc->gameSettings->legacyUI;
}

void GuiMainMenu::keyTyped(char_t, int_t key)
{
#if PLATFORM_3DS
    // START reaches us as ESC and A as RETURN. Either wakes the bottom
    // menu, and while it is still gated nothing else may reach the buttons
    // the panel is hiding (ESC would otherwise rebuild this very screen and
    // put the gate straight back up).
    if (!bottomMenuRevealed)
    {
        if (key == lwjgl::Keyboard::KEY_ESCAPE || key == lwjgl::Keyboard::KEY_RETURN)
            revealBottomMenu();
        return;
    }
#endif
    if (mc == nullptr || mc->gameSettings == nullptr || !mc->gameSettings->legacyUI)
        return;
#if !PLATFORM_PS2 && !PLATFORM_WII
    if (key == lwjgl::Keyboard::KEY_UP)
    {
        moveLegacySelection(-1);
        return;
    }
    if (key == lwjgl::Keyboard::KEY_DOWN)
    {
        moveLegacySelection(1);
        return;
    }
    if (key == lwjgl::Keyboard::KEY_RETURN)
        activateLegacySelection();
#endif
}

void GuiMainMenu::syncLegacySelection()
{
    if (selectedControlIndex < 0 || selectedControlIndex >= static_cast<int_t>(controlList.size()) ||
        controlList[selectedControlIndex] == nullptr || !controlList[selectedControlIndex]->enabled ||
        !controlList[selectedControlIndex]->enabled2)
    {
        selectedControlIndex = legacyFirstSelectableButton(controlList);
    }
    legacyApplyMenuSelection(controlList, hoveredControlIndex >= 0 ? -1 : selectedControlIndex);
}

void GuiMainMenu::moveLegacySelection(int_t direction)
{
    if (hoveredControlIndex >= 0)
        return;
    syncLegacySelection();
    const int_t previous = selectedControlIndex;
    selectedControlIndex = legacyNextSelectableButton(controlList, selectedControlIndex, direction);
    legacyApplyMenuSelection(controlList, selectedControlIndex);
    if (selectedControlIndex != previous && mc != nullptr && mc->sndManager != nullptr)
        mc->sndManager->playSoundFX("random.focus", 1.0f, 1.0f);
}

void GuiMainMenu::activateLegacySelection()
{
    syncLegacySelection();
    const int_t targetIndex = hoveredControlIndex >= 0 ? hoveredControlIndex : selectedControlIndex;
    if (targetIndex < 0 || targetIndex >= static_cast<int_t>(controlList.size()))
        return;
    if (mc != nullptr && mc->sndManager != nullptr)
        mc->sndManager->playSoundFX("random.action", 1.0f, 1.0f);
    actionPerformed(controlList[targetIndex]);
}

void GuiMainMenu::initGui()
{
    // arregla el bug del f3 al salir del mundo
    // sin desactivar el debuginfo, ahora al llegar aqui lo desactiva
    if (mc->gameSettings->showDebugInfo)
        mc->gameSettings->showDebugInfo = false;

    if (viewportTexture >= 0)
        mc->renderEngine->deleteTexture(viewportTexture);
    viewportTexture = -1;
    // The panorama asset is a property of the data pack, not of the UI
    // style: the bottom panel draws it as its backdrop in BOTH styles
    // (see drawTitleBottomHalf), while the top half still keys on legacyUI
    // at its own draw site.
    legacyPanoramaAvailable = mc->renderEngine != nullptr &&
        mc->renderEngine->hasResource(legacyPanoramaResourcePath());
#if !PLATFORM_PS2 && !PLATFORM_WII && !PLATFORM_3DS
    if (!legacyPanoramaAvailable)
    {
        BufferedImage viewportImage(256, 256);
        viewportTexture = mc->renderEngine->allocateAndSetupTexture(&viewportImage);
    }
#endif

    time_t t = time(nullptr);
    struct tm *now = localtime(&t);
    if (now != nullptr)
    {
        const int month = now->tm_mon + 1;
        const int day = now->tm_mday;
        if      (month == 11 && day == 9)  splashText = "Happy birthday, ez!";
        else if (month == 6  && day == 1)  splashText = "Happy birthday, Notch!";
        else if (month == 12 && day == 24) splashText = "Merry X-mas!";
        else if (month == 1  && day == 1)  splashText = "Happy new year!";
    }

    if (mc != nullptr && mc->sndManager != nullptr)
        mc->sndManager->playRandomMusicIfReady();

    StringTranslate *tr = StringTranslate::getInstance();
    if (mc->gameSettings != nullptr && mc->gameSettings->legacyUI)
    {
        legacyCreateMainMenuButtons(controlList, multiplayerButton, width, height, mc->hideQuitButton);
#if PLATFORM_3DS
        // Dual-screen title, +30% edition (owner call): the column and the
        // player card both grow ~30%. The shared layout's sizes are tuned
        // for the 172 px single-screen column, so the buttons are resized
        // and re-laid here from scratch -- the column claims the right
        // block, centred vertically, and the card takes the strip that
        // leaves (drawTitleBottomHalf sizes it the same way). Hit tests and
        // hover read the buttons' own rects, so they follow everything.
        const int_t menuButtonCount = legacyMainMenuButtonCount(mc->hideQuitButton);
        const LegacyMainMenuLayout menuLayout = legacyMainMenuLayout(width, height, menuButtonCount);
        const int_t columnW = menuLayout.buttonWidth * 13 / 10;
        const int_t columnH = menuLayout.buttonHeight * 13 / 10;
        const int_t columnSpacing = std::max(2, columnH / 5);
        const int_t columnHeight = menuButtonCount * columnH + (menuButtonCount - 1) * columnSpacing;
        const int_t columnTop = (height - columnHeight) / 2;
        const int_t columnX = width - columnW - 5;
        for (int i = 0; i < static_cast<int>(controlList.size()); ++i)
        {
            auto *legacy = static_cast<LegacyGuiButton *>(controlList[i]);
            legacy->xPosition = columnX;
            legacy->yPosition = columnTop + i * (columnH + columnSpacing);
            legacy->setButtonSize(columnW, columnH);
        }
#endif
        selectedControlIndex = -1;
        hoveredControlIndex = -1;
        syncLegacySelection();
#if !PLATFORM_PS2
        if (mc->session == nullptr && multiplayerButton != nullptr)
            multiplayerButton->enabled = false;
#endif
        return;
    }

    const int_t y = height / 4 + 40;
    controlList.push_back(new GuiButton(1, width / 2 - 100, y, tr->translateKey("menu.singleplayer")));
    controlList.push_back(multiplayerButton = new GuiButton(2, width / 2 - 100, y + 24, tr->translateKey("menu.multiplayer")));
    controlList.push_back(new GuiButton(3, width / 2 - 100, y + 48, uiText("Mods")));
    controlList.push_back(new GuiButton(6, width / 2 - 100, y + 72, "Skins"));
#if PLATFORM_3DS
    // Below the options row: the legacy column is this port's real menu on
    // the 3DS, so this Java-style row only has to exist and fit the panel.
    controlList.push_back(new GuiButton(7, width / 2 - 100, y + 120, uiText("QR Download")));
#endif

    if (mc->hideQuitButton)
    {
        controlList.push_back(new GuiButton(0, width / 2 - 100, y + 96, tr->translateKey("menu.options")));
    }
    else
    {
        controlList.push_back(new GuiButton(0, width / 2 - 100, y + 96, 98, 20, tr->translateKey("menu.options")));
        controlList.push_back(new GuiButton(4, width / 2 + 2, y + 96, 98, 20, tr->translateKey("menu.quit")));
    }

    controlList.push_back(new GuiButtonLanguage(5, width / 2 - 124, y + 96));
#if !PLATFORM_PS2
    if (mc->session == nullptr)
        multiplayerButton->enabled = false;
#endif
}

void GuiMainMenu::actionPerformed(GuiButton *button)
{
#if PLATFORM_3DS
    // An activation can arrive here before keyTyped sees the key at all --
    // the Java-UI navigation path runs activateKeyboardSelection straight
    // from handleKeyboardInput -- so the "Press START Button" gate also
    // holds here: the hidden button runs nothing, the tap/key reveals.
    if (!bottomMenuRevealed)
    {
        revealBottomMenu();
        return;
    }
#endif
    if (button->id == 0)
    {
        if (mc->gameSettings != nullptr && mc->gameSettings->legacyUI)
            mc->displayGuiScreen(new LegacyHelpOptions(this, mc->gameSettings));
        else
            mc->displayGuiScreen(new GuiOptions(this, mc->gameSettings));
    }
    if (button->id == 5)
    {
        if (mc->gameSettings != nullptr && mc->gameSettings->legacyUI)
            mc->displayGuiScreen(new LegacyLanguageOptions(this, mc->gameSettings));
        else
            mc->displayGuiScreen(new GuiLanguage(this, mc->gameSettings));
    }
    if (button->id == 1)
    {
        if (mc->gameSettings != nullptr && mc->gameSettings->legacyUI)
            mc->displayGuiScreen(new LegacyPlayGameScreen(this));
        else
            mc->displayGuiScreen(new GuiSelectWorld(this));
    }
    if (button->id == 2) mc->displayGuiScreen(new GuiMultiplayer(this));
    if (button->id == 3) mc->displayGuiScreen(new GuiMods(this));
    if (button->id == 6) mc->displayGuiScreen(new GuiSkinSelector(this));
#if PLATFORM_3DS
    if (button->id == 7) mc->displayGuiScreen(new GuiQrDownload(this));
#endif
    if (button->id == 4) mc->shutdown();
}

void GuiMainMenu::drawPanorama(int_t, int_t, float_t partialTick, float_t aspectRatio)
{
    Tessellator *tess = &Tessellator::instance;

    renderMatrixMode(RenderMatrixMode::Projection);
    renderPushMatrix();
    renderLoadIdentity();
    setPerspective(120.0f, aspectRatio, 0.05f, 10.0f);
    renderMatrixMode(RenderMatrixMode::ModelView);
    renderPushMatrix();
    renderLoadIdentity();
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    renderRotate(180.0f, 1.0f, 0.0f, 0.0f);
    renderEnable(RenderCapability::Blend);
    renderDisable(RenderCapability::AlphaTest);
    renderDisable(RenderCapability::CullFace);
    renderDepthMask(false);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);

    // Platform policy keeps the desktop/Wii visual accumulation while letting
    // PS2 use a much cheaper 2x2 cubemap accumulation (24 draws instead of 384).
    const int_t sampleGrid = ClientPlatformPolicy::panoramaSampleGrid();
    for (int_t sample = 0; sample < sampleGrid * sampleGrid; ++sample)
    {
        renderPushMatrix();
        const float_t offsetX = ((static_cast<float_t>(sample % sampleGrid) / sampleGrid) - 0.5f) / 64.0f;
        const float_t offsetY = ((static_cast<float_t>(sample / sampleGrid) / sampleGrid) - 0.5f) / 64.0f;
        renderTranslate(offsetX, offsetY, 0.0f);
        renderRotate(MathHelper::sin((static_cast<float_t>(panoramaTimer) + partialTick) / 400.0f) * 25.0f + 20.0f,
            1.0f, 0.0f, 0.0f);
        renderRotate(-(static_cast<float_t>(panoramaTimer) + partialTick) * 0.1f, 0.0f, 1.0f, 0.0f);

        for (int_t face = 0; face < 6; ++face)
        {
            renderPushMatrix();
            if (face == 1) renderRotate(90.0f, 0.0f, 1.0f, 0.0f);
            if (face == 2) renderRotate(180.0f, 0.0f, 1.0f, 0.0f);
            if (face == 3) renderRotate(-90.0f, 0.0f, 1.0f, 0.0f);
            if (face == 4) renderRotate(90.0f, 1.0f, 0.0f, 0.0f);
            if (face == 5) renderRotate(-90.0f, 1.0f, 0.0f, 0.0f);

            mc->renderEngine->bindTexture(mc->renderEngine->getTexture(
                "/title/bg/panorama" + std::to_string(face) + ".png"));
            tess->startDrawingQuads();
            tess->setColorRGBA_I(0xffffff, 255 / (sample + 1));
            tess->addVertexWithUV(-1.0, -1.0, 1.0, 0.0, 0.0);
            tess->addVertexWithUV(1.0, -1.0, 1.0, 1.0, 0.0);
            tess->addVertexWithUV(1.0, 1.0, 1.0, 1.0, 1.0);
            tess->addVertexWithUV(-1.0, 1.0, 1.0, 0.0, 1.0);
            tess->draw();
            renderPopMatrix();
        }

        renderPopMatrix();
        renderColorMask(true, true, true, false);
    }

    tess->setTranslation(0.0, 0.0, 0.0);
    renderColorMask(true, true, true, true);
    renderMatrixMode(RenderMatrixMode::Projection);
    renderPopMatrix();
    renderMatrixMode(RenderMatrixMode::ModelView);
    renderPopMatrix();
    renderDepthMask(true);
    renderEnable(RenderCapability::CullFace);
    renderEnable(RenderCapability::AlphaTest);
    renderEnable(RenderCapability::DepthTest);
}

void GuiMainMenu::rotateAndBlurSkybox(float_t, bool copyFramebuffer)
{
    if (viewportTexture < 0)
        return;

    mc->renderEngine->bindTexture(viewportTexture);
    if (copyFramebuffer && !renderCopyFramebufferToBoundTexture(0, 0, 256, 256))
        return;

    renderTextureParameters(true, false, false);
    renderEnable(RenderCapability::Blend);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
    renderColorMask(true, true, true, false);

    Tessellator *tess = &Tessellator::instance;
    tess->startDrawingQuads();
    const int_t samples = 3;
    for (int_t i = 0; i < samples; ++i)
    {
        tess->setColorRGBA_F(1.0f, 1.0f, 1.0f, 1.0f / static_cast<float_t>(i + 1));
        const float_t offset = static_cast<float_t>(i - samples / 2) / 256.0f;
        tess->addVertexWithUV(width, height, zLevel, 0.0f + offset, 0.0f);
        tess->addVertexWithUV(width, 0, zLevel, 1.0f + offset, 0.0f);
        tess->addVertexWithUV(0, 0, zLevel, 1.0f + offset, 1.0f);
        tess->addVertexWithUV(0, height, zLevel, 0.0f + offset, 1.0f);
    }
    tess->draw();
    renderColorMask(true, true, true, true);
    renderDisable(RenderCapability::Blend);
}

void GuiMainMenu::renderSkybox(int_t mouseX, int_t mouseY, float_t partialTick)
{
    bool canBlur = viewportTexture >= 0;

    if (canBlur)
    {
        renderViewport(0, 0, 256, 256);
        drawPanorama(mouseX, mouseY, partialTick, 1.0f);
        mc->renderEngine->bindTexture(viewportTexture);
        canBlur = renderCopyFramebufferToBoundTexture(0, 0, 256, 256);
    }

    if (!canBlur)
    {
        renderViewport(0, 0, mc->displayWidth, mc->displayHeight);
        const float_t aspect = mc->displayHeight > 0
            ? static_cast<float_t>(mc->displayWidth) / static_cast<float_t>(mc->displayHeight)
            : 1.0f;
        drawPanorama(mouseX, mouseY, partialTick, aspect);
        return;
    }

    rotateAndBlurSkybox(partialTick, false);
    for (int_t i = 1; i < 8; ++i)
        rotateAndBlurSkybox(partialTick);

    renderViewport(0, 0, mc->displayWidth, mc->displayHeight);
    mc->renderEngine->bindTexture(viewportTexture);
    renderTextureParameters(true, false, false);

    Tessellator *tess = &Tessellator::instance;
    tess->startDrawingQuads();
    const float_t scale = width > height ? 120.0f / static_cast<float_t>(width)
                                         : 120.0f / static_cast<float_t>(height);
    const float_t v = static_cast<float_t>(height) * scale / 256.0f;
    const float_t u = static_cast<float_t>(width) * scale / 256.0f;
    tess->setColorRGBA_F(1.0f, 1.0f, 1.0f, 1.0f);
    tess->addVertexWithUV(0.0, height, zLevel, 0.5f - v, 0.5f + u);
    tess->addVertexWithUV(width, height, zLevel, 0.5f - v, 0.5f - u);
    tess->addVertexWithUV(width, 0.0, zLevel, 0.5f + v, 0.5f - u);
    tess->addVertexWithUV(0.0, 0.0, zLevel, 0.5f + v, 0.5f + u);
    tess->draw();
}

void GuiMainMenu::drawTitleArt(int_t mouseX, int_t mouseY, float_t partialTick)
{
    const bool legacyUi = mc->gameSettings != nullptr && mc->gameSettings->legacyUI;
    const bool legacyPanoramaDrawn = legacyUi && legacyPanoramaAvailable &&
        legacyDrawPanorama(mc, width, height, legacyScenePanoramaTimer(), partialTick, zLevel);

    if (!legacyPanoramaDrawn)
        renderSkybox(mouseX, mouseY, partialTick);

    if (legacyPanoramaDrawn)
    {
        // Legacy Console uses the panorama itself as the soft background. Keep
        // only a subtle darkening layer so the menu remains readable without
        // the expensive Java cubemap blur/white wash.
        drawGradientRect(0, 0, width, height,
            static_cast<int_t>(0x18000000u), static_cast<int_t>(0x50000000u));
    }
    else
    {
        drawGradientRect(0, 0, width, height, static_cast<int_t>(0x80ffffffu), 0x00ffffff);
        drawGradientRect(0, 0, width, height, 0x00000000, static_cast<int_t>(0x80000000u));
    }

    LegacyUiRect legacyTitleRect{};
    bool legacyTitleDrawn = false;
    if (legacyUi)
    {
        const LegacySceneLayout scene = legacySceneLayout(width, height);
        LegacyMainMenuLayout titleLayout{};
        titleLayout.titleY = scene.titleY;
        titleLayout.titleMaxWidth = scene.titleMaxWidth;
        titleLayout.titleMaxHeight = scene.titleMaxHeight;
#if PLATFORM_3DS
        // The top panel carries only banner + splash + hint, so the banner
        // can afford to sit lower and run a tenth larger than the shared
        // scene metric tuned for single-screen titles -- it reads less
        // cramped against the top edge, and the splash (anchored to the
        // banner's bottom edge) follows it down automatically.
        titleLayout.titleY += height / 10;
        titleLayout.titleMaxWidth = titleLayout.titleMaxWidth * 11 / 10;
        titleLayout.titleMaxHeight = titleLayout.titleMaxHeight * 11 / 10;
#endif
        legacyTitleDrawn = legacyDrawTitleTexture(mc, titleLayout, width, zLevel, &legacyTitleRect);
    }

    Tessellator *tess = &Tessellator::instance;
    if (!legacyTitleDrawn)
    {
        const int_t logoWidth = 274;
        const int_t logoX = width / 2 - logoWidth / 2;
        const int_t logoY = 30;
        renderBindTexture(mc->renderEngine->getTexture("/title/mclogo.png"));
        renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        if (updateCounter < 1.0E-4f)
        {
            drawTexturedModalRect(logoX, logoY, 0, 0, 99, 44);
            drawTexturedModalRect(logoX + 99, logoY, 129, 0, 27, 44);
            drawTexturedModalRect(logoX + 125, logoY, 126, 0, 3, 44);
            drawTexturedModalRect(logoX + 128, logoY, 99, 0, 26, 44);
            drawTexturedModalRect(logoX + 155, logoY, 0, 45, 155, 44);
        }
        else
        {
            drawTexturedModalRect(logoX, logoY, 0, 0, 155, 44);
            drawTexturedModalRect(logoX + 155, logoY, 0, 45, 155, 44);
        }
    }

    tess->setColorOpaque_I(0xffffff);
    const float_t splashScaleRaw = 1.8f - MathHelper::abs(MathHelper::sin(
        (static_cast<float_t>(System::currentTimeMillis() % 1000LL) / 1000.0f) * 3.1415927f * 2.0f) * 0.1f);
    float_t splashScale = (splashScaleRaw * 100.0f) /
        static_cast<float_t>(fontRenderer->getStringWidth(splashText) + 32);

    // The splash normalises itself to a constant width, a rule tuned against the
    // 274 px vanilla logo. The Legacy banner instead scales with the screen, so the
    // splash kept its desktop size while the banner shrank and ended up written
    // across the title: it covered 54 % of the banner at a small resolution against
    // vanilla's 43 %. Scaling it with the banner, and insetting its anchor by the
    // same factor, holds that overlap constant at every resolution.
    constexpr float_t VANILLA_LOGO_WIDTH = 274.0f;
    // Vanilla's proportion, trimmed: the Legacy banner is wide and thin, so a splash
    // sized like vanilla's reads as heavier over it than over the chunky Java logo.
    constexpr float_t SPLASH_TITLE_TRIM = 0.85f;
#ifdef PS2_PLATFORM
    constexpr float_t SPLASH_ANCHOR_INSET = 28.0f;
#else
    constexpr float_t SPLASH_ANCHOR_INSET = 34.0f;
#endif
    float_t titleFactor = 1.0f;
    if (legacyTitleDrawn && legacyTitleRect.width > 0)
    {
        titleFactor = (static_cast<float_t>(legacyTitleRect.width) / VANILLA_LOGO_WIDTH) *
            SPLASH_TITLE_TRIM;
        splashScale *= titleFactor;
    }

#if PLATFORM_3DS
    // The dual-screen title makes the splash its own centred line of art
    // under the banner, so it stops normalising itself to the text length
    // (a long splash rendered at barely two thirds of a short one) and
    // takes the fixed TITLE_TEXT_SCALE instead -- the same size class as
    // the hint row's icons. Only vanilla's gentle breathing pulse is kept.
    splashScale = TITLE_TEXT_SCALE * splashScaleRaw / 1.8f;
#endif
    const float_t splashWidth = static_cast<float_t>(fontRenderer->getStringWidth(splashText)) * splashScale;
    float_t splashCenterX = legacyTitleDrawn
        ? static_cast<float_t>(legacyTitleRect.x + legacyTitleRect.width) - SPLASH_ANCHOR_INSET * titleFactor
        : static_cast<float_t>(width / 2 + 90);
#if PLATFORM_3DS
    // Dead-centre: with the tilt gone (see the draw below) the splash reads
    // as a straight title line, and the off-centre anchor only made sense
    // while it hung off the banner's corner.
    splashCenterX = static_cast<float_t>(width / 2);
    // The 3DS title splits the art across the two panels: this half carries
    // only the banner and the splash over the panorama. Vanilla hangs the
    // splash off the banner's lower-right corner; the first cut at a 3DS
    // placement dropped it a fixed 14 px below the banner instead -- but
    // with the menu half moved to the touch screen nothing fills the space
    // under the banner anymore, so those 14 px still read as the splash
    // being glued to the title. Centre it in the band the split leaves open
    // -- banner above, the "A Select" hint pinned to the bottom edge below
    // (wherever legacyHintRowY puts it) -- so it sits clearly between the
    // two and still tracks the banner at any title scale.
    const float_t splashBandTop = legacyTitleDrawn
        ? static_cast<float_t>(legacyTitleRect.y + legacyTitleRect.height)
        : 74.0f; // Java logo fallback: drawn at y=30, 44 px tall
    const float_t splashBandBottom = static_cast<float_t>(legacyHintRowY(height));
    // drawCenteredString below anchors the glyphs 8 local units above the
    // translate origin, so the visible text sits above splashCenterY; lift
    // the origin by half a glyph to truly centre the splash in the band.
    const float_t splashCenterY = (splashBandTop + splashBandBottom) * 0.5f +
        (splashBandBottom - splashBandTop) * 0.10f + 4.0f * splashScale;
#else
    const float_t splashCenterY = legacyTitleDrawn
        ? static_cast<float_t>(legacyTitleRect.y + legacyTitleRect.height - 2)
        : 70.0f;
#endif
    // A narrow window would otherwise push the splash past the edge of the screen.
    if (splashCenterX + splashWidth * 0.5f > static_cast<float_t>(width - 4))
        splashCenterX = static_cast<float_t>(width - 4) - splashWidth * 0.5f;
    if (splashCenterX - splashWidth * 0.5f < 4.0f)
        splashCenterX = 4.0f + splashWidth * 0.5f;

    renderPushMatrix();
    renderTranslate(splashCenterX, splashCenterY, 0.0f);
#if !PLATFORM_3DS
    // Vanilla's -20 tilt; the dual-screen 3DS title hangs its splash as a
    // straight centred line instead.
    renderRotate(-20.0f, 0.0f, 0.0f, 1.0f);
#endif
    renderScale(splashScale, splashScale, splashScale);
    drawCenteredString(fontRenderer, splashText, 0, -8, 0xffff00);
    renderPopMatrix();

#if PLATFORM_3DS
    // New 3DS Edition's select prompt, along the bottom edge of the top
    // panel where it points down at the menu the touch screen now holds.
    // It appears only once START has lifted the gate (bottomMenuRevealed):
    // before that there is nothing to select on either panel, and a prompt
    // naming a button that does nothing yet is exactly what the gate exists
    // to avoid.
    if (bottomMenuRevealed)
    {
        // "(A) Select", with the A drawn as the 3DS face-button glyph: a
        // ring around the letter, done procedurally with the tessellator
        // so it never depends on a packed icon asset (the keyboard-icon
        // path controlIconTexture takes on this platform renders a flat
        // key, and with no icon at all the row degrades to "[A] Select").
        const std::string action = uiText("Select");
        constexpr float_t GLYPH_RADIUS_OUT = 7.0f;
        constexpr float_t GLYPH_RADIUS_IN = 5.6f;
        constexpr int_t GLYPH_TEXT_GAP = 3;
        const int_t glyphSize = static_cast<int_t>(GLYPH_RADIUS_OUT * 2.0f);
        const int_t textWidth = fontRenderer->getStringWidth(action);
        const int_t rowY = legacyHintRowY(height);
        const int_t left = (width - (glyphSize + GLYPH_TEXT_GAP + textWidth)) / 2;
        const float_t glyphCenterX = static_cast<float_t>(left) + GLYPH_RADIUS_OUT;
        const float_t glyphCenterY = static_cast<float_t>(rowY) + 4.0f;

        // The official console UI sits its hint rows on a translucent black
        // strip; same here, so the prompt reads over any panorama. The
        // strip runs flush to the panel's bottom edge.
        drawRect(0, rowY - 3, width, height, static_cast<int_t>(0x88000000u));

        renderDisable(RenderCapability::Texture2D);
        Tessellator *tess = &Tessellator::instance;
        tess->setColorOpaque_I(0xf0f0f0);
        tess->startDrawingQuads();
        constexpr int GLYPH_SEGMENTS = 12;
        for (int i = 0; i < GLYPH_SEGMENTS; ++i)
        {
            const float a0 = (static_cast<float>(i) * 6.2831853f) / GLYPH_SEGMENTS;
            const float a1 = (static_cast<float>(i + 1) * 6.2831853f) / GLYPH_SEGMENTS;
            const float c0 = MathHelper::cos(a0), s0v = MathHelper::sin(a0);
            const float c1 = MathHelper::cos(a1), s1v = MathHelper::sin(a1);
            tess->addVertex(glyphCenterX + c0 * GLYPH_RADIUS_OUT, glyphCenterY + s0v * GLYPH_RADIUS_OUT, zLevel);
            tess->addVertex(glyphCenterX + c1 * GLYPH_RADIUS_OUT, glyphCenterY + s1v * GLYPH_RADIUS_OUT, zLevel);
            tess->addVertex(glyphCenterX + c1 * GLYPH_RADIUS_IN, glyphCenterY + s1v * GLYPH_RADIUS_IN, zLevel);
            tess->addVertex(glyphCenterX + c0 * GLYPH_RADIUS_IN, glyphCenterY + s0v * GLYPH_RADIUS_IN, zLevel);
        }
        tess->draw();
        renderEnable(RenderCapability::Texture2D);

        const std::string letter = "A";
        fontRenderer->drawStringWithShadow(letter,
            static_cast<int_t>(glyphCenterX) - fontRenderer->getStringWidth(letter) / 2,
            rowY, 0xf0f0f0);
        fontRenderer->drawStringWithShadow(action,
            left + glyphSize + GLYPH_TEXT_GAP, rowY, 0xf0f0f0);
    }
#endif
}

// Footer (version/copyright or the Legacy hint row) plus the button column
// itself: the last thing every platform's title screen draws, in whichever
// space width/height currently describe (the whole screen, or the 3DS
// bottom panel).
void GuiMainMenu::drawMenuFooter(bool legacyUi, int_t mouseX, int_t mouseY, float_t partialTick)
{
    if (!legacyUi)
    {
        drawString(fontRenderer, "Minecraft 1.2.5", 2, height - 10, 0xffffff);
        const std::string copyright = "Copyright Mojang AB. Do not distribute!";
        drawString(fontRenderer, copyright, width - fontRenderer->getStringWidth(copyright) - 2, height - 10, 0xffffff);
    }
    else
    {
        syncLegacySelection();
        drawLegacyMenuHints(mc, width, height, false);
    }

    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}

#if PLATFORM_3DS
void GuiMainMenu::drawTitleBottomHalf(int_t mouseX, int_t mouseY, float_t partialTick)
{
    const bool legacyUi = mc->gameSettings != nullptr && mc->gameSettings->legacyUI;

    // The panel's backdrop does NOT key on legacyUi: with the Java-style UI
    // the cubemap skybox owns the top half, and the panel would otherwise
    // sit on a bare gradient pair. legacyDrawPanorama is one scrolling
    // texture pass of five quads -- the cubemap path accumulates
    // sampleGrid^2 x 6 faces and is far too much to pay twice per frame on
    // an Old 3DS -- so it is the only affordable backdrop for a second
    // surface, used for both styles whenever the asset exists. Without the
    // asset the panel falls back to the plain gradient pair.
    const bool legacyPanoramaDrawn = legacyPanoramaAvailable &&
        legacyDrawPanorama(mc, width, height, legacyScenePanoramaTimer(), partialTick, zLevel);

    if (legacyPanoramaDrawn)
    {
        drawGradientRect(0, 0, width, height,
            static_cast<int_t>(0x18000000u), static_cast<int_t>(0x50000000u));
    }
    else
    {
        drawGradientRect(0, 0, width, height, static_cast<int_t>(0x80ffffffu), 0x00ffffff);
        drawGradientRect(0, 0, width, height, 0x00000000, static_cast<int_t>(0x80000000u));
    }

    // "Press START Button": everything else on this panel stays hidden
    // until START (ESC), A (RETURN) or a touch wakes it -- the gate the
    // other input paths (keyTyped, mouseClicked, actionPerformed) enforce.
    if (!bottomMenuRevealed)
    {
        // Same size as the splash on the top panel (TITLE_TEXT_SCALE): the
        // gate is the panel's one line of art, and the two reads should
        // match. The translate/scale pair reproduces the splash draw's
        // anchoring, with the +4-unit lift that centres the scaled glyphs.
        renderPushMatrix();
        renderTranslate(static_cast<float_t>(width / 2),
            static_cast<float_t>(height) * 0.5f + 4.0f * TITLE_TEXT_SCALE, 0.0f);
        renderScale(TITLE_TEXT_SCALE, TITLE_TEXT_SCALE, TITLE_TEXT_SCALE);
        drawCenteredString(fontRenderer, uiText("Press START Button"), 0, -8, 0xffffff);
        renderPopMatrix();
        return;
    }

    // The player card is part of the Legacy console title composition --
    // the slid column, the strip, the name-over-preview layout only read
    // together. The Java-style UI keeps the panel to its own menu, so the
    // card hides with it.
    if (legacyUi)
    {
        // The card sizes from the strip the Legacy column leaves on the
        // left (see BOTTOM_COLUMN_SHIFT_X, applied in initGui) and takes
        // 150% of its first cut (~46% of the panel height), centred
        // vertically -- the name glyph plus a gap ride above the preview
        // (12 = the 8 px glyph + 4 px gap).
        const LegacyMainMenuLayout cardLayout = legacyMainMenuLayout(width, height,
            legacyMainMenuButtonCount(mc->hideQuitButton));
        // The strip left of the +30% column (see initGui); the card takes
        // 130% of its first cut and centres in it.
        const int_t columnX = width - cardLayout.buttonWidth * 13 / 10 - 5;
        const int_t cardStripWidth = columnX - 8;
        const float_t cardWidth = static_cast<float_t>(std::max<int_t>(12, cardStripWidth * 87 / 100));
        const float_t cardHeight = cardWidth * 2.0f;
        const float_t cardX = (static_cast<float_t>(cardStripWidth) - cardWidth) * 0.5f;
        const float_t cardNameY = (static_cast<float_t>(height) - cardHeight - 12.0f) * 0.5f;
        const float_t cardPreviewY = cardNameY + 12.0f;
        const SkinEntry *skin = SkinManager::getSkinById(SkinManager::getSelectedSkinId());
        GuiSkinSelector::drawSkinFrontPreview(mc, zLevel, skin,
            cardX, cardPreviewY, cardWidth, cardHeight, 1.0f);
        if (mc->session != nullptr)
        {
            const int_t nameWidth = fontRenderer->getStringWidth(mc->session->username);
            const int_t nameX = std::max<int_t>(2,
                static_cast<int_t>(cardX + cardWidth * 0.5f) - nameWidth / 2);
            drawString(fontRenderer, mc->session->username, nameX, static_cast<int_t>(cardNameY), 0xffffff);
        }
    }

    drawMenuFooter(legacyUi, mouseX, mouseY, partialTick);
}

void GuiMainMenu::revealBottomMenu()
{
    bottomMenuRevealed = true;
    if (mc != nullptr && mc->sndManager != nullptr)
        mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
}
#endif

void GuiMainMenu::mouseClicked(int_t x, int_t y, int_t button)
{
#if PLATFORM_3DS
    // A tap anywhere on the gated panel wakes the menu; once it is up, the
    // buttons take the click as usual.
    if (!bottomMenuRevealed)
    {
        revealBottomMenu();
        return;
    }
#endif
    GuiScreen::mouseClicked(x, y, button);
}

void GuiMainMenu::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    const bool legacyUi = mc->gameSettings != nullptr && mc->gameSettings->legacyUI;
    // Hover lives in the bottom panel's canvas, so it is only meaningful
    // against the button column there.
    hoveredControlIndex = legacyUi ? legacyHoveredSelectableButton(controlList, mouseX, mouseY) : -1;
    if (hoveredControlIndex >= 0)
        selectedControlIndex = hoveredControlIndex;

#if PLATFORM_3DS
    // Dual-screen split. The banner half owns the top LCD and lays itself
    // out in top-screen pixels (the projection EntityRenderer installed for
    // the world/HUD covers the full display, and on this port the GUI scale
    // resolves to 1, so that space is exactly displayWidth x displayHeight);
    // the menu half owns the bottom panel's 320x240 canvas, which is the
    // canvas setWorldAndResolution left in width/height. Every layout
    // helper reads those two members, so they are swapped to each half's
    // space and restored around it.
    const int_t panelWidth = width;
    const int_t panelHeight = height;
    width = mc->displayWidth;
    height = mc->displayHeight;
    drawTitleArt(mouseX, mouseY, partialTick);
    width = panelWidth;
    height = panelHeight;

    // Begin splits the frame and hands out a freshly cleared panel, so the
    // half's backdrop lands on black. If the target cannot be allocated it
    // returns false and the menu draws on the top screen instead: the
    // 320-wide layout in the 400-wide projection leaves the right edge
    // short, but every coordinate still lines up with the input scaling.
    const bool bottomPanelPass = renderBottomPanelBegin();
    drawTitleBottomHalf(mouseX, mouseY, partialTick);
    if (bottomPanelPass)
        renderBottomPanelEnd();
#else
    drawTitleArt(mouseX, mouseY, partialTick);
    drawMenuFooter(legacyUi, mouseX, mouseY, partialTick);
#endif
}
