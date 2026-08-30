#ifdef WII_PLATFORM

#include <cmath>
#include <gccore.h>
#include <wiiuse/wpad.h>

#include "lwjgl/Keyboard.h"
#include "lwjgl/Mouse.h"
#include "platform/Input.h"
#include "wii/gx_wii.h"
#include "wii/input/WiiButtonBindings.h"
#include "wii/input/WiiInputDebug.h"
#include "wii/input/WiiPadInternal.h"
#include "wii/input/WiiPadKeyCodes.h"
#include "wii/input/WiiPointer.h"
#include "wii/input/WiiRemote.h"

namespace
{
constexpr float kDegToRad = 0.017453292519943295f;

void readJoystick(const joystick_t& js, float& outX, float& outY)
{
	outX = 0.0f;
	outY = 0.0f;

	const float magnitude = js.mag > 1.0f ? 1.0f : js.mag;
	if (magnitude > 0.0f && std::isfinite(magnitude) && std::isfinite(js.ang))
	{
		const float radians = js.ang * kDegToRad;
		outX = std::sin(radians) * magnitude;
		outY = std::cos(radians) * magnitude;
		return;
	}

	const int centerX = js.center.x;
	const int centerY = js.center.y;
	if (centerX == 0 && centerY == 0)
		return;

	const float rangeX = js.max.x > js.min.x ? (js.max.x - js.min.x) * 0.5f : 100.0f;
	const float rangeY = js.max.y > js.min.y ? (js.max.y - js.min.y) * 0.5f : 100.0f;

	outX = (static_cast<float>(js.pos.x) - static_cast<float>(centerX)) / rangeX;
	outY = (static_cast<float>(js.pos.y) - static_cast<float>(centerY)) / rangeY;

	if (outX > 1.0f) outX = 1.0f; else if (outX < -1.0f) outX = -1.0f;
	if (outY > 1.0f) outY = 1.0f; else if (outY < -1.0f) outY = -1.0f;
}
} // namespace

