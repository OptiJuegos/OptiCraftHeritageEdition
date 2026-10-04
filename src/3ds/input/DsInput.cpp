// DsInput.cpp -- touchscreen + circle-pad input for the 3DS port.
//
// One self-contained libctru scan per frame, shaped into the snapshot that
// platform/InputBackend_3DS.cpp feeds into the shared Input API, plus the
// key/mouse/wheel edges the engine actually plays with.
//
// A button means different things with a screen open and with it closed, so
// each one has up to two channels:
//
//   button  menu channel (PLATFORM_TEXT_*)   gameplay channel
//   ------  -------------------------------  --------------------------------
//   A       TYPE      confirm/select         jump      -> DS_KEY_A
//   B       BACK      cancel                 use       -> mouse button 1
//   X       SPACE     GUI/virtual-kbd space  attack    -> mouse button 0
//   Y       CLOSE     exit/back-out          inventory -> DS_KEY_Y
//   L       SPACE     left shoulder          use       -> mouse button 1 (place)
//   R       SHIFT     right shoulder         attack    -> mouse button 0 (destroy)
//   SELECT  SPACE     (PS2 SELECT/Wii MINUS) sneak     -> DS_KEY_SELECT
//   D-pad   UP/DOWN/LEFT/RIGHT               UP       -> open chat (multiplayer)
//                                                         LEFT/RIGHT -> hotbar wheel
//                                                         +1/-1; DOWN -> F5 (perspective)
//   START   ENTER     only while typing      pause     -> KEY_ESCAPE
//   ZL/ZR   --                               hotbar wheel +1/-1 (New 3DS only)
//
//   circle pad -> stick axes (analog movement, PLATFORM_DIRECT_ANALOG_MOVEMENT)
//   C-Stick  -> look pad in gameplay (New 3DS only; Old hardware reports a
//               zeroed position, so the channel is inert there). Deltas ride
//               the same touch-look pipeline as the face-button camera,
//               scaled by deflection at a frame-rate-free rate.
//   menu L/R -> their own DS_KEY_L/DS_KEY_R codes while a screen is up (they
//               cannot be SPACE/SHIFT, which the keyboard and the container
//               navigator already own); the creative screen maps them to
//               its category tabs.
//   touch      -> menus: absolute pointer + click. Gameplay: LOOK ONLY (panel
//                 drags move the camera; the triggers own the buttons). While
//                 a text field has focus the on-screen keyboard owns the
//                 panel and reads RAW 320x240 coordinates instead.
//
// FACE-BUTTON CAMERA (the OptiCraft-Options toggle, gameplay only): A/B/X/Y
// become a look pad (Y=left A=right X=up B=down) at a frame-rate-free rate,
// attack/use live on the shoulders alone, jump = SELECT tap or double-tap B,
// sneak = hold SELECT, inventory = double-tap Y. The deltas ride the touch
// look pipeline, so the sensitivity slider covers both.
//
// Attack and Use are mouse buttons rather than keys because that is what
// GameSettings binds them to (-100 / -99) and what clickMouse() reads -- and
// L and R are the shoulders as clicks, deliberately swapped from the PC's
// left/right mouse (the player's ask): L funnels into button 1 with B, R into
// button 0 with X (updateGameplay ORs the sources, so any hold order works).
// That is also why they are menu-suppressed: buttons pushed while a screen is
// open become a click under the cursor (GuiScreen::handleInput() drains the
// queue unconditionally), whereas the touch tap must keep working there.
//
// The gameplay channel is dropped while a screen is open, and a button still
// physically held across the menu/gameplay boundary stays dropped until it is
// released. Without that second half the press that confirmed a menu would
// arrive again as a gameplay button the moment the world opened -- exactly
// the bug WiiPadState.cpp documents as "joining a world bounced straight back
// to the title". The PS2 solves the same problem with releaseGameplayKeys().
//
// Coordinates: the top screen (400x240) is the game surface, the bottom
// panel (320x240) is touch. Touch X is scaled into top-screen pixels
// (x * screenW / 320); Y is copied as-is because both screens are 240 tall,
// so the GUI can compare pointer coordinates against 400x240 directly. The
// exception is text entry: while a field has focus the coordinates stay in
// the panel's own 320x240 space (no scaling, no mouse-queue forwarding),
// because the fallback keyboard lays itself out there -- see dsInputPoll().
#ifdef CTR_PLATFORM

#include "3ds/input/DsInput.h"

#include <3ds.h>

#include "3ds/input/DsPadKeyCodes.h"
#include "lwjgl/Keyboard.h"
#include "lwjgl/Mouse.h"
#include "platform/ConsoleInputClock.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

// The gameplay touch-HUD crafting button's open-request (consumed via
// platformConsumeTouchCraftRequest by the game side): the action opens a
// screen, so no key code is involved at all -- a fixed code hit OptiFine's
// zoom, and the user's crafting binding may be anything. File scope on
// purpose: InputBackend_3DS externs it from outside the anonymous namespace
// below.
bool g_touchCraftRequested = false;

// The gameplay pad's Pocket-Edition tap request (consumed via
// platformConsumeTouchPadTap by the game side, which turns it into place or
// swing by what the crosshair targets). File scope like the craft flag:
// InputBackend_3DS externs it.
bool g_touchPadTapRequested = false;

