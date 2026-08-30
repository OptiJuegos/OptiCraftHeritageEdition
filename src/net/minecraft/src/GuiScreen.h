#pragma once

#include "Gui.h"
#include <vector>
#include <string>
#include "java/String.h"

class Minecraft;
class GuiButton;
class GuiParticle;
class FontRenderer;

// net.minecraft.src.GuiScreen
class GuiScreen : public Gui
{
public:
	GuiScreen();
	virtual ~GuiScreen();

	virtual void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick);

protected:
	virtual void keyTyped(char_t c, int_t key);

public:
	static jstring getClipboardString();
	static void setClipboardString(const std::string &text);
	static bool isCtrlKeyDown();
	static bool isShiftKeyDown();
	static bool func_50051_l() { return isCtrlKeyDown(); }
	static bool func_50049_m() { return isShiftKeyDown(); }
	static void func_50050_a(const std::string &text) { setClipboardString(text); }

protected:
	virtual void mouseClicked(int_t x, int_t y, int_t button);
	virtual void mouseMovedOrUp(int_t x, int_t y, int_t button);
	virtual void actionPerformed(GuiButton *button);

public:
	void setWorldAndResolution(Minecraft *minecraft, int_t w, int_t h);
	virtual void initGui();
	void handleInput();
	virtual void handleMouseInput();
	void handleKeyboardInput();
	virtual void updateScreen();
	virtual void onGuiClosed();
	void drawDefaultBackground();
	void drawWorldBackground(int_t ticks);
	void drawBackground(int_t ticks);
	virtual bool doesGuiPauseGame();
	virtual void deleteWorld(bool confirmed, int_t worldNum);
	virtual void confirmClicked(bool confirmed, int_t id);
	virtual void selectNextField();

protected:
	Minecraft *mc;

public:
	int_t width;
	int_t height;

protected:
	std::vector<GuiButton *> controlList;

public:
	bool field_948_f;

protected:
	FontRenderer *fontRenderer;

public:
	GuiParticle *guiParticles;  // field_25091_h

private:
	GuiButton *selectedButton;
};
