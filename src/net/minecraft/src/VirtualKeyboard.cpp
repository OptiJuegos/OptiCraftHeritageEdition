#include "VirtualKeyboard.h"
#include "java/String.h"

#if defined(PS2_PLATFORM) || defined(WII_PLATFORM) || defined(CTR_PLATFORM)

#include "platform/ConsoleInputClock.h"
#include "GuiTextField.h"
#include "FontRenderer.h"
#include "lwjgl/Keyboard.h"
#include "platform/Input.h"
#if defined(CTR_PLATFORM)
#include "3ds/DsSwkbd.h"
#include "ChatAllowedCharacters.h"
#include "platform/RenderAPI.h"
#endif


namespace
{
	// 4 rows x 11 columns. Shift uppercases the letters; digits/symbols are
	// unchanged. Covers world names, seeds and chat. Space/backspace/etc. are on
	// the face buttons (see render() help line).
	const char* KB_ROWS[] = {
		"1234567890?",
		"qwertyuiop@",
		"asdfghjkl_=",
		"zxcvbnm/.-:",
	};
	const int KB_COLS     = 11;
	const int KB_ROWCOUNT = 4;

	// Shared with the container slot navigation, which needs the same clock for
	// the same reason: see ConsoleInputClock.h.
	int nowMs()
	{
		return consoleInputNowMs();
	}
}

VirtualKeyboard& VirtualKeyboard::instance()
{
	static VirtualKeyboard s_kb;
	return s_kb;
}

void VirtualKeyboard::resetSelection()
{
	selX = 0;
	selY = 0;
	shift = false;
	panelPositionInitialized = false;
	lastMoveMs = nowMs();
}

void VirtualKeyboard::typeSelectedKey()
{
	char c = KB_ROWS[selY][selX];
	if (shift && c >= 'a' && c <= 'z')
		c = (char)(c - 'a' + 'A');
	lwjgl::Keyboard::detail::pushChar((int)(unsigned char)c);
}

void VirtualKeyboard::notifyFocus(GuiTextField* field, bool focused)
{
	if (focused)
	{
		if (focusedField != field)
		{
			focusedField = field;
			resetSelection();
		}
		lastHeld = platformTextInputSnapshot(platformMenuPad()).held;
		nextRepeatMs = nowMs() + 250;
#if defined(CTR_PLATFORM)
		// The system keyboard is opened on the next tick rather than from
		// here: GuiChat (and friends) write the field's initial text right
		// after focusing it, and an applet launched inside setFocused()
		// would return before that write and be overwritten by it.
		pendingNativeOpen = true;
#endif
	}
	else if (focusedField == field)
	{
		focusedField = nullptr;
#if defined(CTR_PLATFORM)
		pendingNativeOpen = false;
#endif
	}
	platformSetTextInputExclusive(focusedField != nullptr);
}

// See the header. Only a pointer comparison: the field may already be freed
// memory (its screen was purged), and a field that IS the caller's own is
// left untouched.
void VirtualKeyboard::dropForeignField(GuiTextField* field)
{
	if (focusedField != nullptr && focusedField != field)
		notifyFocus(focusedField, false);
}

// The focused field's memory is going away. Unlike dropForeignField this is
// unconditional: the caller IS the field the keyboard may be holding, so
// there is nothing to compare against -- clear it, release text-exclusive
// input, and make sure no deferred native launch still points at it. The
// 3DS's async system keyboard makes this reachable: the game loop keeps
// running while the applet is up, so a disconnect (or any screen swap) can
// scrap the focused field's screen mid-dialog, and the dialog's outcome then
// has nowhere to go but the discard in pollNativeKeyboard().
void VirtualKeyboard::fieldDestroyed(GuiTextField* field)
{
	if (field == nullptr || focusedField != field)
		return;
	focusedField = nullptr;
#if defined(CTR_PLATFORM)
	pendingNativeOpen = false;
#endif
	platformSetTextInputExclusive(false);
}

