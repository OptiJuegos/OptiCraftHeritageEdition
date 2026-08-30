#include "GuiMainMenu.h"
#include "platform/Log.h"
#include "platform/PlatformConfig.h"
#include "java/String.h"
#include "java/BufferedImage.h"
#include "GuiButton.h"
#include "GuiButtonLanguage.h"
#include "GuiLanguage.h"
#include "GuiOptions.h"
#include "GuiSelectWorld.h"
#include "GuiMultiplayer.h"
#include "GuiTexturePacks.h"
#include "StringTranslate.h"
#include "Tessellator.h"
#include "MathHelper.h"
#include "FontRenderer.h"
#include "RenderEngine.h"
#include "Minecraft.h"
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
{
    try
    {
        std::vector<std::string> lines;
        std::unique_ptr<std::istream> splashStream;
#ifndef PS2_PLATFORM
        const std::string resolved = GameResources::resolve("/title/splashes.txt");
        std::unique_ptr<std::ifstream> fileStream(new std::ifstream(resolved));
        if (fileStream && *fileStream)
            splashStream = std::move(fileStream);
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
}

bool GuiMainMenu::doesGuiPauseGame()
{
    return false;
}

void GuiMainMenu::keyTyped(char_t, int_t)
{
}

void GuiMainMenu::initGui()
{
    if (viewportTexture >= 0)
        mc->renderEngine->deleteTexture(viewportTexture);
    viewportTexture = -1;
#if !PLATFORM_PS2 && !PLATFORM_WII
    BufferedImage viewportImage(256, 256);
    viewportTexture = mc->renderEngine->allocateAndSetupTexture(&viewportImage);
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
    const int_t y = height / 4 + 48;
    controlList.push_back(new GuiButton(1, width / 2 - 100, y, tr->translateKey("menu.singleplayer")));
    controlList.push_back(multiplayerButton = new GuiButton(2, width / 2 - 100, y + 24, tr->translateKey("menu.multiplayer")));
    controlList.push_back(new GuiButton(3, width / 2 - 100, y + 48, tr->translateKey("menu.mods")));

    if (mc->hideQuitButton)
    {
        controlList.push_back(new GuiButton(0, width / 2 - 100, y + 72, tr->translateKey("menu.options")));
    }
    else
    {
        controlList.push_back(new GuiButton(0, width / 2 - 100, y + 84, 98, 20, tr->translateKey("menu.options")));
        controlList.push_back(new GuiButton(4, width / 2 + 2, y + 84, 98, 20, tr->translateKey("menu.quit")));
    }

    controlList.push_back(new GuiButtonLanguage(5, width / 2 - 124, y + 84));
#if !PLATFORM_PS2
    if (mc->session == nullptr)
        multiplayerButton->enabled = false;
#endif
}

void GuiMainMenu::actionPerformed(GuiButton *button)
{
    if (button->id == 0) mc->displayGuiScreen(new GuiOptions(this, mc->gameSettings));
    if (button->id == 5) mc->displayGuiScreen(new GuiLanguage(this, mc->gameSettings));
    if (button->id == 1) mc->displayGuiScreen(new GuiSelectWorld(this));
    if (button->id == 2) mc->displayGuiScreen(new GuiMultiplayer(this));
    if (button->id == 3) mc->displayGuiScreen(new GuiTexturePacks(this));
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
    renderSkybox(mouseX, mouseY, partialTick);

    drawGradientRect(0, 0, width, height, static_cast<int_t>(0x80ffffffu), 0x00ffffff);
    drawGradientRect(0, 0, width, height, 0x00000000, static_cast<int_t>(0x80000000u));

    Tessellator *tess = &Tessellator::instance;
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

    tess->setColorOpaque_I(0xffffff);
#ifdef PS2_PLATFORM
    const float_t splashScaleRaw = 1.8f - MathHelper::abs(MathHelper::sin(
        (static_cast<float_t>(System::currentTimeMillis() % 1000LL) / 1000.0f) * 3.1415927f * 2.0f) * 0.1f);
    const float_t splashScale = (splashScaleRaw * 100.0f) /
        static_cast<float_t>(fontRenderer->getStringWidth(splashText) + 32);
    const float_t splashWidth = static_cast<float_t>(fontRenderer->getStringWidth(splashText)) * splashScale;
    float_t splashCenterX = static_cast<float_t>(width / 2 + 90);
    if (splashCenterX + splashWidth * 0.5f > static_cast<float_t>(width - 4))
        splashCenterX = static_cast<float_t>(width - 4) - splashWidth * 0.5f;
    if (splashCenterX - splashWidth * 0.5f < 4.0f)
        splashCenterX = 4.0f + splashWidth * 0.5f;
    fontRenderer->drawStringScaled(splashText, splashCenterX - splashWidth * 0.5f,
        70.0f - 8.0f * splashScale, splashScale, 0xffff00);
#else
    renderPushMatrix();
    renderTranslate(static_cast<float_t>(width / 2 + 90), 70.0f, 0.0f);
    renderRotate(-20.0f, 0.0f, 0.0f, 1.0f);
    float_t splashScale = 1.8f - MathHelper::abs(MathHelper::sin(
        (static_cast<float_t>(System::currentTimeMillis() % 1000LL) / 1000.0f) * 3.1415927f * 2.0f) * 0.1f);
    splashScale = (splashScale * 100.0f) /
        static_cast<float_t>(fontRenderer->getStringWidth(splashText) + 32);
    renderScale(splashScale, splashScale, splashScale);
    drawCenteredString(fontRenderer, splashText, 0, -8, 0xffff00);
    renderPopMatrix();
#endif

    drawString(fontRenderer, "Minecraft 1.2.5", 2, height - 10, 0xffffff);
    const std::string copyright = "Copyright Mojang AB. Do not distribute!";
    drawString(fontRenderer, copyright, width - fontRenderer->getStringWidth(copyright) - 2, height - 10, 0xffffff);

    GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
