#pragma once

#if defined(PS2_PLATFORM) || defined(WII_PLATFORM) || defined(CTR_PLATFORM)

#include "Gui.h"
#include "java/Type.h"

class GuiTextField;
class FontRenderer;

// On-screen keyboard for PS2, Wii and 3DS. It behaves like SDL2 text input: it pops up while a
// GuiTextField is focused (chat, world name/seed, rename, sign...), is navigated
// with the D-pad and typed with Cross. Characters are injected into the
// lwjgl::Keyboard event queue, so the focused field receives them through the
// normal keyTyped() path -- no per-screen wiring needed.
//
// The 3DS prefers the SYSTEM keyboard (swkbd, src/3ds/DsSwkbd.h): it opens on
// the tick after focus arrives, runs on a helper thread (the game loop -- and
// the server session behind it -- keeps running while the applet owns both
// screens), and on OK it fills the field and leaves it unfocused, so a confirm
// press can then submit it (chat sends on KEY_RETURN, which the menu channel
// pushes for A). When the applet cannot be used, the panel below draws on the
// bottom LCD instead of over the top screen and the pad plus touch type as on
// the other consoles.
class VirtualKeyboard : public Gui
{
public:
	static VirtualKeyboard& instance();

	// Called by GuiTextField::setFocused(). The keyboard is active while a field
	// is focused; focusing a different field resets the selection.
	void notifyFocus(GuiTextField* field, bool focused);
	// Called by GuiScreen::handleInput right before tick(): the keyboard may
	// only hold the focused field of the screen that is receiving input.
	// Screens outlive their turn as current (they are kept for the
	// back-stack and purged only on return to gameplay), so a field left
	// focused would otherwise keep the keyboard "active" over a foreign,
	// possibly dangling pointer -- holding text-exclusive input and eating
	// the platform text snapshot before the container navigator reads it.
	void dropForeignField(GuiTextField* field);
	// Called by ~GuiTextField: the focused field is about to be freed. With
	// the 3DS's async system keyboard the game loop keeps running while the
	// applet is up, so a screen swap (a server disconnect, a container
	// closing) can scrap the focused field's screen mid-dialog -- the
	// keyboard must drop the pointer instead of writing the dialog's result
	// into freed memory.
	void fieldDestroyed(GuiTextField* field);
	bool isActive() const { return focusedField != nullptr; }

	// Per-frame while active: read the pad and inject input events.
	void tick();
	// Draw the keyboard panel (call after the screen is drawn, in scaled coords).
	void render(FontRenderer* font, int_t screenWidth, int_t screenHeight);

private:
	VirtualKeyboard() = default;
	void resetSelection();
	// Inject the character under the selection as a typed event.
	void typeSelectedKey();
#if defined(CTR_PLATFORM)
	// Launch the system keyboard for the focused field (see tick()).
	void openNativeKeyboard();
	// Consume a finished system-keyboard dialog (see tick()): applies the
	// outcome to the field if it is still alive, or discards it, and joins
	// the applet's helper thread.
	void pollNativeKeyboard();
#endif

	GuiTextField*  focusedField = nullptr;
	int_t          selX = 0;
	int_t          selY = 0;
	bool           shift = false;
	unsigned int lastHeld = 0;
	int            nextRepeatMs = 0;
	int_t          lastScreenWidth = 0;
	int_t          lastScreenHeight = 0;
	float_t        panelX = 0.0f;
	float_t        panelY = 0.0f;
	int            lastMoveMs = 0;
	bool           panelPositionInitialized = false;
#if defined(CTR_PLATFORM)
	// A focus event launched the system keyboard on the next tick (deferred
	// so the screen finishes wiring the field first).
	bool           pendingNativeOpen = false;
	// The swkbd applet is up right now: launched asynchronously (see
	// openNativeKeyboard), awaiting its outcome in pollNativeKeyboard().
	// Input is suspended for the field -- the applet owns the buttons and
	// both screens -- while the game loop keeps running.
	bool           nativeAppletOpen = false;
	// swkbd failed once this session: keep the bottom-screen panel for every
	// field instead of retrying the launch (see openNativeKeyboard).
	bool           nativeKeyboardFailed = false;
	// Draw on the bottom LCD rather than over the top screen.
	bool           bottomMode = false;
	// Previous tick's touch state, for the panel's tap-to-type edge.
	bool           lastPointerValid = false;
#endif
};

#endif // PS2_PLATFORM || WII_PLATFORM || CTR_PLATFORM