namespace
{
// Top-screen geometry remembered by dsInputInit(): touch arrives in panel
// pixels and has to be mapped into the same space the GUI lives in.
int g_screenW = 400;
int g_screenH = 240;

// dsInputDebugLine() reports nullptr until the input system has been set up
// (the "idle" case DsInput.h allows).
bool g_initialized = false;

DsInputState g_state = {};

// PLATFORM_TEXT_* rising edges accumulated between polls, cleared by
// dsInputConsumePressed(): the consume-on-read contract DsInput.h shares
// with the Wii's wiiTextInputConsumePressed().
std::uint32_t g_latchedPressed = 0;

// Previous poll's mapped mask, so edges are detected against the mask rather
// than raw keys -- one physical button can map to several bits (SELECT, L, X
// all set SPACE) or to none at all (START).
std::uint32_t g_prevHeld = 0;

// Previous poll's touch sample. The panel reports an ABSOLUTE contact point,
// but the mouse queue wants relative motion too, so deltas are differenced
// here -- the same job the Wii's IR producer does in WiiPointer.cpp.
#include "platform/TouchHudLayout.h"

// The gameplay touch-HUD widget key currently latched down (0 = none).
// Widget actions press the key on contact and release it on lift, at least
// one frame apart: pushing both edges into the same poll made Minecraft's
// keyboard drain process down+up together, and the state-based checks
// (inventory, hotbar slots) never saw the press at all -- the pause button
// alone worked, because its check is event-driven inside the drain loop.
int g_widgetKeyDown = 0;

// Press a gameplay touch-HUD widget's action key. The crafting button opens
// the inventory's own 2x2 grid -- the owner's call, the same screen
// inventory opens.
void pressTouchHudWidget(touchHud::WidgetHit hit)
{
	int key = 0;
	switch (hit.widget)
	{
	case touchHud::Widget::Hotbar:
		key = lwjgl::Keyboard::KEY_1 + hit.slot;
		break;
	case touchHud::Widget::Inventory:
		// The 3DS inventory binding rides the pad's Y-button code (the
		// same code the physical Y pushes through the gameplay channel);
		// KEY_E only reached a desktop binding nobody re-set.
		key = DS_KEY_Y;
		break;
	case touchHud::Widget::Crafting:
		// Handled on the game side (GuiIngame opens the legacy crafting
		// screen directly through platformConsumeTouchCraftRequest): a
		// fixed key code reached OptiFine's zoom on real settings, and the
		// crafting binding itself may be anything.
		g_touchCraftRequested = true;
		return;
	case touchHud::Widget::Pause:
		key = lwjgl::Keyboard::KEY_ESCAPE;
		break;
	default:
		return;
	}
	if (g_widgetKeyDown != 0 && g_widgetKeyDown != key)
		lwjgl::Keyboard::detail::pushKey(g_widgetKeyDown, false);
	g_widgetKeyDown = key;
	lwjgl::Keyboard::detail::pushKey(key, true);
}

// Release the latched widget key: on finger lift, on sliding off the
// widget, or when a menu takes the panel away.
void releaseTouchHudWidget()
{
	if (g_widgetKeyDown != 0)
	{
		lwjgl::Keyboard::detail::pushKey(g_widgetKeyDown, false);
		g_widgetKeyDown = 0;
	}
}

bool g_prevTouchDown = false;
// True while the previous touch contact sat on a gameplay touch-HUD
// widget: widget contacts never feed the camera, and leaving one must
// restart the pointer publish as a fresh contact rather than difference
// against the stale pre-widget sample.
bool g_prevTouchOnWidget = false;
// True while the CURRENT contact began on a gameplay touch-HUD widget:
// that contact is what opens the widget's screen (craft, inventory,
// pause), and while it stays down it must not also become the new menu's
// first phantom click at the button position.
bool g_contactStartedOnWidget = false;
// Pocket-Edition-style pad gestures (gameplay, non-widget contacts only):
// a short stationary touch taps (see g_touchPadTapRequested), a longer
// stationary hold breaks (button 0 held while g_padBreakActive), and
// moving the contact turns it back into the camera drag it always was.
s64 g_padContactStartMs = 0;
int g_padStartX = 0;
int g_padStartY = 0;
bool g_padTapArmed = false;
bool g_padBreakActive = false;
// The "Touch Click" toggle (stored settings key pocketTouch): while false
// the PE gestures above never arm, so the pad degrades to the plain camera
// drag it was before they existed.
bool g_pocketTouch = true;
int g_prevTouchX = 0;
int g_prevTouchY = 0;

// Last poll's text-exclusive state (a field focused). dsInputPoll compares
// against it to spot the transitions that change what touch and START mean.
bool g_prevTextExclusive = false;

// Last poll's container-navigation state: the same transition detection for
// the B button's split-half role and the START escape below.
bool g_prevContainerNav = false;

// Face-button camera (the OptiCraft-Options toggle): while on, in gameplay
// the diamond becomes a look pad (Y left, A right, X up, B down), attack and
// place live only on the shoulders, SELECT taps jump / holds sneak, and a
// double tap of B or Y still fires jump / inventory. Deltas ride the same
// mouse-queue look pipeline the touch panel uses, so sensitivity settings
// apply to both alike.
bool g_faceButtonCamera = false;
std::uint32_t g_faceCameraMask = 0;      // face buttons present in face mode
int g_selectDownSinceMs = 0;              // 0 = not down
bool g_selectSneakLatched = false;
int g_lastFaceJumpTapMs = 0;              // last B tap edge, for double-tap
int g_lastFaceInventoryTapMs = 0;        // last Y tap edge, for double-tap
int g_faceCameraLastMs = 0;              // poll timestamp, for frame-rate-free rates
int g_faceJumpHoldFrames = 0;            // polls left on a synthesized jump press

constexpr int kFaceSneakHoldMs = 350;     // SELECT held this long = sneak
constexpr int kFaceDoubleTapMs = 300;    // taps closer than this = double
// How many polls a synthesized jump stays pressed. Jump is level-sampled
// (the movement input reads keyBindJump->pressed inside the entity tick),
// unlike the inventory's edge-consumed isPressed(): a down+up pair pushed
// in the same poll nets to false before the tick ever looks. Five polls is
// ~one game tick or more at 30 and 60 fps alike -- about the hold of a real
// button tap.
constexpr int kFaceJumpHoldPolls = 5;
// Look rate in mouse pixels per second: the default sensitivity cube is 1.0,
// so one pixel is 0.15 degrees -- 480 px/s is a comfortable ~72 deg/s sweep.
// Shared by the face-button camera and the New 3DS C-Stick channel below,
// which scale it by how far the input is pushed.
constexpr float kFaceCameraPixelsPerSec = 480.0f;

// New 3DS C-Stick camera state. The nub self-centres well, but a resting
// offset must not creep the view, so the same deadzone+rescale the published
// stick axes get (InputBackend_3DS) is applied before any rate scaling --
// the camera reads DsInputState's RAW axes, because the deadzone lives in the
// backend half and must not be applied twice.
constexpr float kCStickDeadzone = 0.20f;
int g_cstickCameraLastMs = 0;          // poll timestamp, frame-rate-free rate
float g_cstickAccumX = 0.0f;           // sub-pixel remainders: a gentle nudge
float g_cstickAccumY = 0.0f;           // at 60 fps moves <1 px per poll

// Gameplay channel state. Separate from PLATFORM_TEXT_*, which is the menu
// channel and keeps working with a screen open.
//
// GP_* bits are one per *action*, not per physical button, because attack and
// use are mouse buttons while jump and friends are keys and they need
// different edge handling.
constexpr std::uint32_t GP_JUMP        = 1u << 0;
constexpr std::uint32_t GP_INVENTORY   = 1u << 1;
constexpr std::uint32_t GP_SNEAK       = 1u << 2;
constexpr std::uint32_t GP_USE         = 1u << 3; // mouse button 1 (B or L)
constexpr std::uint32_t GP_ATTACK      = 1u << 4; // mouse button 0 (X or R)
constexpr std::uint32_t GP_DPAD_UP     = 1u << 5;
constexpr std::uint32_t GP_DPAD_DOWN   = 1u << 6;
constexpr std::uint32_t GP_DPAD_LEFT   = 1u << 7; // wheel +1 (previous slot)
constexpr std::uint32_t GP_DPAD_RIGHT  = 1u << 8; // wheel -1 (next slot)
// KEY_B alone. Gameplay folds B into GP_USE together with L, but the menu
// navigation below must not: L is the SPACE/SHIFT pair's left half there
// (the legacy crafting screen's category tabs), and riding GP_USE made it
// push KEY_ESCAPE -- one shoulder switched category while the other closed
// the inventory (2026-09-29, 3DS).
constexpr std::uint32_t GP_BACK        = 1u << 9;
// New 3DS triggers. They share the hotbar wheel with the D-pad's horizontal
// pair rather than folding into GP_DPAD_LEFT/RIGHT: the D-pad bits also
// carry menu navigation while a screen is open, and ZL/ZR must not step the
// GUI. The bits stay zero on Old hardware (KEY_ZL/KEY_ZR never report).
constexpr std::uint32_t GP_SLOT_PREV   = 1u << 10; // ZL: wheel +1 (previous slot)
constexpr std::uint32_t GP_SLOT_NEXT   = 1u << 11; // ZR: wheel -1 (next slot)

// Latched by dsInputPoll() from the screen-open state Display hands it; the
// input layer cannot discover that for itself (see DsInput.h).
bool g_inMenu = false;
bool g_prevInMenu = false;

// Buttons held across a menu/gameplay boundary: ignored until physically
// released, so a confirm press is not re-delivered as a gameplay press.
std::uint32_t g_suppressed = 0;

// Last gameplay mask actually delivered, so keys/mouse move only on a change
// -- the same diffing WiiPadState::flushKeysAndButtons() does.
std::uint32_t g_prevGameplay = 0;

// Menu-context navigation edges (see updateGameplay): the D-pad and A/B
// pushed as keyboard arrow/return/escape codes while a screen is up.
std::uint32_t g_prevMenuNav = 0;

// The menu shoulders' own edge state. L/R cannot ride the arrows' channel:
// in menus they mean SPACE/SHIFT to the on-screen keyboard and the container
// navigator, so screens that want them as buttons (the creative screen's
// category tabs) get them as their dedicated DS_KEY_* pad codes instead.
std::uint32_t g_prevMenuShoulders = 0;

// Mouse button levels. Button 0 is driven by the touch tap OR GP_ATTACK, so
// they are tracked together rather than per source.
bool g_prevBtn0 = false;
bool g_prevBtn1 = false;

// The bottom touch panel: fixed 320x240 hardware on every 3DS model.
constexpr int kTouchPanelW = 320;

// circlePosition carries no named maximum in libctru (hid.h declares dx/dy
// as plain s16 with no range documentation), and the pad saturates around
// +/-156 on both axes, so normalise against that directly.
constexpr float kCirclePadMax = 156.0f;

char g_debugLine[112];

std::uint32_t mapTextButtons(u32 keys)
{
	std::uint32_t value = 0;
	if (keys & KEY_DUP)    value |= PLATFORM_TEXT_UP;
	if (keys & KEY_DDOWN)  value |= PLATFORM_TEXT_DOWN;
	if (keys & KEY_DLEFT)  value |= PLATFORM_TEXT_LEFT;
	if (keys & KEY_DRIGHT) value |= PLATFORM_TEXT_RIGHT;
	if (keys & KEY_A)      value |= PLATFORM_TEXT_TYPE;
	// In a container B is the close button (B/START push KEY_ESCAPE below --
	// the same back convention as every other menu), so the navigator's
	// split-half/place-one click moves to X for the container's lifetime;
	// anywhere else B stays the keyboard's backspace and X the space bar.
	const bool containerNav = platformContainerNavigationActive();
	if ((keys & KEY_B) && !containerNav) value |= PLATFORM_TEXT_BACK;
	if (keys & KEY_X)      value |= (containerNav ? PLATFORM_TEXT_BACK : PLATFORM_TEXT_SPACE);
	if (keys & KEY_Y)      value |= PLATFORM_TEXT_CLOSE;
	if (keys & KEY_L)      value |= PLATFORM_TEXT_SPACE;
	if (keys & KEY_R)      value |= PLATFORM_TEXT_SHIFT;
	if (keys & KEY_SELECT) value |= PLATFORM_TEXT_SPACE;
	// START is context-split rather than absent: while a field has focus it
	// is ENTER -- the submit key VirtualKeyboard::tick pushes as KEY_RETURN
	// (so A can send chat after the system keyboard filled the field) --
	// and otherwise forwardStartToEscape() keeps its fixed KEY_ESCAPE role.
	if ((keys & KEY_START) && platformTextInputExclusive())
		value |= PLATFORM_TEXT_ENTER;
	// ZL/ZR carry no menu meaning: they are the hotbar wheel in gameplay
	// (readGameplayButtons) and the D-pad owns menu scrolling. Pure D-pad
	// bits otherwise: the circle pad is analog movement and reaches the game
	// through the stick axes, not through this mask; the D-pad's gameplay
	// roles are the hotbar wheel and chat (see updateGameplay()), and DOWN
	// has no gameplay action at all.
	return value;
}

// The physical buttons that carry a gameplay action (header table). Kept
// separate from mapTextButtons(): this mask is what inMenu gates.
std::uint32_t readGameplayButtons(u32 keys)
{
	std::uint32_t value = 0;
	if (keys & KEY_A)      value |= GP_JUMP;
	if (keys & KEY_B)      value |= GP_USE | GP_BACK;
	if (keys & KEY_X)      value |= GP_ATTACK;
	if (keys & KEY_Y)      value |= GP_INVENTORY;
	// The shoulders are clicks, deliberately swapped from the PC's mouse
	// (the player's ask): L places (joins B on button 1), R attacks (joins X
	// on button 0) -- see the header table.
	if (keys & KEY_L)      value |= GP_USE;
	if (keys & KEY_R)      value |= GP_ATTACK;
	if (keys & KEY_SELECT) value |= GP_SNEAK;
	if (keys & KEY_DUP)    value |= GP_DPAD_UP;
	if (keys & KEY_DDOWN)  value |= GP_DPAD_DOWN;
	if (keys & KEY_DLEFT)  value |= GP_DPAD_LEFT;
	if (keys & KEY_DRIGHT) value |= GP_DPAD_RIGHT;
	// New 3DS only: the extra triggers step the hotbar alongside the D-pad's
	// horizontal pair (see the GP_SLOT_* declaration for why they are separate
	// bits). Zero on Old hardware, so nothing changes there.
	if (keys & KEY_ZL)     value |= GP_SLOT_PREV;
	if (keys & KEY_ZR)     value |= GP_SLOT_NEXT;
	return value;
}

// The circle pad's menu direction as GP_DPAD_* bits: the dominant axis past
// a deliberate-push threshold on the RAW axes (the deadzone rescale happens
// downstream in InputBackend_3DS; 0.5 raw is about a half push). stickY is
// down-positive (see the circle-pad block in dsInputPoll).
std::uint32_t menuStickNavBits()
{
	constexpr float kMenuStickThreshold = 0.5f;
	int x = 0;
	int y = 0;
	if (g_state.stickX < -kMenuStickThreshold) x = -1;
	else if (g_state.stickX > kMenuStickThreshold) x = 1;
	if (g_state.stickY > kMenuStickThreshold) y = 1;
	else if (g_state.stickY < -kMenuStickThreshold) y = -1;
	if (x != 0 && y != 0)
	{
		if (std::abs(g_state.stickX) >= std::abs(g_state.stickY)) y = 0;
		else x = 0;
	}
	std::uint32_t bits = 0;
	if (x < 0) bits |= GP_DPAD_LEFT;
	if (x > 0) bits |= GP_DPAD_RIGHT;
	if (y < 0) bits |= GP_DPAD_UP;
	if (y > 0) bits |= GP_DPAD_DOWN;
	return bits;
}

// The same direction as the PLATFORM_TEXT_* bits the text mask carries, so
// the stick rides the exact channel the D-pad already owns there.
std::uint32_t menuStickTextBits(std::uint32_t navBits)
{
	std::uint32_t bits = 0;
	if (navBits & GP_DPAD_LEFT)  bits |= PLATFORM_TEXT_LEFT;
	if (navBits & GP_DPAD_RIGHT) bits |= PLATFORM_TEXT_RIGHT;
	if (navBits & GP_DPAD_UP)    bits |= PLATFORM_TEXT_UP;
	if (navBits & GP_DPAD_DOWN)  bits |= PLATFORM_TEXT_DOWN;
	return bits;
}

// Deliver the gameplay half of this frame's scan. `touchDown` comes in
// because button 0 has two sources and the finger is exempt from inMenu: it
// is the pointer, and it has to keep clicking screens.
//
// Ordering matters and is deliberate: the touch position is published before
// this runs, so every click below lands where the finger is.
void updateGameplay(u32 keys, bool touchDown)
{
	const std::uint32_t held = readGameplayButtons(keys);
	// The circle pad's menu direction, for the keyboard-code channel below.
	const std::uint32_t stickBits = menuStickNavBits();

	if (g_inMenu != g_prevInMenu)
	{
		// Anything down at the boundary keeps its old meaning until it is
		// physically released (header comment; WiiPadState's g_suppressedKeys
		// is the same rule).
		g_suppressed |= held;
		g_prevInMenu = g_inMenu;
		// Latched menu edges must not cross the boundary in either
		// direction -- Ps2InputMapper clears them on both transitions.
		g_latchedPressed = 0;
		// The navigation pushes below seed from here, so a button held at
		// the boundary does not fire a menu navigation step it was never
		// pressed for; it navigates on its next fresh press instead. The
		// stick seeds the same way: walking with the pad pushed is the
		// normal way a pause screen opens, and the menu must not step on
		// the deflection it opened with.
		g_prevMenuNav = held | stickBits;
		g_prevMenuShoulders = g_inMenu ? (keys & (KEY_L | KEY_R)) : 0u;
	}
	g_suppressed &= held; // forget buttons that have since been released

	// Gameplay does not use the text/menu latches. Drop them every frame so
	// an in-world press cannot be replayed as a menu press later (the same
	// per-frame clear Ps2InputMapper does after updateGameplay()).
	if (!g_inMenu)
		g_latchedPressed = 0;

	const std::uint32_t active = g_inMenu ? 0u : (held & ~g_suppressed);
	const std::uint32_t changed = active ^ g_prevGameplay;
	const std::uint32_t pressed = active & ~g_prevGameplay;

	// Keys are pushed on every edge, both down and up, so isKeyDown() and
	// KeyBinding::pressed stay truthful and nothing is left stuck down. The
	// D-pad is deliberately absent: it stopped carrying movement (the circle
	// pad owns that) and now feeds the hotbar wheel below in gameplay and
	// menu navigation when a screen is up.
	if (changed & GP_JUMP)      lwjgl::Keyboard::detail::pushKey(DS_KEY_A, (active & GP_JUMP) != 0);
	if (changed & GP_INVENTORY) lwjgl::Keyboard::detail::pushKey(DS_KEY_Y, (active & GP_INVENTORY) != 0);
	if (changed & GP_SNEAK)     lwjgl::Keyboard::detail::pushKey(DS_KEY_SELECT, (active & GP_SNEAK) != 0);

	// Menu navigation (the console's menu schema). The legacy screens'
	// keyTyped handlers listen for the keyboard arrow/return/escape codes,
	// which nothing else on this console emits -- the gameplay pushes above
	// are gated off in menus by design, so the D-pad and A never reached them
	// and the menu sat there deaf to the D-pad. In menu context those buttons
	// deliver the navigation codes directly, both edges like every push
	// above; in gameplay the same buttons keep their action meanings through
	// the key bindings and these codes are not pushed at all.
	//
	// Nothing goes out while a field has focus: the text channel
	// (VirtualKeyboard::tick) is the only reader meant to act on those
	// buttons, and the screen underneath would otherwise act too -- chat
	// submitted on the very first A press that way. g_prevMenuNav keeps
	// tracking the buttons anyway, so one held across focus arriving does
	// not fire a stale step when focus goes away; it navigates on its next
	// fresh press instead (same rule as g_suppressed at the menu boundary).
	// START's KEY_ESCAPE is decided in dsInputPoll(), and B here is the
	// screens' own back button.
	const bool typing = platformTextInputExclusive();
	// The stick rides the same D-pad navigation bits -- one mechanism for
	// both, and every menu that answers the D-pad answers the stick.
	const std::uint32_t navActive = g_inMenu
	    ? ((held | stickBits) & (GP_DPAD_UP | GP_DPAD_DOWN | GP_DPAD_LEFT | GP_DPAD_RIGHT |
	                             GP_JUMP | GP_BACK))
	    : 0u;
	const std::uint32_t navChanged = navActive ^ g_prevMenuNav;
	if (!typing)
	{
		if (navChanged & GP_DPAD_UP)    lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_UP, (navActive & GP_DPAD_UP) != 0);
		if (navChanged & GP_DPAD_DOWN)  lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_DOWN, (navActive & GP_DPAD_DOWN) != 0);
		if (navChanged & GP_DPAD_LEFT)  lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_LEFT, (navActive & GP_DPAD_LEFT) != 0);
		if (navChanged & GP_DPAD_RIGHT) lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_RIGHT, (navActive & GP_DPAD_RIGHT) != 0);
		if (navChanged & GP_JUMP)       lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_RETURN, (navActive & GP_JUMP) != 0);
		// B alone stays the screens' back button everywhere -- containers
		// included (their split-half/place-one click rides X instead while a
		// container is open; see mapTextButtons). PS2/Wii keep their own
		// back buttons live during container navigation too. L shares
		// GP_USE with B in gameplay, but in a menu it is the keyboard
		// pair's left half (SPACE: the crafting screen's category tabs),
		// so the escape rides B's dedicated GP_BACK bit instead.
		if ((navChanged & GP_BACK) != 0)
			lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_ESCAPE, (navActive & GP_BACK) != 0);
	}
	g_prevMenuNav = navActive;

	// Menu shoulders: L/R pushed as their dedicated DS_KEY_* pad codes so a
	// screen can bind them (the creative screen's category tabs) without
	// touching the SPACE/SHIFT pair the on-screen keyboard and the container
	// navigator own. Edge-driven like the arrows above, seeded at the menu
	// boundary for the same reason, and excluded from the Controls screen's
	// rebind listener -- gameplay L/R are mouse clicks, so a binding nothing
	// can emit would be a dead entry in that screen.
	const std::uint32_t menuShoulders = g_inMenu ? (keys & (KEY_L | KEY_R)) : 0u;
	const std::uint32_t shoulderChanged = menuShoulders ^ g_prevMenuShoulders;
	if (!typing && !platformPadRebindExclusive() && shoulderChanged != 0)
	{
		if (shoulderChanged & KEY_L)
			lwjgl::Keyboard::detail::pushKey(DS_KEY_L, (menuShoulders & KEY_L) != 0);
		if (shoulderChanged & KEY_R)
			lwjgl::Keyboard::detail::pushKey(DS_KEY_R, (menuShoulders & KEY_R) != 0);
	}
	g_prevMenuShoulders = menuShoulders;

	const int x = g_state.pointerX;
	const int y = g_state.pointerY;

	// The wheel is an impulse rather than a state, so it fires on the press
	// only. Signs match Ps2InputMapper (R1 -> -1, L1 -> +1) and
	// InventoryPlayer::changeCurrentItem() subtracts its argument, so LEFT
	// steps the hotbar back and RIGHT steps it forward. The D-pad owns the
	// wheel in gameplay precisely because L/R clicked away to mouse buttons
	// (header table). ZL/ZR (New 3DS) join the same wheel as dedicated
	// impulse bits, so the player can hold the D-pad free for the camera.
	if (pressed & (GP_DPAD_LEFT | GP_SLOT_PREV))  lwjgl::Mouse::detail::pushWheel(1, x, y);
	if (pressed & (GP_DPAD_RIGHT | GP_SLOT_NEXT)) lwjgl::Mouse::detail::pushWheel(-1, x, y);

	// Chat: KEY_T is what keyBindChat is bound to (GameSettings' fixed
	// default), and pushing it from a pad button is how the Wii already does
	// this (WiiRemote.cpp's Plus+Minus chord). D-pad UP carries no gameplay
	// action since the wheel took LEFT/RIGHT, so it opens the chat where the
	// game has one (Minecraft::runTick opens it in multiplayer). Edge-driven,
	// and unreachable with a screen open: this whole channel is off in menus.
	if (pressed & GP_DPAD_UP)
	{
		lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_T, true);
		lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_T, false);
	}

	// D-pad DOWN had no gameplay action either (header table) and no face
	// button was spare, so it takes the desktop's F5: cycling the player's
	// perspective (Minecraft::runTick). Both edges like the chat push, and
	// menus keep the D-pad for navigation so this stays gameplay-only.
	if (pressed & GP_DPAD_DOWN)
	{
		lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_F5, true);
		lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_F5, false);
	}

	// Mouse buttons are a level. `touchDown` here is the MENU pointer click
	// (see dsInputPoll): gameplay never passes it, so button 0 in game is
	// the trigger channel and the pad's Pocket-Edition hold alone, and any
	// hold order between L and a finger on the panel cannot release one
	// with the other. The pad hold routes by what the crosshair targets
	// (platformCrosshairTargetsBlock, updated by GuiIngame each frame):
	// a block breaks (button 0), air uses the held item (button 1 -- eat
	// food, draw bow, block with sword).
	const bool padHoldOnBlock = g_padBreakActive && platformCrosshairTargetsBlock();
	const bool padHoldOnAir = g_padBreakActive && !platformCrosshairTargetsBlock();
	const bool want0 = touchDown || (active & GP_ATTACK) != 0 || padHoldOnBlock;
	const bool want1 = (active & GP_USE) != 0 || padHoldOnAir;
	if (want0 != g_prevBtn0)
	{
		lwjgl::Mouse::detail::pushButton(0, want0, x, y);
		g_prevBtn0 = want0;
	}
	if (want1 != g_prevBtn1)
	{
		lwjgl::Mouse::detail::pushButton(1, want1, x, y);
		g_prevBtn1 = want1;
	}

	g_prevGameplay = active;
}

