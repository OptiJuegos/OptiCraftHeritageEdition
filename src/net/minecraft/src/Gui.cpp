#include "Gui.h"
#include "Tessellator.h"
#include "FontRenderer.h"
#include "platform/RenderAPI.h"

Gui::Gui()
	: zLevel(0.0f)
{
}

void Gui::drawHorizontalLine(int_t x1, int_t x2, int_t y, int_t color)
{
	if (x2 < x1)
	{
		int_t tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	drawRect(x1, y, x2 + 1, y + 1, color);
}

void Gui::drawVerticalLine(int_t x, int_t y1, int_t y2, int_t color)
{
	if (y2 < y1)
	{
		int_t tmp = y1;
		y1 = y2;
		y2 = tmp;
	}
	drawRect(x, y1 + 1, x + 1, y2, color);
}

void Gui::drawRect(int_t x1, int_t y1, int_t x2, int_t y2, int_t color)
{
	if (x1 < x2)
	{
		int_t tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	if (y1 < y2)
	{
		int_t tmp = y1;
		y1 = y2;
		y2 = tmp;
	}
	float_t a = (float_t)((color >> 24) & 0xff) / 255.0f;
	float_t r = (float_t)((color >> 16) & 0xff) / 255.0f;
	float_t g = (float_t)((color >> 8)  & 0xff) / 255.0f;
	float_t b = (float_t)( color        & 0xff) / 255.0f;
	Tessellator *tess = &Tessellator::instance;
	renderEnable(RenderCapability::Blend);
	renderDisable(RenderCapability::Texture2D);
	renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
	renderColor4f(r, g, b, a);
	tess->startDrawingQuads();
	tess->addVertex(x1, y2, 0.0);
	tess->addVertex(x2, y2, 0.0);
	tess->addVertex(x2, y1, 0.0);
	tess->addVertex(x1, y1, 0.0);
	tess->draw();
	renderEnable(RenderCapability::Texture2D);
	renderDisable(RenderCapability::Blend);
}

void Gui::drawGradientRect(int_t x1, int_t y1, int_t x2, int_t y2, int_t colorTop, int_t colorBottom)
{
	float_t aT = (float_t)((colorTop    >> 24) & 0xff) / 255.0f;
	float_t rT = (float_t)((colorTop    >> 16) & 0xff) / 255.0f;
	float_t gT = (float_t)((colorTop    >> 8)  & 0xff) / 255.0f;
	float_t bT = (float_t)( colorTop           & 0xff) / 255.0f;
	float_t aB = (float_t)((colorBottom >> 24) & 0xff) / 255.0f;
	float_t rB = (float_t)((colorBottom >> 16) & 0xff) / 255.0f;
	float_t gB = (float_t)((colorBottom >> 8)  & 0xff) / 255.0f;
	float_t bB = (float_t)( colorBottom        & 0xff) / 255.0f;
	renderDisable(RenderCapability::Texture2D);
	renderEnable(RenderCapability::Blend);
	renderDisable(RenderCapability::AlphaTest);
	renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
	renderShadeModel(RenderShadeModel::Smooth);
	Tessellator *tess = &Tessellator::instance;
	tess->startDrawingQuads();
	tess->setColorRGBA_F(rT, gT, bT, aT);
	tess->addVertex(x2, y1, 0.0);
	tess->addVertex(x1, y1, 0.0);
	tess->setColorRGBA_F(rB, gB, bB, aB);
	tess->addVertex(x1, y2, 0.0);
	tess->addVertex(x2, y2, 0.0);
	tess->draw();
	renderShadeModel(RenderShadeModel::Flat);
	renderDisable(RenderCapability::Blend);
	renderEnable(RenderCapability::AlphaTest);
	renderEnable(RenderCapability::Texture2D);
}

void Gui::drawCenteredString(FontRenderer *fontrenderer, const std::string &s, int_t x, int_t y, int_t color)
{
	fontrenderer->drawStringWithShadow(s, x - fontrenderer->getStringWidth(s) / 2, y, color);
}

void Gui::drawString(FontRenderer *fontrenderer, const std::string &s, int_t x, int_t y, int_t color)
{
	fontrenderer->drawStringWithShadow(s, x, y, color);
}

void Gui::drawTexturedModalRect(int_t x, int_t y, int_t texX, int_t texY, int_t w, int_t h)
{
	float_t f  = 0.00390625f; // 1/256
	float_t f1 = 0.00390625f;
	Tessellator *tess = &Tessellator::instance;

	tess->startDrawingQuads();
	tess->addVertexWithUV(x + 0, y + h, zLevel, (float_t)(texX + 0) * f,  (float_t)(texY + h) * f1);
	tess->addVertexWithUV(x + w, y + h, zLevel, (float_t)(texX + w) * f,  (float_t)(texY + h) * f1);
	tess->addVertexWithUV(x + w, y + 0, zLevel, (float_t)(texX + w) * f,  (float_t)(texY + 0) * f1);
	tess->addVertexWithUV(x + 0, y + 0, zLevel, (float_t)(texX + 0) * f,  (float_t)(texY + 0) * f1);
	tess->draw();
}
