#include "net/minecraft/src/UiStrings.h"
#include "GuiMainMenu.h"
#include "platform/Log.h"
#include "platform/PlatformConfig.h"
#include "java/String.h"
#include "java/BufferedImage.h"
#include "GuiButton.h"
#include "GuiCredits.h"
#ifdef PS2_PLATFORM
#include "ps2/storage/save/Ps2SaveStorage.h"
#include "GuiStorageMessage.h"
#endif
#include "GuiOptions.h"
#include "legacy/LegacyHelpOptions.h"
#include "GuiSelectWorld.h"
#include "GuiMultiplayer.h"
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
#include <fstream>
#include <memory>
#include <vector>
#include <ctime>
#include <cmath>
#include <cstdint>

namespace
{
Random g_mainMenuRand;

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
    if (viewportTexture >= 0)
        mc->renderEngine->deleteTexture(viewportTexture);
    viewportTexture = -1;
    legacyPanoramaAvailable = mc->gameSettings != nullptr && mc->gameSettings->legacyUI &&
        mc->renderEngine != nullptr && mc->renderEngine->hasResource(legacyPanoramaResourcePath());
#if !PLATFORM_PS2 && !PLATFORM_WII
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

    StringTranslate *tr = StringTranslate::getInstance();
    if (mc->gameSettings != nullptr && mc->gameSettings->legacyUI)
    {
        legacyCreateMainMenuButtons(controlList, multiplayerButton, width, height, mc->hideQuitButton);
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

    controlList.push_back(new GuiButton(0, width / 2 - 100, y + 48, 98, 20, tr->translateKey("menu.options")));
    controlList.push_back(new GuiButton(5, width / 2 + 2, y + 48, 98, 20, uiText("Credits")));
    if (!mc->hideQuitButton)
        controlList.push_back(new GuiButton(4, width / 2 - 100, y + 72, tr->translateKey("menu.quit")));
#if !PLATFORM_PS2
    if (mc->session == nullptr)
        multiplayerButton->enabled = false;
#endif
}

void GuiMainMenu::actionPerformed(GuiButton *button)
{
    if (button->id == 0)
    {
        if (mc->gameSettings != nullptr && mc->gameSettings->legacyUI)
            mc->displayGuiScreen(new LegacyHelpOptions(this, mc->gameSettings));
        else
            mc->displayGuiScreen(new GuiOptions(this, mc->gameSettings));
    }
    if (button->id == 5)
    {
        mc->displayGuiScreen(new GuiCredits(this, mc->gameSettings));
    }
    if (button->id == 1)
    {
#ifdef PS2_PLATFORM
        if (!Ps2SaveStorage::available(Ps2SaveStorage::target()))
        {
            mc->displayGuiScreen(new GuiStorageMessage(this, mc->gameSettings,
                uiText("World storage unavailable. Check the selected device in Game Options.")));
            return;
        }
#endif
        if (mc->gameSettings != nullptr && mc->gameSettings->legacyUI)
            mc->displayGuiScreen(new LegacyPlayGameScreen(this));
        else
            mc->displayGuiScreen(new GuiSelectWorld(this));
    }
    if (button->id == 2) mc->displayGuiScreen(new GuiMultiplayer(this));
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

void GuiMainMenu::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    const bool legacyUi = mc->gameSettings != nullptr && mc->gameSettings->legacyUI;
    hoveredControlIndex = legacyUi ? legacyHoveredSelectableButton(controlList, mouseX, mouseY) : -1;
    if (hoveredControlIndex >= 0)
        selectedControlIndex = hoveredControlIndex;
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
            drawTexturedModalRect(logoX + 155, logoY, 0, 45, PLATFORM_PS2 ? 119 : 155, 44);
        }
        else
        {
            drawTexturedModalRect(logoX, logoY, 0, 0, 155, 44);
            drawTexturedModalRect(logoX + 155, logoY, 0, 45, PLATFORM_PS2 ? 119 : 155, 44);
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

    const float_t splashWidth = static_cast<float_t>(fontRenderer->getStringWidth(splashText)) * splashScale;
    float_t splashCenterX = legacyTitleDrawn
        ? static_cast<float_t>(legacyTitleRect.x + legacyTitleRect.width) - SPLASH_ANCHOR_INSET * titleFactor
        : static_cast<float_t>(width / 2 + 90);
    const float_t splashCenterY = legacyTitleDrawn
        ? static_cast<float_t>(legacyTitleRect.y + legacyTitleRect.height - 2)
        : 70.0f;
    // A narrow window would otherwise push the splash past the edge of the screen.
    if (splashCenterX + splashWidth * 0.5f > static_cast<float_t>(width - 4))
        splashCenterX = static_cast<float_t>(width - 4) - splashWidth * 0.5f;
    if (splashCenterX - splashWidth * 0.5f < 4.0f)
        splashCenterX = 4.0f + splashWidth * 0.5f;

    renderPushMatrix();
    renderTranslate(splashCenterX, splashCenterY, 0.0f);
    renderRotate(-20.0f, 0.0f, 0.0f, 1.0f);
    renderScale(splashScale, splashScale, splashScale);
    drawCenteredString(fontRenderer, splashText, 0, -8, 0xffff00);
    renderPopMatrix();

    if (!legacyUi)
    {
        drawString(fontRenderer, "Minecraft 1.2.5", 2, height - 10, 0xffffff);
        const std::string copyright = "Copyright Mojang AB. Do not distribute!";
        drawString(fontRenderer, copyright, width - fontRenderer->getStringWidth(copyright) - 2, height - 10, 0xffffff);
    }
    else
    {
        syncLegacySelection();
        drawLegacyMenuHints(fontRenderer, width, height, false);
    }

    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