// The face-button camera channel: called from dsInputPoll() only in gameplay
// with the toggle on. The four face buttons never reach updateGameplay() in
// that mode (dsInputPoll strips them first), so this is their whole meaning.
// Begin a synthesized jump: press now, release kFaceJumpHoldPolls later.
// The hold (not a same-poll edge) is what makes it a jump -- see the
// counter's declaration above.
void startFaceJumpHold()
{
	lwjgl::Keyboard::detail::pushKey(DS_KEY_A, true);
	g_faceJumpHoldFrames = kFaceJumpHoldPolls;
}

void updateFaceButtonCamera(u32 keys, u32 keysPressed, u32 keysReleased)
{
	(void)keysReleased;
	const int now = consoleInputNowMs();

	// Expire a synthesized jump before anything else, so the level is
	// false again by the poll the hold ends on.
	if (g_faceJumpHoldFrames > 0 && --g_faceJumpHoldFrames == 0)
		lwjgl::Keyboard::detail::pushKey(DS_KEY_A, false);

	// Look pad: hold to pan, at a frame-rate-free rate. The signs mirror a
	// touch drag exactly (right-drag = look right, down-drag = look down),
	// because both ride the same pushMotion -> mouseXYChange -> turnEntity
	// pipeline and share the sensitivity settings.
	int dx = 0;
	int dy = 0;
	if (keys & KEY_A) dx += 1;
	if (keys & KEY_Y) dx -= 1;
	if (keys & KEY_B) dy += 1;
	if (keys & KEY_X) dy -= 1;
	if (dx != 0 || dy != 0)
	{
		int elapsedMs = now - g_faceCameraLastMs;
		if (elapsedMs < 0)
			elapsedMs = 0;
		if (elapsedMs > 100)
			elapsedMs = 100;
		const float pixels =
			kFaceCameraPixelsPerSec * static_cast<float>(elapsedMs) / 1000.0f;
		lwjgl::Mouse::detail::pushMotion(g_state.pointerX, g_state.pointerY,
			static_cast<int>(dx * pixels), static_cast<int>(dy * pixels));
	}
	g_faceCameraLastMs = now;

	// SELECT: tap = jump, hold = sneak. The level push (and its release) is
	// what the ordinary sneak channel does; a tap that turns out to be a
	// hold must not also jump, so the jump fires on release instead.
	if (keysPressed & KEY_SELECT)
	{
		g_selectDownSinceMs = now;
		g_selectSneakLatched = false;
	}
	else if (keys & KEY_SELECT)
	{
		if (!g_selectSneakLatched &&
		    now - g_selectDownSinceMs >= kFaceSneakHoldMs)
		{
			g_selectSneakLatched = true;
			lwjgl::Keyboard::detail::pushKey(DS_KEY_SELECT, true);
		}
	}
	else if (g_selectDownSinceMs != 0)
	{
		if (g_selectSneakLatched)
			lwjgl::Keyboard::detail::pushKey(DS_KEY_SELECT, false);
		else
		{
			// A short tap: jump, held for a few polls rather than edged,
			// so the level survives until the entity tick reads it.
			startFaceJumpHold();
		}
		g_selectDownSinceMs = 0;
	}

	// Double-tap B = jump, double-tap Y = inventory: one tap is a camera
	// blip, two quick ones fire the action.
	if (keysPressed & KEY_B)
	{
		if (now - g_lastFaceJumpTapMs < kFaceDoubleTapMs)
		{
			startFaceJumpHold();
			g_lastFaceJumpTapMs = 0;
		}
		else
			g_lastFaceJumpTapMs = now;
	}
	if (keysPressed & KEY_Y)
	{
		if (now - g_lastFaceInventoryTapMs < kFaceDoubleTapMs)
		{
			lwjgl::Keyboard::detail::pushKey(DS_KEY_Y, true);
			lwjgl::Keyboard::detail::pushKey(DS_KEY_Y, false);
			g_lastFaceInventoryTapMs = 0;
		}
		else
			g_lastFaceInventoryTapMs = now;
	}
}