void VirtualKeyboard::tick()
{
#if defined(CTR_PLATFORM)
	// Consume a finished dialog first, before the isActive() gate below: its
	// field may already be gone (a screen swap mid-dialog clears it through
	// fieldDestroyed()), and an unconsumed result would block every later
	// field on the one-dialog-at-a-time contract in DsSwkbd.
	pollNativeKeyboard();
#endif
	if (!isActive())
		return;

#if defined(CTR_PLATFORM)
	// While the system keyboard is up the applet owns the buttons and both
	// screens: nothing to inject and no panel to navigate. The game keeps
	// ticking behind it -- that is the point (DsSwkbd.h) -- and the outcome
	// arrives through pollNativeKeyboard() above.
	if (nativeAppletOpen)
		return;

	if (pendingNativeOpen)
	{
		pendingNativeOpen = false;
		openNativeKeyboard();
		if (nativeAppletOpen || !isActive())
			return; // dialog launched (async), or its result already closed the field
	}
#endif

	unsigned int held = 0;
	unsigned int pressed = 0;
	const PlatformTextInputSnapshot pad = platformTextInputSnapshot(platformMenuPad());
	if (!pad.connected)
		return;
	held = pad.held;
	pressed = pad.pressed;
	lastHeld = held;

	const int now = nowMs();
#if defined(PS2_PLATFORM)
	// Text input owns the face buttons and D-pad, while the otherwise-unused
	// right stick moves the keyboard panel. Work in scaled GUI coordinates so
	// speed remains consistent across video modes and GUI scales.
	if (panelPositionInitialized && lastScreenWidth > 0 && lastScreenHeight > 0)
	{
		float_t elapsed = static_cast<float_t>(now - lastMoveMs) / 1000.0f;
		if (elapsed < 0.0f)
			elapsed = 0.0f;
		if (elapsed > 0.05f)
			elapsed = 0.05f;
		const PlatformGamepadSnapshot stick = platformGamepadSnapshot(platformMenuPad());
		constexpr float_t MOVE_SPEED = 180.0f;
		panelX += stick.rightX * MOVE_SPEED * elapsed;
		panelY += stick.rightY * MOVE_SPEED * elapsed;

		const int keyW = 22, keyH = 18, gap = 3;
		const int panelW = KB_COLS * keyW + gap * 2;
		const PlatformKeyboardHints& hints = platformKeyboardHints();
		const int hintHeight = hints.lineCount > 1 ? hints.lineCount * 10 + 20 : 30;
		const int panelH = KB_ROWCOUNT * keyH + hintHeight;
		const float_t minX = lastScreenWidth >= panelW + 4 ? 2.0f : 0.0f;
		const float_t minY = lastScreenHeight >= panelH + 4 ? 2.0f : 0.0f;
		const float_t maxX = static_cast<float_t>(lastScreenWidth >= panelW + 4 ? lastScreenWidth - panelW - 2 : 0);
		const float_t maxY = static_cast<float_t>(lastScreenHeight >= panelH + 4 ? lastScreenHeight - panelH - 2 : 0);
		if (panelX < minX) panelX = minX;
		if (panelY < minY) panelY = minY;
		if (panelX > maxX) panelX = maxX;
		if (panelY > maxY) panelY = maxY;
	}
#endif
	lastMoveMs = now;

	// Pointer coordinates use the physical framebuffer while this overlay uses
	// Minecraft's scaled GUI canvas. Convert them before hit-testing the keys.
	if (pad.pointerValid && pad.pointerWidth > 0 && pad.pointerHeight > 0 &&
		lastScreenWidth > 0 && lastScreenHeight > 0)
	{
		const int pointerX = pad.pointerX * lastScreenWidth / pad.pointerWidth;
		const int pointerY = pad.pointerY * lastScreenHeight / pad.pointerHeight;
		const int keyW = 22, keyH = 18, gap = 3;
		const int panelW = KB_COLS * keyW + gap * 2;
		const PlatformKeyboardHints& hints = platformKeyboardHints();
		const int hintHeight = hints.lineCount > 1 ? hints.lineCount * 10 + 20 : 30;
		const int panelH = KB_ROWCOUNT * keyH + hintHeight;
		const int px = panelPositionInitialized ? static_cast<int>(panelX) : (lastScreenWidth - panelW) / 2;
		const int py = panelPositionInitialized ? static_cast<int>(panelY) : lastScreenHeight - panelH - 4;
		const int gridY = py + 16;
		const int col = (pointerX - px - gap) / keyW;
		const int row = (pointerY - gridY) / keyH;
		if (pointerX >= px + gap && pointerY >= gridY &&
			col >= 0 && col < KB_COLS && row >= 0 && row < KB_ROWCOUNT &&
			pointerX < px + gap + KB_COLS * keyW &&
			pointerY < gridY + KB_ROWCOUNT * keyH)
		{
			selX = col;
			selY = row;
#if defined(CTR_PLATFORM)
			// Tap-to-type on the bottom-screen panel: the finger IS the
			// panel the keys are drawn on, so landing on a key types it
			// outright (A still types the highlighted key for the pad-only
			// player). Only a fresh contact types -- holding the finger
			// still must not machine-gun the character.
			if (bottomMode && !lastPointerValid)
				typeSelectedKey();
#endif
		}
	}
#if defined(CTR_PLATFORM)
	// Edge bookkeeping runs every tick, even while the finger is up: the
	// next contact must still look like the first one of a new tap.
	lastPointerValid = pad.pointerValid;
#endif

	int moveX = 0, moveY = 0;
	const unsigned int keyLeft = PLATFORM_TEXT_LEFT, keyRight = PLATFORM_TEXT_RIGHT;
	const unsigned int keyUp = PLATFORM_TEXT_UP, keyDown = PLATFORM_TEXT_DOWN;
	const unsigned int keyType = PLATFORM_TEXT_TYPE, keyBack = PLATFORM_TEXT_BACK;
	const unsigned int keySpace = PLATFORM_TEXT_SPACE, keyShift = PLATFORM_TEXT_SHIFT;
	const unsigned int keyEnter = PLATFORM_TEXT_ENTER, keyClose = PLATFORM_TEXT_CLOSE;
	if (pressed & keyLeft)  moveX = -1;
	if (pressed & keyRight) moveX = 1;
	if (pressed & keyUp)    moveY = -1;
	if (pressed & keyDown)  moveY = 1;

	// D-pad auto-repeat for held directions.
	const unsigned int heldDpad = held & (keyLeft | keyRight | keyUp | keyDown);
	if (!moveX && !moveY && heldDpad && now >= nextRepeatMs)
	{
		if (held & keyLeft)  moveX = -1;
		if (held & keyRight) moveX = 1;
		if (held & keyUp)    moveY = -1;
		if (held & keyDown)  moveY = 1;
		nextRepeatMs = now + 90;
	}
	else if (pressed & heldDpad)
	{
		nextRepeatMs = now + 250;
	}

	selX += moveX;
	selY += moveY;
	if (selX < 0) selX = KB_COLS - 1;
	if (selX >= KB_COLS) selX = 0;
	if (selY < 0) selY = KB_ROWCOUNT - 1;
	if (selY >= KB_ROWCOUNT) selY = 0;

	// Cross: type the selected key.
	if (pressed & keyType)
		typeSelectedKey();
	// Square: backspace.
	if (pressed & keyBack)
	{
		lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_BACK, true);
		lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_BACK, false);
	}
	// Select: space.
	if (pressed & keySpace)
		lwjgl::Keyboard::detail::pushChar((int)' ');
	// Triangle: toggle shift (caps).
	if (pressed & keyShift)
		shift = !shift;
	// Start: Enter (submits chat / confirms the field).
	if (pressed & keyEnter)
	{
		lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_RETURN, true);
		lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_RETURN, false);
	}
	// Circle: close the keyboard by unfocusing the field (back to cursor nav).
	if (pressed & keyClose)
	{
		if (focusedField)
			focusedField->setFocused(false); // -> notifyFocus(this, false)
	}
}

