#pragma once

#include "Gui.h"
#include <vector>
#include <string>
#include "java/String.h"

class Minecraft;
class GuiButton;
class GuiParticle;
class FontRenderer;
class GuiTextField;

// net.minecraft.src.GuiScreen
class GuiScreen : public Gui
{
public:
	GuiScreen();
	virtual ~GuiScreen();

	virtual void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick);
	// Dual-screen GUI (3DS only): true when drawScreen() runs its own pass
	// over the bottom LCD -- the title screen draws its top half first and
	// then opens the pass for the menu half. EntityRenderer wraps
	// drawScreen() in renderBottomPanelBegin/End only when this returns
	// false, so the two passes never nest.
	virtual bool managesBottomPanelPass() const { return false; }

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
	void clearControlList(); // Drop mouse capture before replacing buttons.
	virtual bool usesSpecializedMenuNavigation() const;

public:
	void setWorldAndResolution(Minecraft *minecraft, int_t w, int_t h);
	virtual void initGui();
	void handleInput();

protected:
	virtual void handleSpecializedMenuInput();
	void handleSplitscreenPlayerInput();

public:
	virtual void handleMouseInput();
	void handleKeyboardInput();
	virtual void updateScreen();
	virtual void onGuiClosed();
	// Legacy console menus normally use D-pad navigation. Containers opt in to
	// a free pointer as well, so an analogue stick can take over from a slot
	// selection without changing the behaviour of other legacy screens.
	virtual bool allowsPlatformPointerInput() const { return false; }
	virtual bool suppressesPlatformPointerInput() const { return false; }
	bool usesSpecializedMenuNavigationForPlatform() const { return usesSpecializedMenuNavigation(); }
	void drawDefaultBackground();
	void drawWorldBackground(int_t ticks);
	void drawBackground(int_t ticks);
	virtual bool doesGuiPauseGame();
	virtual void deleteWorld(bool confirmed, int_t worldNum);
	virtual void confirmClicked(bool confirmed, int_t id);
	virtual void selectNextField();
	void notifyTextFieldFocus(GuiTextField *field, bool focused);

	void setOwnerPlayerIndex(int idx) { m_ownerPlayerIndex = idx; }
	virtual int getOwnerPlayerIndex() const;

protected:
	Minecraft *mc;
	int m_ownerPlayerIndex;

public:
	int_t width;
	int_t height;

protected:
	std::vector<GuiButton *> controlList;

public:
	bool field_948_f;

protected:
	FontRenderer *fontRenderer;
	virtual bool isJavaUiKeyboardNavigationEnabled() const;

public:
	GuiParticle *guiParticles;  // field_25091_h

protected:
	// Screens with a pad-facing confirm of their own override this: the
	// Java-UI navigation ring otherwise claims KEY_RETURN (the 3DS pad's A
	// twin) as "click the selected button" before the screen's keyTyped
	// can see it.
	virtual bool handleJavaUiNavigationKey(int_t key);

private:
	void syncKeyboardSelection();
	bool moveKeyboardSelection(int_t direction);
	bool activateKeyboardSelection();
	bool adjustKeyboardSelection(int_t direction);
	void moveMenuCursorToKeyboardSelection();
	void clearKeyboardSelectionFromPointer();
	void handleConsoleJavaUiNavigation();

	GuiButton *selectedButton;
	int_t keyboardSelectedControlIndex;
	GuiTextField *focusedTextField;
};