// Deadzone + rescale for the C-Stick, mirroring InputBackend_3DS's
// applyStickDeadzone: the camera consumes the RAW axes (deadzones are
// downstream for the shared snapshot, per DsInput.h), so the filter has to
// happen here for this channel.
float cStickDeflection(float raw)
{
	if (raw > -kCStickDeadzone && raw < kCStickDeadzone)
		return 0.0f;
	const float sign = raw < 0.0f ? -1.0f : 1.0f;
	float magnitude = (std::abs(raw) - kCStickDeadzone) / (1.0f - kCStickDeadzone);
	if (magnitude < 0.0f)
		magnitude = 0.0f;
	if (magnitude > 1.0f)
		magnitude = 1.0f;
	return magnitude * sign;
}

// The New 3DS C-Stick look channel: a deflection-scaled look pad riding the
// same pushMotion -> mouseXYChange -> turnEntity pipeline as the touch panel
// and the face-button camera, so the sensitivity slider and the invert
// option cover all three alike. dsInputPoll() calls it in gameplay only --
// menus keep the nub inert (the D-pad and circle pad own navigation there)
// and so does text entry. On Old hardware irrstCstickRead reports libctru's
// zeroed cache, both deflections stay 0, and nothing ever fires.
void updateCStickCamera()
{
	const int now = consoleInputNowMs();
	int elapsedMs = now - g_cstickCameraLastMs;
	g_cstickCameraLastMs = now;
	if (elapsedMs < 0)
		elapsedMs = 0;
	if (elapsedMs > 100)
		elapsedMs = 100;

	const float deflX = cStickDeflection(g_state.cstickX);
	const float deflY = cStickDeflection(g_state.cstickY);
	if (deflX == 0.0f && deflY == 0.0f)
	{
		// Drop the remainders too: re-centering the nub must not replay the
		// half-pixel a previous poll left behind as one last twitch.
		g_cstickAccumX = 0.0f;
		g_cstickAccumY = 0.0f;
		return;
	}

	// Signs mirror a touch drag exactly (right-drag = look right, down-drag =
	// look down) because both ride the same pipeline. cstickY already follows
	// the stick axes' down-positive convention (the circle-pad block in
	// dsInputPoll negates libctru's upward dy), so it maps straight on.
	const float pixels =
		kFaceCameraPixelsPerSec * static_cast<float>(elapsedMs) / 1000.0f;
	g_cstickAccumX += deflX * pixels;
	g_cstickAccumY += deflY * pixels;
	const int dx = static_cast<int>(g_cstickAccumX);
	const int dy = static_cast<int>(g_cstickAccumY);
	g_cstickAccumX -= static_cast<float>(dx);
	g_cstickAccumY -= static_cast<float>(dy);
	if (dx != 0 || dy != 0)
		lwjgl::Mouse::detail::pushMotion(g_state.pointerX, g_state.pointerY, dx, dy);
}