#if defined(CTR_PLATFORM)
void VirtualKeyboard::openNativeKeyboard()
{
	if (focusedField == nullptr)
		return;
	if (nativeKeyboardFailed)
	{
		// The applet failed once this session (see dsSwkbdOpenAsync): keep
		// every field on the bottom-screen panel rather than retrying the
		// launch.
		bottomMode = true;
		return;
	}

	// Async launch: the dialog runs on DsSwkbd's helper thread, and the game
	// loop -- world tick, network read/write/dispatch -- keeps running while
	// the applet owns both screens. That is what keeps a server session
	// alive while the player types a chat line or a /register command; the
	// blocking shape this replaced is what froze the client and let the
	// server time it out. The outcome is consumed by pollNativeKeyboard()
	// on a later tick.
	if (!dsSwkbdOpenAsync(focusedField->getText(), focusedField->getMaxStringLength()))
	{
		// One dialog at a time (an earlier result is still unconsumed) or the
		// helper could not start: this field types on the panel instead, and
		// the stale result is picked up by the poll at the top of tick().
		bottomMode = true;
		return;
	}
	nativeAppletOpen = true;
}

void VirtualKeyboard::pollNativeKeyboard()
{
	if (!nativeAppletOpen)
		return;
	if (dsSwkbdActive())
		return; // the applet is still up; the player is still typing

	DsSwkbdResult result;
	if (!dsSwkbdTakeResult(result))
		return; // dialog closed but the result is not stored yet; try next tick
	nativeAppletOpen = false;

	if (focusedField == nullptr)
	{
		// The focused field's screen was scrapped mid-dialog (see
		// fieldDestroyed): the outcome has nowhere to go.
		return;
	}

	switch (result.outcome)
	{
	case DsSwkbdOutcome::Confirmed:
		// Same filter + clamp GuiTextField::writeText applies to typed
		// characters, then leave the field: with it unfocused a confirm
		// press reaches the screen as KEY_RETURN, which is what submits
		// chat ("Aceptar deja texto, A envia").
		focusedField->setText(
			ChatAllowedCharacters::filterAllowedCharacters(result.text));
		focusedField->setFocused(false);
		break;
	case DsSwkbdOutcome::Cancelled:
		// Keep whatever was in the field and close the same way OK does, so
		// the player always lands in the same state once the applet leaves.
		focusedField->setFocused(false);
		break;
	case DsSwkbdOutcome::Unavailable:
		nativeKeyboardFailed = true;
		bottomMode = true;
		break;
	}
}
#endif // CTR_PLATFORM

