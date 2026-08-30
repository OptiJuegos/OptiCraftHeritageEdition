#include "LoadingScreenRenderer.h"

#include "FontRenderer.h"
#include "Minecraft.h"
#include "MinecraftError.h"
#include "RenderEngine.h"
#include "ScaledResolution.h"
#include "Tessellator.h"
#include "java/String.h"
#include "java/Arithmetic.h"
#include "java/System.h"
#include "platform/RenderAPI.h"
#include "pc/lwjgl/Display.h"

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
	field_1007_c = s;
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
	field_1004_a = s;
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