// START edges -> KEY_ESCAPE on the keyboard queue, both down and up so
// isKeyDown(KEY_ESCAPE) stays truthful. This split (escape on the key queue,
// nothing in the held mask) is the fixed pause/back role: the PS2's START
// carries ENTER as well as part of its fuller mapping, and this console's
// gains ENTER only while a field has focus (mapTextButtons), so the escape
// forwarding below never runs at the same time as the enter bit.
//
// The parameters are NOT called keysDown/keysUp: libctru's hid.h defines
// those as compatibility macros expanding to hidKeysDown/hidKeysUp, which
// would silently rewrite every use of the name.
void forwardStartToEscape(u32 keysPressed, u32 keysReleased)
{
	if (keysPressed & KEY_START)
		lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_ESCAPE, true);
	if (keysReleased & KEY_START)
		lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_ESCAPE, false);
}
} // namespace

void dsInputInit(int screenW, int screenH)
{
	// Idempotent by contract (Display::create may run again): reset the
	// whole snapshot so a re-entry doesn't replay stale edges or a dangling
	// touch position.
	if (screenW > 0)
		g_screenW = screenW;
	if (screenH > 0)
		g_screenH = screenH;
	g_initialized = true;
	g_state = DsInputState{};
	g_latchedPressed = 0;
	g_prevHeld = 0;
	g_prevTouchDown = false;
	g_prevTouchX = 0;
	g_prevTouchY = 0;
	g_prevTextExclusive = false;
	g_prevContainerNav = false;
	g_faceCameraMask = 0;
	g_selectDownSinceMs = 0;
	g_selectSneakLatched = false;
	g_lastFaceJumpTapMs = 0;
	g_lastFaceInventoryTapMs = 0;
	g_faceCameraLastMs = 0;
	g_faceJumpHoldFrames = 0;
	g_cstickCameraLastMs = 0;
	g_cstickAccumX = 0.0f;
	g_cstickAccumY = 0.0f;
	g_inMenu = false;
	g_prevInMenu = false;
	g_suppressed = 0;
	g_prevGameplay = 0;
	g_prevMenuNav = 0;
	g_prevMenuShoulders = 0;
	g_prevBtn0 = false;
	g_prevBtn1 = false;
}