void VirtualKeyboard::render(FontRenderer* font, int_t screenWidth, int_t screenHeight)
{
	if (!isActive() || font == nullptr)
		return;

#if defined(CTR_PLATFORM)
	// The system keyboard opens on the next tick (notifyFocus defers it) and
	// then covers both screens for as long as it is up: do not flash this
	// panel for the frame in between, and do not draw it under the applet --
	// the renderer suspends frames entirely once the applet launches.
	if (pendingNativeOpen || nativeAppletOpen)
		return;

	// The fallback panel draws on the bottom LCD in the panel's own pixels,
	// which is also the space DsInput feeds the hit-test with while a field
	// has focus (raw 320x240, see InputBackend_3DS) -- lastScreenWidth/
	// Height below are exactly what tick() converts touch coordinates
	// against, so both modes come out aligned. If the panel cannot be made,
	// the pass never starts and the panel falls back over the top screen at
	// GUI scale, where the same conversion lands the finger correctly too.
	bool bottomPass = false;
	if (bottomMode)
		bottomPass = renderKeyboardBottomBegin();
	if (bottomPass)
	{
		screenWidth = 320;
		screenHeight = 240;
	}
#endif
	lastScreenWidth = screenWidth;
	lastScreenHeight = screenHeight;
	const int keyW = 22, keyH = 18, gap = 3;
	const int panelW = KB_COLS * keyW + gap * 2;
	const PlatformKeyboardHints& hints = platformKeyboardHints();
	const int hintHeight = hints.lineCount > 1 ? hints.lineCount * 10 + 20 : 30;
	const int panelH = KB_ROWCOUNT * keyH + hintHeight;
	if (!panelPositionInitialized)
	{
		panelX = static_cast<float_t>((screenWidth - panelW) / 2);
		panelY = static_cast<float_t>(screenHeight - panelH - 4);
		panelPositionInitialized = true;
		lastMoveMs = nowMs();
	}
	const float_t minX = screenWidth >= panelW + 4 ? 2.0f : 0.0f;
	const float_t minY = screenHeight >= panelH + 4 ? 2.0f : 0.0f;
	const float_t maxX = static_cast<float_t>(screenWidth >= panelW + 4 ? screenWidth - panelW - 2 : 0);
	const float_t maxY = static_cast<float_t>(screenHeight >= panelH + 4 ? screenHeight - panelH - 2 : 0);
	if (panelX < minX) panelX = minX;
	if (panelY < minY) panelY = minY;
	if (panelX > maxX) panelX = maxX;
	if (panelY > maxY) panelY = maxY;
	const int px = static_cast<int>(panelX);
	const int py = static_cast<int>(panelY);

	drawRect(px - 2, py - 2, px + panelW + 2, py + panelH + 2, 0xdd000000);
	drawRect(px, py, px + panelW, py + panelH, 0xff202020);

	// Keep the active text visible even when the keyboard covers the original
	// field. Long values scroll from the left so the insertion end stays shown.
	jstring preview = focusedField->getText() + "_";
	const int previewW = panelW - 10;
	bool clipped = false;
	while (String::utf16Length(preview) > 1 && font->getStringWidth(preview) > previewW)
	{
		preview = String::substringUtf16(preview, 1, String::utf16Length(preview));
		clipped = true;
	}
	if (clipped)
	{
		preview = "<" + preview;
		while (String::utf16Length(preview) > 1 && font->getStringWidth(preview) > previewW)
		{
			const int_t length = String::utf16Length(preview);
			preview = String::substringUtf16(preview, 0, 1) + String::substringUtf16(preview, 2, length);
		}
	}
	drawRect(px + 3, py + 3, px + panelW - 3, py + 14, 0xff000000);
	drawString(font, preview, px + 5, py + 5, shift ? 0xffff80 : 0xffffff);

	const int gridY = py + 16;
	for (int row = 0; row < KB_ROWCOUNT; row++)
	{
		for (int col = 0; col < KB_COLS; col++)
		{
			const int x0 = px + gap + col * keyW;
			const int y0 = gridY + row * keyH;
			const bool sel = (row == selY && col == selX);
			drawRect(x0, y0, x0 + keyW - 2, y0 + keyH - 2, sel ? 0xffffffff : 0xff707070);
			drawRect(x0 + 1, y0 + 1, x0 + keyW - 3, y0 + keyH - 3, sel ? 0xff606060 : 0xff101010);
			char ch = KB_ROWS[row][col];
			if (shift && ch >= 'a' && ch <= 'z')
				ch = (char)(ch - 'a' + 'A');
			const char s[2] = { ch, 0 };
			drawString(font, std::string(s), x0 + 8, y0 + 5, 0xffffff);
		}
	}

	for (int line = 0; line < hints.lineCount; ++line)
	{
		if (hints.lines[line] != nullptr)
			drawString(font, hints.lines[line], px + 2, py + panelH - (hints.lineCount - line) * 10, 0xcccccc);
	}

#if defined(CTR_PLATFORM)
	if (bottomPass)
		renderKeyboardBottomEnd(); // back to the top screen for the rest of the frame
#endif
}

#endif // PS2_PLATFORM || WII_PLATFORM || CTR_PLATFORM
