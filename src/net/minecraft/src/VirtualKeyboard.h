#pragma once

#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)

#include "Gui.h"
#include "java/Type.h"

class GuiTextField;
class FontRenderer;

// On-screen keyboard for PS2 and Wii. It behaves like SDL2 text input: it pops up while a
// GuiTextField is focused (chat, world name/seed, rename, sign...), is navigated
// with the D-pad and typed with Cross. Characters are injected into the
// lwjgl::Keyboard event queue, so the focused field receives them through the
// normal keyTyped() path -- no per-screen wiring needed.
class VirtualKeyboard : public Gui
{
public:
	static VirtualKeyboard& instance();

	// Called by GuiTextField::setFocused(). The keyboard is active while a field
	// is focused; focusing a different field resets the selection.
	void notifyFocus(GuiTextField* field, bool focused);
	bool isActive() const { return focusedField != nullptr; }

	// Per-frame while active: read the pad and inject input events.
	void tick();
	// Draw the keyboard panel (call after the screen is drawn, in scaled coords).
	void render(FontRenderer* font, int_t screenWidth, int_t screenHeight);

private:
	VirtualKeyboard() = default;
	void resetSelection();

	GuiTextField*  focusedField = nullptr;
	int_t          selX = 0;
	int_t          selY = 0;
	bool           shift = false;
	unsigned int lastHeld = 0;
	int            nextRepeatMs = 0;
	int_t          lastScreenWidth = 0;
	int_t          lastScreenHeight = 0;
};

#endif // PS2_PLATFORM || WII_PLATFORM