void dsInputSetFaceButtonCamera(bool enabled)
{
	g_faceButtonCamera = enabled;
}

void dsInputSetPocketTouch(bool enabled)
{
	g_pocketTouch = enabled;
	// Turning it off mid-gesture must not leave a tap armed or a break running
	// on the old setting: the pad would keep placing/holding until that finger
	// lifted. Contacts still in progress degrade to a plain camera drag.
	if (!g_pocketTouch)
	{
		g_padTapArmed = false;
		g_padBreakActive = false;
	}
}

void dsInputPoll(bool inMenu)
{
	// The scan lives here so the poll is self-contained: HID state is latched
	// per scan, so every read below must come from the same one, and callers
	// (lwjgl::Display::processMessages) never have to remember to scan.
	hidScanInput();
	const u32 heldKeysRaw = hidKeysHeld();
	// Never name locals keysDown/keysUp: hid.h's compatibility macros
	// (#define keysDown hidKeysDown) would rewrite the tokens.
	const u32 keysPressed = hidKeysDown();
	const u32 keysReleased = hidKeysUp();

	// Which channel the gameplay buttons route to this frame. Read by
	// updateGameplay() below rather than threaded through every helper.
	g_inMenu = inMenu;

	// A text field owns the session's input while it has focus (set by
	// VirtualKeyboard::notifyFocus): it decides what the touch panel and
	// START mean this frame.
	const bool typing = platformTextInputExclusive();

	// Circle pad -> raw -1..1 stick axes (deadzones are downstream, per
	// DsInput.h). Read before the text mask below: while a screen is open
	// the stick also rides the menu channels (menuStickNavBits), and both
	// channels must see the same poll's deflection. libctru's dy grows
	// UPWARD (push up = positive, the opposite of the raw-joystick
	// contract), so negate it into the down-positive Y the backends and
	// MovementInputFromOptions expect -- "stick up reads negative"
	// (InputBackend_3DS). Clamp because the s16 can exceed the nominal
	// saturation.
	circlePosition circle = {};
	hidCircleRead(&circle);
	g_state.stickX = std::clamp(static_cast<float>(circle.dx) / kCirclePadMax, -1.0f, 1.0f);
	g_state.stickY = std::clamp(static_cast<float>(-circle.dy) / kCirclePadMax, -1.0f, 1.0f);

	// New 3DS C-Stick -> raw -1..1 with the same axis conventions as the
	// circle pad above (Y negated into down-positive, so "nub up reads
	// negative" the way every downstream consumer expects). hidScanInput()
	// already refreshed libctru's cache through irrstScanInput() -- hidInit()
	// starts ir:rst on New hardware only -- and on Old hardware the call
	// returns that zeroed cache, so the axes read as a centred stick.
	circlePosition cstick = {};
	hidCstickRead(&cstick);
	g_state.cstickX = std::clamp(static_cast<float>(cstick.dx) / kCirclePadMax, -1.0f, 1.0f);
	g_state.cstickY = std::clamp(static_cast<float>(-cstick.dy) / kCirclePadMax, -1.0f, 1.0f);

	// Buttons -> PLATFORM_TEXT_* mask; rising edges accumulate until
	// dsInputConsumePressed() takes them. The circle pad's menu direction
	// joins the D-pad bits unconditionally: the latch is dropped in gameplay
	// anyway (updateGameplay clears it below) and the unconditional OR keeps
	// the edge differencing continuous across menu boundaries, so a stick
	// held while a screen opens does not fire a step it was never pushed
	// for.
	const std::uint32_t held =
	    mapTextButtons(heldKeysRaw) | menuStickTextBits(menuStickNavBits());
	g_latchedPressed |= held & ~g_prevHeld;
	g_prevHeld = held;
	g_state.held = held;

	// Touch -> absolute pointer in top-screen pixels, forwarded into the
	// mouse queue so the GUI is clickable from the bottom screen. Only the
	// MOTION lives here: the button 0 edge it implies is emitted by
	// updateGameplay(), which knows whether X is allowed to add a second
	// source to it.
	//
	// While typing none of that happens: the coordinates stay in the panel's
	// own 320x240 space for the on-screen keyboard's hit-testing (which
	// converts them against InputBackend_3DS's pointerWidth), and nothing
	// enters the mouse queue -- a click at panel coordinates would land
	// under the top screen's cursor instead of under the finger.
	const bool touchDown = (heldKeysRaw & KEY_TOUCH) != 0;
	bool widgetContact = false;
	if (touchDown)
	{
		touchPosition touch = {};
		hidTouchRead(&touch);

		// Dual-screen gameplay HUD: the panel's touch widgets own their
		// contact -- the camera pad must not look while a finger is on one,
		// and a fresh contact fires the widget's action once instead. A
		// drag that merely slides onto a widget does nothing; everything
		// else on the panel stays the camera pad exactly as before.
		if (!typing && !g_inMenu)
		{
			const touchHud::WidgetHit hit =
			    touchHud::hitTest(static_cast<int>(touch.px), static_cast<int>(touch.py));
			if (hit.widget != touchHud::Widget::None)
			{
				widgetContact = true;
				if (!g_prevTouchDown || g_prevTextExclusive)
					pressTouchHudWidget(hit);
			}
		}

		if (!widgetContact)
		{
			// Sliding off a widget (or never touching one): any latched
			// widget key comes up so the camera pad can take over.
			releaseTouchHudWidget();

			// Pocket-Edition-style pad gestures (gameplay, non-widget
			// contacts only): a stationary hold starts breaking (button 0
			// held until lift; the aim can keep adjusting while it runs),
			// a drag before that threshold is the camera alone, and a
			// short stationary contact stays armed as a tap whose verdict
			// the lift decides (the game side turns it into place or
			// swing from the crosshair target). Skipped entirely when the
			// "Touch Click" toggle is off: the pad stays a camera drag.
			if (!g_inMenu && !typing && g_pocketTouch)
			{
				if (!g_prevTouchDown || g_prevTextExclusive)
				{
					g_padContactStartMs = osGetTime();
					g_padStartX = static_cast<int>(touch.px);
					g_padStartY = static_cast<int>(touch.py);
					g_padTapArmed = true;
					g_padBreakActive = false;
				}
				const int moveX = static_cast<int>(touch.px) - g_padStartX;
				const int moveY = static_cast<int>(touch.py) - g_padStartY;
				if (g_padTapArmed &&
				    (moveX > 12 || -moveX > 12 || moveY > 12 || -moveY > 12))
				{
					g_padTapArmed = false;
					g_padBreakActive = false;
				}
				if (g_padTapArmed && !g_padBreakActive &&
				    osGetTime() - g_padContactStartMs >= 180)
				{
					g_padBreakActive = true;
				}
			}

			// X: 320-wide panel -> top-screen width. Y: both screens are 240
			// tall, so it needs no scaling; clamp against the stored height
			// anyway so the pointer can never land outside the GUI regardless
			// of what geometry dsInputInit was handed.
			const int x = typing
			    ? static_cast<int>(touch.px)
			    : static_cast<int>(touch.px) * g_screenW / kTouchPanelW;
			const int y = typing
			    ? static_cast<int>(touch.py)
			    : std::min(static_cast<int>(touch.py), g_screenH - 1);

			g_state.pointerActive = true;
			g_state.pointerX = x;
			g_state.pointerY = y;

			// Publish the position before any click derived from it, and on new
			// contact publish it first so the click lands where the finger is.
			// A finger surviving the end of a typing session counts as a new
			// contact too: the previous sample was in the other coordinate
			// space, and differencing across that would fling the cursor once.
			// Leaving a widget counts the same way: its samples never reached
			// the pointer, so the publish must restart from zero deltas.
			if (!typing)
			{
				if (!g_prevTouchDown || g_prevTextExclusive || g_prevTouchOnWidget)
					lwjgl::Mouse::detail::pushMotion(x, y, 0, 0);
				else if (x != g_prevTouchX || y != g_prevTouchY)
					lwjgl::Mouse::detail::pushMotion(x, y, x - g_prevTouchX, y - g_prevTouchY);
			}
		}
		// A widget contact leaves pointerActive/X/Y at their last sample:
		// no motion reaches the camera while the finger is on the widget.
	}
	else
	{
		// Release at the last contact point, not at (0,0): pointerX/Y keep
		// the final sample, which is where updateGameplay() emits the up.
		g_state.pointerActive = false;
		// The finger lifting off a widget ends its action: the latched key
		// comes up here -- a hold on the hotbar selects once, not forever.
		releaseTouchHudWidget();
		// The pad gesture's verdict on lift: a short stationary contact
		// taps (the game side decides place vs swing); anything longer or
		// dragged already acted, or was the camera all along. An off
		// "Touch Click" never arms g_padTapArmed, so this stays quiet.
		if (!g_inMenu && !typing && g_pocketTouch && g_padTapArmed &&
		    osGetTime() - g_padContactStartMs < 180)
			g_touchPadTapRequested = true;
		g_padTapArmed = false;
		g_padBreakActive = false;
	}
	const bool newContact = !g_prevTouchDown || g_prevTextExclusive;
	g_prevTouchDown = touchDown;
	g_prevTouchOnWidget = widgetContact;
	// A fresh contact remembers whether it began on a touch-HUD widget:
	// that contact is what opens the widget's screen, and its held finger
	// must not double as the new menu's first click -- the crafting screen
	// entered with a phantom grab at the button position that way, and
	// every item move after it fought the invisible grabbed stack.
	if (newContact)
		g_contactStartedOnWidget = widgetContact;
	g_prevTouchX = g_state.pointerX;
	g_prevTouchY = g_state.pointerY;

	// Face-button camera: in gameplay with the toggle on, the diamond and
	// SELECT leave the ordinary gameplay channel entirely (their roles move
	// to updateFaceButtonCamera below), so attack/use live on the shoulders
	// alone and nothing double-fires. In menus the buttons keep their usual
	// meanings -- the mode is a camera, not a full control scheme swap.
	u32 keys = heldKeysRaw;
	if (g_faceButtonCamera && !g_inMenu && !typing)
	{
		constexpr std::uint32_t kFaceMask = KEY_A | KEY_B | KEY_X | KEY_Y | KEY_SELECT;
		const std::uint32_t faceHeld = keys & kFaceMask;
		keys &= ~kFaceMask;
		updateFaceButtonCamera(faceHeld, keysPressed & kFaceMask, keysReleased & kFaceMask);
	}
	else if (g_selectDownSinceMs != 0 || g_selectSneakLatched || g_faceJumpHoldFrames > 0)
	{
		// The face context ended (menu opened, field focused) while a SELECT
		// tap/hold was mid-flight: a latched sneak must be released or the
		// player stays crouched behind the menu, a pending tap must not
		// jump on the way back, and a jump press still on its hold must
		// come up too or the player hops once when the menu closes.
		if (g_selectSneakLatched)
			lwjgl::Keyboard::detail::pushKey(DS_KEY_SELECT, false);
		if (g_faceJumpHoldFrames > 0)
			lwjgl::Keyboard::detail::pushKey(DS_KEY_A, false);
		g_selectDownSinceMs = 0;
		g_selectSneakLatched = false;
		g_faceJumpHoldFrames = 0;
	}

	// New 3DS C-Stick look, gameplay only: menus and text entry keep the nub
	// inert so it never fights the D-pad/keyboard, the same way the
	// face-button camera stands down outside gameplay.
	if (!g_inMenu && !typing)
		updateCStickCamera();

	// After the touch block, so clicks carry this frame's coordinates. The
	// finger is a click only where there is something to click: in menus it
	// IS the pointer, but in gameplay the triggers own both buttons and
	// touch is the camera alone -- dragging the panel must not swing the
	// pickaxe. Text entry keeps its own exclusion either way. And the
	// contact that OPENED a menu through a touch-HUD widget never clicks
	// inside it: the held finger lands where the button was, not where the
	// user meant to act.
	updateGameplay(keys, g_inMenu && touchDown && !typing && !g_contactStartedOnWidget);

	// Container navigation still runs its transition detector: an escape
	// key that was already down when a container opened (pressed in the menu
	// the container came from) is released on the handoff so it cannot stay
	// stuck. B and START themselves keep closing the container -- the
	// navigator's split-half click rides X instead while one is open.
	const bool containerNav = platformContainerNavigationActive();
	if (containerNav && !g_prevContainerNav)
		lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_ESCAPE, false);
	g_prevContainerNav = containerNav;

	// START keeps its fixed KEY_ESCAPE role only while nothing has focus:
	// pause in-world, "go back" in a screen -- containers included, whose
	// slot cursor never conflicts with the pause/back key -- edge-driven off
	// keysPressed rather than the held mask so one held across the boundary
	// does not re-fire. With a field focused, mapTextButtons turned START
	// into ENTER instead (what submits chat), and forwarding its edges as
	// ESC here would close the very screen being typed in. An escape key
	// whose press went out before focus arrived is released on the
	// transition, otherwise it would stay stuck down for the rest of the
	// session.
	if (typing && !g_prevTextExclusive)
		lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_ESCAPE, false);
	g_prevTextExclusive = typing;
	if (!typing)
		forwardStartToEscape(keysPressed, keysReleased);
}

const DsInputState& dsInputState()
{
	return g_state;
}

std::uint32_t dsInputConsumePressed()
{
	const std::uint32_t pressed = g_latchedPressed;
	g_latchedPressed = 0;
	return pressed;
}

const char* dsInputDebugLine()
{
	// Idle = the input system hasn't been set up yet; DsInput.h lets this
	// report nullptr until dsInputInit() has run.
	if (!g_initialized)
		return nullptr;
	std::snprintf(g_debugLine, sizeof(g_debugLine),
	              "held=%03X t%c %d,%d cp%+.2f,%+.2f n%+.2f,%+.2f",
	              static_cast<unsigned>(g_state.held),
	              g_state.pointerActive ? '+' : '-',
	              g_state.pointerX, g_state.pointerY,
	              static_cast<double>(g_state.stickX),
	              static_cast<double>(g_state.stickY),
	              static_cast<double>(g_state.cstickX),
	              static_cast<double>(g_state.cstickY));
	return g_debugLine;
}

#endif // CTR_PLATFORM
