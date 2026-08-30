#pragma once

#include "Gui.h"
#include <string>

class Minecraft;

// net.minecraft.src.GuiButton
class GuiButton : public Gui
{
public:
	GuiButton(int_t id, int_t x, int_t y, const std::string &text);
	GuiButton(int_t id, int_t x, int_t y, int_t w, int_t h, const std::string &text);
	virtual ~GuiButton() = default;

protected:
	virtual int_t getHoverState(bool hovered);

public:
	virtual void drawButton(Minecraft *mc, int_t mouseX, int_t mouseY);

protected:
	virtual void mouseDragged(Minecraft *mc, int_t mouseX, int_t mouseY);

public:
	virtual void mouseReleased(int_t mouseX, int_t mouseY);
	virtual bool mousePressed(Minecraft *mc, int_t mouseX, int_t mouseY);

protected:
	int_t width;
	int_t height;

public:
	int_t xPosition;
	int_t yPosition;
	std::string displayString;
	int_t id;
	bool enabled;
	bool enabled2;

	// Accesores para deteccion de hover (tooltips OptiFine); width/height son protected.
	int_t getButtonWidth() const { return width; }
	int_t getButtonHeight() const { return height; }
};