namespace WiiRemote
{

using namespace WiiPadInternal;

void poll(WiiPadInternal::FrameState& state)
{
	WiiInputDebugState& debug = wiiInputDebugState();

	WPADData *wd = WPAD_Data(WPAD_CHAN_0);
	if (!wd)
		return;

	// WPAD_Probe is the supported "what is plugged into this channel" query, and
	// it is consulted in preference to wd->exp.type because that field only
	// describes the last data report that arrived: around a hot-plug, or while a
	// handshake is still in flight, it can read WPAD_EXP_NONE with a Nunchuk very
	// much attached. Trusting it alone is one of the ways a Nunchuk ends up "not
	// working".
	//
	// But neither query gets to VETO the poll. This is the only presence check in
	// the function on purpose: a probe that fails for one frame, or an err field
	// holding some transient not-ready value, must not silence the buttons and
	// the pointer as well -- that would trade a Nunchuk bug for a dead Wiimote.
	// So the two are merged, and the poll continues regardless.
	u32 expType = WPAD_EXP_NONE;
	if (WPAD_Probe(WPAD_CHAN_0, &expType) != WPAD_ERR_NONE)
		expType = WPAD_EXP_NONE;

	debug.wmConnected = true;
	debug.expProbed   = (int)expType;

	if (expType == WPAD_EXP_NONE)
		expType = (u32)wd->exp.type;

	debug.expType   = (int)expType;
	debug.wmButtons = wd->btns_h;
	debug.irValid   = wd->ir.valid != 0;
	debug.irX       = (int)wd->ir.x;
	debug.irY       = (int)wd->ir.y;
	debug.irDots    = wd->ir.num_dots;
	state.textInputSnapshot.irValid = wd->ir.valid != 0;
	state.textInputSnapshot.irX = (int)wd->ir.x;
	state.textInputSnapshot.irY = (int)wd->ir.y;
	state.textInputSnapshot.irWidth = wiigl_width();
	state.textInputSnapshot.irHeight = wiigl_height();

	// btns_d is libogc's own "pressed this frame" mask, and it covers the
	// expansion bits too -- WPAD_ReadPending computes it as (held & ~last) over
	// the full 32-bit mask, of which the top half is the Nunchuk/Classic
	// buttons. So the pulse-style bindings below need no remembered state.
	const u32 held = wd->btns_h;
	const u32 down = wd->btns_d;

	if (platformPadRebindExclusive())
	{
		// GuiWiiControls is listening for a new binding. Report exactly one
		// pressed face button as a synthetic key-down event instead of its
		// normal job; the capture flow filters by family, so a press here is
		// ignored unless a Wiimote or Classic slot is the one being edited.
		static const struct { u32 mask; int code; } kRemappableWiimote[] = {
			{ WPAD_BUTTON_A, WII_KEY_WM_A }, { WPAD_BUTTON_B, WII_KEY_WM_B },
			{ WPAD_BUTTON_1, WII_KEY_WM_1 }, { WPAD_BUTTON_2, WII_KEY_WM_2 },
			{ WPAD_BUTTON_MINUS, WII_KEY_WM_MINUS },
			// Nunchuk C/Z bits are merged into the same held/down mask (see the
			// comment above `held`), so these are naturally never captured when
			// no Nunchuk is attached -- no expType check needed here.
			{ WPAD_NUNCHUK_BUTTON_C, WII_KEY_NUNCHUK_C }, { WPAD_NUNCHUK_BUTTON_Z, WII_KEY_NUNCHUK_Z },
		};
		static const struct { u32 mask; int code; } kRemappableClassic[] = {
			{ WPAD_CLASSIC_BUTTON_A, WII_KEY_CC_A }, { WPAD_CLASSIC_BUTTON_B, WII_KEY_CC_B },
			{ WPAD_CLASSIC_BUTTON_X, WII_KEY_CC_X }, { WPAD_CLASSIC_BUTTON_Y, WII_KEY_CC_Y },
			{ WPAD_CLASSIC_BUTTON_MINUS, WII_KEY_CC_MINUS },
		};
		int capturedCode = 0;
		for (const auto &entry : kRemappableWiimote)
			if (down & entry.mask) { capturedCode = entry.code; break; }
		if (!capturedCode && expType == WPAD_EXP_CLASSIC)
			for (const auto &entry : kRemappableClassic)
				if (down & entry.mask) { capturedCode = entry.code; break; }
		if (capturedCode)
		{
			lwjgl::Keyboard::detail::pushKey(capturedCode, true);
			lwjgl::Keyboard::detail::pushKey(capturedCode, false);
		}
		return;
	}

	// Nunchuk C/Z are part of the same held/down mask (see the comment above),
	// so Attack/Use/Sneak read correctly here whether or not a Nunchuk is
	// actually attached -- the bit is simply always 0 without one.
	const WiiButtonBindings::FamilySnapshot &wmButtons = WiiButtonBindings::get().wiimote;
	state.setKey(VK_JUMP,        (held & wmButtons.jump) != 0);
	state.setKey(VK_SNEAK,       (held & wmButtons.sneak) != 0);
	state.setKey(VK_INVENTORY,   (held & wmButtons.inventory) != 0);
	state.setKey(VK_THIRDPERSON, (held & wmButtons.thirdPerson) != 0);
	state.setKey(VK_ESCAPE,      (held & WPAD_BUTTON_PLUS) != 0);
	state.setKey(VK_ARROW_UP,    (held & WPAD_BUTTON_UP) != 0);
	state.setKey(VK_ARROW_DOWN,  (held & WPAD_BUTTON_DOWN) != 0);
	state.setKey(VK_ARROW_LEFT,  (held & WPAD_BUTTON_LEFT) != 0);
	state.setKey(VK_ARROW_RIGHT, (held & WPAD_BUTTON_RIGHT) != 0);
	// In a GUI (mouse not grabbed) the click button is always physical A,
	// regardless of what Attack is rebound to for gameplay -- same convention
	// as every other menu confirm button in the game (see PAD_CROSS on PS2).
	state.setMouse(VM_ATTACK,    lwjgl::Mouse::isGrabbed()
		? (held & wmButtons.attack) != 0
		: (held & WPAD_BUTTON_A) != 0);
	state.setMouse(VM_USE,       (held & wmButtons.use) != 0);

	if (held & WPAD_BUTTON_A)      state.textInputHeld |= WII_TEXT_TYPE;
	if (held & WPAD_BUTTON_B)      state.textInputHeld |= WII_TEXT_BACK;
	if (held & WPAD_BUTTON_PLUS)   state.textInputHeld |= WII_TEXT_ENTER;
	if (held & WPAD_BUTTON_MINUS)  state.textInputHeld |= WII_TEXT_SPACE;
	if (held & WPAD_BUTTON_1)      state.textInputHeld |= WII_TEXT_CLOSE;
	if (held & WPAD_BUTTON_2)      state.textInputHeld |= WII_TEXT_SHIFT;
	if (held & WPAD_BUTTON_UP)     state.textInputHeld |= WII_TEXT_UP;
	if (held & WPAD_BUTTON_DOWN)   state.textInputHeld |= WII_TEXT_DOWN;
	if (held & WPAD_BUTTON_LEFT)   state.textInputHeld |= WII_TEXT_LEFT;
	if (held & WPAD_BUTTON_RIGHT)  state.textInputHeld |= WII_TEXT_RIGHT;

	// Chat moved to the two-trigger chord below. Restore the simple utility
	// bindings: 1 = diagnostics, 2 = drop (by default; drop is rebindable).
	state.setKey(VK_DEBUG, (held & WPAD_BUTTON_1) != 0);
	state.setKey(VK_DROP,  (held & wmButtons.drop) != 0);

	// D-pad left/right double as hotbar prev/next, exactly as on the GameCube
	// pad -- see the note there for why binding this on top of the arrow keys
	// is safe.
	const bool alternativeGameplay = state.alternativeControls && lwjgl::Mouse::isGrabbed();
	if (!alternativeGameplay)
	{
		if (down & WPAD_BUTTON_RIGHT) state.wheel -= 1;
		if (down & WPAD_BUTTON_LEFT)  state.wheel += 1;
	}
	else
	{
		state.setKey(VK_FORWARD, (held & WPAD_BUTTON_UP) != 0);
		state.setKey(VK_BACK,    (held & WPAD_BUTTON_DOWN) != 0);
		state.setKey(VK_LEFT,    (held & WPAD_BUTTON_LEFT) != 0);
		state.setKey(VK_RIGHT,   (held & WPAD_BUTTON_RIGHT) != 0);
	}

	if (expType == WPAD_EXP_NUNCHUK)
	{
		const joystick_t &js = wd->exp.nunchuk.js;
		debug.jsAng     = js.ang;
		debug.jsMag     = js.mag;
		debug.jsRawX    = js.pos.x;
		debug.jsRawY    = js.pos.y;
		debug.jsCenterX = js.center.x;
		debug.jsCenterY = js.center.y;

		float x, y;
		readJoystick(wd->exp.nunchuk.js, x, y);
		state.stickSnapshot.connected = true;
		state.stickSnapshot.x = x;
		state.stickSnapshot.y = y;
		if (alternativeGameplay)
		{
			WiiPointer::addStickLook(state.applyDeadzone(x), -state.applyDeadzone(y));
		}
		else
		{
			state.moveX += state.applyDeadzone(x);
			state.moveY += state.applyDeadzone(y);
		}

		if (wd->exp.nunchuk.btns_held & NUNCHUK_BUTTON_Z)
			state.textInputHeld |= WII_TEXT_SPACE;
		if (wd->exp.nunchuk.btns_held & NUNCHUK_BUTTON_C)
			state.textInputHeld |= WII_TEXT_SHIFT;

		// Chat used to be a B+Z chord, but B/Z are now Jump/Attack by default
		// (see WiiButtonBindings.cpp) and jump-attack is a normal combat move,
		// so that chord fired chat by accident constantly. Plus+Minus doesn't
		// collide with any Attack/Use/Jump/Sneak assignment; the tradeoff is
		// that both already have their own job (pause, inventory) and still
		// fire alongside the chord -- holding them together opens chat *and*
		// pauses *and* opens the inventory.
		if ((held & (WPAD_BUTTON_PLUS | WPAD_BUTTON_MINUS)) == (WPAD_BUTTON_PLUS | WPAD_BUTTON_MINUS) &&
		    (down & (WPAD_BUTTON_PLUS | WPAD_BUTTON_MINUS)) &&
		    lwjgl::Mouse::isGrabbed() && !platformTextInputExclusive())
		{
			lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_T, true);
			lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_T, false);
		}
	}
	else if (expType == WPAD_EXP_CLASSIC)
	{
		const bool classicChatChord =
			(held & (WPAD_CLASSIC_BUTTON_FULL_L | WPAD_CLASSIC_BUTTON_FULL_R)) ==
			(WPAD_CLASSIC_BUTTON_FULL_L | WPAD_CLASSIC_BUTTON_FULL_R);
		if (classicChatChord &&
		    (down & (WPAD_CLASSIC_BUTTON_FULL_L | WPAD_CLASSIC_BUTTON_FULL_R)) &&
		    lwjgl::Mouse::isGrabbed() && !platformTextInputExclusive())
		{
			lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_T, true);
			lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_T, false);
		}
		const WiiButtonBindings::FamilySnapshot &ccButtons = WiiButtonBindings::get().classic;
		state.setKey(VK_JUMP,      (held & ccButtons.jump) != 0);
		state.setKey(VK_SNEAK,     (held & ccButtons.sneak) != 0);
		state.setKey(VK_INVENTORY, (held & ccButtons.inventory) != 0);
		state.setKey(VK_DROP,      (held & ccButtons.drop) != 0);
		state.setKey(VK_ESCAPE,    (held & WPAD_CLASSIC_BUTTON_PLUS) != 0);
		state.setKey(VK_DEBUG,     (held & WPAD_CLASSIC_BUTTON_MINUS) != 0);
		state.setKey(VK_ARROW_UP,    (held & WPAD_CLASSIC_BUTTON_UP) != 0);
		state.setKey(VK_ARROW_DOWN,  (held & WPAD_CLASSIC_BUTTON_DOWN) != 0);
		state.setKey(VK_ARROW_LEFT,  (held & WPAD_CLASSIC_BUTTON_LEFT) != 0);
		state.setKey(VK_ARROW_RIGHT, (held & WPAD_CLASSIC_BUTTON_RIGHT) != 0);
		state.setKey(VK_THIRDPERSON, (held & ccButtons.thirdPerson) != 0);
		state.setMouse(VM_USE,     !classicChatChord && (held & ccButtons.use) != 0);
		// In a GUI (mouse not grabbed) the click button is always physical A,
		// regardless of what Attack is rebound to for gameplay -- same
		// convention as every other menu confirm button in the game (see
		// PAD_CROSS on PS2).
		state.setMouse(VM_ATTACK,  lwjgl::Mouse::isGrabbed()
			? (!classicChatChord && (held & ccButtons.attack) != 0)
			: (held & WPAD_CLASSIC_BUTTON_A) != 0);

		if (held & WPAD_CLASSIC_BUTTON_A)      state.textInputHeld |= WII_TEXT_TYPE;
		if (held & WPAD_CLASSIC_BUTTON_B)      state.textInputHeld |= WII_TEXT_BACK;
		if (held & WPAD_CLASSIC_BUTTON_PLUS)   state.textInputHeld |= WII_TEXT_ENTER;
		if (held & WPAD_CLASSIC_BUTTON_MINUS)  state.textInputHeld |= WII_TEXT_SPACE;
		if (held & WPAD_CLASSIC_BUTTON_UP)     state.textInputHeld |= WII_TEXT_UP;
		if (held & WPAD_CLASSIC_BUTTON_DOWN)   state.textInputHeld |= WII_TEXT_DOWN;
		if (held & WPAD_CLASSIC_BUTTON_LEFT)   state.textInputHeld |= WII_TEXT_LEFT;
		if (held & WPAD_CLASSIC_BUTTON_RIGHT)  state.textInputHeld |= WII_TEXT_RIGHT;

		// ZL/ZR are the hotbar, for the same reason the GameCube pad puts it on
		// the D-pad: without it there is no way to change the selected item.
		if (down & WPAD_CLASSIC_BUTTON_ZR) state.wheel -= 1;
		if (down & WPAD_CLASSIC_BUTTON_ZL) state.wheel += 1;

		const classic_ctrl_t &cc = wd->exp.classic;
		float lx, ly, rx, ry;
		readJoystick(cc.ljs, lx, ly);
		readJoystick(cc.rjs, rx, ry);
		state.moveX += state.applyDeadzone(lx);
		state.moveY += state.applyDeadzone(ly);
		WiiPointer::addStickLook(state.applyDeadzone(rx), -state.applyDeadzone(ry));
	}

	// --- IR pointer -> look ---
	// Two different meanings, picked by whether the engine has the mouse:
	//
	//   grabbed (in game)  offset from centre is a RATE. Hold the Wiimote
	//                      pointing up-right and the camera keeps turning
	//                      up-right instead of stopping at the edge of the
	//                      screen. See applyIrLook.
	//   not grabbed (GUI)  the pointer IS the cursor, absolutely, because
	//                      pointing at a button has to put the cursor on it.
	//
	// ir.valid goes false the moment the player points away from the sensor bar.
	// Dropping the frame *and* forgetting the previous position is essential:
	// keeping it would turn the next re-acquisition into one enormous delta and
	// snap the camera across the world.
	//
	// Use ir.x / ir.y and nothing else. ir_t carries the pointer in THREE
	// different coordinate spaces and only the last one is the one we want:
	//
	//   ax, ay   raw sensor space
	//   sx, sy   smoothed, still an intermediate space
	//   x,  y    bounded -- the only pair expressed in the virtual resolution
	//            handed to WPAD_SetVRes (Display_wii.cpp passes the framebuffer
	//            size), which is the space the mouse cursor lives in
	//
	// Reading sx/sy instead looks like a free noise reduction and is not: the
	// deltas come out on a different scale, round to zero, and the camera then
	// sits in the middle of the screen and never turns.
	if (wd->ir.valid)
	{
		const int ix = (int)wd->ir.x;
		const int iy = (int)wd->ir.y;
		debug.irMag = WiiPointer::handleIr(ix, iy, !state.alternativeControls);
	}
	else
	{
		WiiPointer::loseIr();
	}
}

} // namespace WiiRemote

#endif // WII_PLATFORM
