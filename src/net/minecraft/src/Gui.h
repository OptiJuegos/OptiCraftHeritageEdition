#pragma once

#include "java/Type.h"
#include <string>

class FontRenderer;
class Tessellator;

// net.minecraft.src.Gui
class Gui
{
public:
	Gui();
	virtual ~Gui() = default;

protected:
	void drawHorizontalLine(int_t x1, int_t x2, int_t y, int_t color);
	void drawVerticalLine(int_t x, int_t y1, int_t y2, int_t color);
	void drawRect(int_t x1, int_t y1, int_t x2, int_t y2, int_t color);
	void drawGradientRect(int_t x1, int_t y1, int_t x2, int_t y2, int_t colorTop, int_t colorBottom);

public:
	void drawCenteredString(FontRenderer *fontrenderer, const std::string &s, int_t x, int_t y, int_t color);
	void drawString(FontRenderer *fontrenderer, const std::string &s, int_t x, int_t y, int_t color);
	void drawTexturedModalRect(int_t x, int_t y, int_t texX, int_t texY, int_t w, int_t h);

protected:
	float_t zLevel;
};
