#include "LoadingScreenRenderer.h"
#include "UiStrings.h"

#include <algorithm>
#include "FontRenderer.h"
#include "GameSettings.h"
#include "Minecraft.h"
#include "MinecraftError.h"
#include "RenderEngine.h"
#include "ScaledResolution.h"
#include "Tessellator.h"
#include "java/String.h"
#include "java/Arithmetic.h"
#include "java/System.h"
#include "platform/RenderAPI.h"
#include "legacy/LegacyMainMenuLayout.h"
#include "legacy/LegacyPanorama.h"
#include "legacy/LegacySceneState.h"
#include "legacy/LegacyUiAssets.h"
#include "pc/lwjgl/Display.h"

namespace
{
constexpr int_t LEGACY_PROGRESS_BAR_WIDTH = 320;
constexpr int_t LEGACY_PROGRESS_BAR_HEIGHT = 10;

void drawSolidQuad(Tessellator *tessellator, int_t left, int_t top, int_t right, int_t bottom,
    int_t color, int_t alpha)
{
    tessellator->startDrawingQuads();
    tessellator->setColorRGBA_I(color, alpha);
    tessellator->addVertex(left, bottom, 0.0);
    tessellator->addVertex(right, bottom, 0.0);
    tessellator->addVertex(right, top, 0.0);
    tessellator->addVertex(left, top, 0.0);
    tessellator->draw();
}

void drawLegacyFallbackBackground(Minecraft *mc, int_t width, int_t height)
{
    Tessellator *tessellator = &Tessellator::instance;
    renderBindTexture(mc->renderEngine->getTexture("/gui/background.png"));
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    constexpr float_t TILE = 32.0f;
    tessellator->startDrawingQuads();
    tessellator->setColorOpaque_I(0x303030);
    tessellator->addVertexWithUV(0.0, height, 0.0, 0.0, static_cast<float_t>(height) / TILE);
    tessellator->addVertexWithUV(width, height, 0.0, static_cast<float_t>(width) / TILE,
        static_cast<float_t>(height) / TILE);
    tessellator->addVertexWithUV(width, 0.0, 0.0, static_cast<float_t>(width) / TILE, 0.0);
    tessellator->addVertexWithUV(0.0, 0.0, 0.0, 0.0, 0.0);
    tessellator->draw();
}

void drawLegacyLoadingBackground(Minecraft *mc, int_t width, int_t height)
{
    bool drewPanorama = false;
#if !defined(PS2_PLATFORM)
    if (mc != nullptr && mc->renderEngine != nullptr &&
        mc->renderEngine->hasResource(legacyPanoramaResourcePath()))
    {
        drewPanorama = legacyDrawPanorama(mc, width, height, legacyScenePanoramaTimer(), 0.0f, 0.0f);
    }
#endif
    if (!drewPanorama)
        drawLegacyFallbackBackground(mc, width, height);

    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void drawLegacyLoadingTitle(Minecraft *mc, int_t width, int_t height)
{
    LegacyMainMenuLayout titleLayout{};
    titleLayout.titleY = std::max<int_t>(12, height / 12);
    titleLayout.titleMaxWidth = std::min<int_t>(360, width - 40);
    titleLayout.titleMaxHeight = std::min<int_t>(72, std::max<int_t>(40, height / 6));
    legacyDrawTitleTexture(mc, titleLayout, width, 1.0f, nullptr);
}

void drawLegacyLoadingProgress(Minecraft *mc, int_t width, int_t height, int_t progress,
    const std::string &message)
{
    drawLegacyLoadingBackground(mc, width, height);
    drawLegacyLoadingTitle(mc, width, height);

    FontRenderer *font = mc->fontRenderer;
    const int_t centerX = width / 2;
    const int_t contentY = std::max<int_t>(height / 2 - 12, 108);
    const int_t barWidth = std::max<int_t>(80, std::min<int_t>(LEGACY_PROGRESS_BAR_WIDTH, width - 48));
    const int_t barX = centerX - barWidth / 2;
    const int_t barY = contentY + 26;

    if (font != nullptr)
    {
        static const std::string heading = "Initializing world";
        constexpr float_t headingScale = 2.5f;
        const float_t headingWidth = static_cast<float_t>(font->getStringWidth(heading)) * headingScale;
        const float_t headingX = static_cast<float_t>(centerX) - headingWidth * 0.5f;
        const float_t headingY = static_cast<float_t>(contentY - 22);
        font->drawStringScaled(heading, headingX + 1.0f, headingY + 1.0f, headingScale, 0x202020);
        font->drawStringScaled(heading, headingX, headingY, headingScale, 0xffffff);

        if (!message.empty())
            font->drawStringWithShadow(message, barX, barY - 12, 0xffffff);
    }

    if (progress >= 0)
    {
        const int_t clamped = std::max<int_t>(0, std::min<int_t>(100, progress));
        const int_t filledWidth = barWidth * clamped / 100;

        renderDisable(RenderCapability::Texture2D);
        drawSolidQuad(&Tessellator::instance, barX - 1, barY - 1,
            barX + barWidth + 1, barY + LEGACY_PROGRESS_BAR_HEIGHT + 1,
            0x303030, 255);
        drawSolidQuad(&Tessellator::instance, barX, barY,
            barX + barWidth, barY + LEGACY_PROGRESS_BAR_HEIGHT,
            0x8a8a8a, 255);
        if (filledWidth > 0)
            drawSolidQuad(&Tessellator::instance, barX, barY,
                barX + filledWidth, barY + LEGACY_PROGRESS_BAR_HEIGHT, 0x00d20a, 255);
        renderEnable(RenderCapability::Texture2D);
    }

    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}
}

LoadingScreenRenderer::LoadingScreenRenderer(Minecraft *minecraft)
	: field_1004_a("")
	, mc(minecraft)
	, field_1007_c("")
	, field_1006_d(System::currentTimeMillis())
	, field_1005_e(false)
{
}

void LoadingScreenRenderer::setProgressMessage(const std::string &s)
{
	field_1005_e = false;
	func_597_c(jstring(s));
}

void LoadingScreenRenderer::resetProgressAndMessageUTF8(const std::string &s)
{
	field_1005_e = true;
	func_597_c(jstring(field_1007_c));
}

void LoadingScreenRenderer::func_597_c(const jstring &s)
{
	prepareProgressDisplay(s);
}

void LoadingScreenRenderer::prepareProgressDisplay(const std::string &s)
{
	if (!mc->running)
	{
		if (field_1005_e)
			return;
		throw MinecraftError();
	}
	field_1007_c = uiText(s);
	ScaledResolution scaledresolution(mc->gameSettings, mc->displayWidth, mc->displayHeight);
	renderClear(RenderClearMask::Depth);
	renderMatrixMode(RenderMatrixMode::Projection);
	renderLoadIdentity();
	renderOrtho(0.0, scaledresolution.field_25121_a, scaledresolution.field_25120_b, 0.0, 100.0, 300.0);
	renderMatrixMode(RenderMatrixMode::ModelView);
	renderLoadIdentity();
	renderTranslate(0.0f, 0.0f, -200.0f);
}

void LoadingScreenRenderer::setLoadingMessage(const std::string &s)
{
	if (!mc->running)
	{
		if (field_1005_e)
			return;
		throw MinecraftError();
	}
	field_1006_d = 0LL;
	field_1004_a = uiText(s);
	setLoadingProgress(-1);
	field_1006_d = 0LL;
}

void LoadingScreenRenderer::setLoadingProgress(int_t i)
{
	if (!mc->running)
	{
		if (field_1005_e)
			return;
		throw MinecraftError();
	}
	long_t l = System::currentTimeMillis();
	if (JavaArithmetic::longSub(l, field_1006_d) < 100LL)
		return;
	field_1006_d = l;
	ScaledResolution scaledresolution(mc->gameSettings, mc->displayWidth, mc->displayHeight);
	int_t j = scaledresolution.getScaledWidth();
	int_t k = scaledresolution.getScaledHeight();
	renderClear(RenderClearMask::Depth);
	renderMatrixMode(RenderMatrixMode::Projection);
	renderLoadIdentity();
	renderOrtho(0.0, scaledresolution.field_25121_a, scaledresolution.field_25120_b, 0.0, 100.0, 300.0);
	renderMatrixMode(RenderMatrixMode::ModelView);
	renderLoadIdentity();
	renderTranslate(0.0f, 0.0f, -200.0f);
	renderClear(RenderClearMask::Color | RenderClearMask::Depth);
	if (mc->gameSettings != nullptr && mc->gameSettings->legacyUI)
	{
		drawLegacyLoadingProgress(mc, j, k, i, field_1004_a);
		lwjgl::Display::update();
		return;
	}
	Tessellator *tessellator = &Tessellator::instance;
	int_t i1 = mc->renderEngine->getTexture("/gui/background.png");
	renderBindTexture(i1);
	float f = 32.0f;
	tessellator->startDrawingQuads();
	tessellator->setColorOpaque_I(0x404040);
	tessellator->addVertexWithUV(0.0, k, 0.0, 0.0, (float)k / f);
	tessellator->addVertexWithUV(j, k, 0.0, (float)j / f, (float)k / f);
	tessellator->addVertexWithUV(j, 0.0, 0.0, (float)j / f, 0.0);
	tessellator->addVertexWithUV(0.0, 0.0, 0.0, 0.0, 0.0);
	tessellator->draw();
	if (i >= 0)
	{
		int_t byte0 = 100;
		int_t byte1 = 2;
		int_t j1 = j / 2 - byte0 / 2;
		int_t k1 = k / 2 + 16;
		renderDisable(RenderCapability::Texture2D);
		tessellator->startDrawingQuads();
		tessellator->setColorOpaque_I(0x808080);
		tessellator->addVertex(j1, k1, 0.0);
		tessellator->addVertex(j1, k1 + byte1, 0.0);
		tessellator->addVertex(j1 + byte0, k1 + byte1, 0.0);
		tessellator->addVertex(j1 + byte0, k1, 0.0);
		tessellator->setColorOpaque_I(0x80ff80);
		tessellator->addVertex(j1, k1, 0.0);
		tessellator->addVertex(j1, k1 + byte1, 0.0);
		tessellator->addVertex(j1 + i, k1 + byte1, 0.0);
		tessellator->addVertex(j1 + i, k1, 0.0);
		tessellator->draw();
		renderEnable(RenderCapability::Texture2D);
	}
	mc->fontRenderer->drawStringWithShadow(field_1007_c, (j - mc->fontRenderer->getStringWidth(field_1007_c)) / 2, k / 2 - 4 - 16, 0xffffff);
	mc->fontRenderer->drawStringWithShadow(field_1004_a, (j - mc->fontRenderer->getStringWidth(field_1004_a)) / 2, (k / 2 - 4) + 8, 0xffffff);
	lwjgl::Display::update();
}

void LoadingScreenRenderer::printText(const jstring &s)
{
	setProgressMessage(s);
}

void LoadingScreenRenderer::displaySavingString(const jstring &s)
{
	resetProgressAndMessageUTF8(s);
}

void LoadingScreenRenderer::resetProgressAndMessage(const jstring &s)
{
	displaySavingString(s);
}

void LoadingScreenRenderer::displayLoadingString(const jstring &s)
{
	setLoadingMessage(s);
}
