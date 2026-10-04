// InputBackend_3DS.cpp -- the 3DS half of the shared platform Input API.
//
// Deliberately thin, like InputBackend_WII.cpp: src/3ds/input/DsInput.cpp
// owns the libctru scan and produces one snapshot per frame (buttons ->
// PLATFORM_TEXT_* mask for an open screen, the gameplay channel for a closed
// one, touch -> absolute pointer in top-screen pixels, circle pad -> stick
// axes), and this file only reshapes it into the shared structs. The
// circle-pad axes ride in platformGamepadSnapshot() with the same
// deadzone+rescale the PS2 backend applies (Ps2AnalogFilter); the raw variant
// stays unfiltered for the calibration screens. The New 3DS C-Stick fills the
// right-side fields the same way (deadzone+rescale filtered, raw in the raw
// variant); on an Old 3DS it reads as a centred stick, so those fields stay
// zero there exactly as they did before the nub existed.
//
// Note on the mode flags Input.h also declares (text-input-exclusive,
// container-navigation, pad-rebind): those are NOT part of the backend half --
// src/platform/Input.cpp is a common source compiled into every target and
// already owns all three flag pairs, on every platform. Re-defining them here
// would be a duplicate-symbol link error; PS2/Wii/PC backends don't define
// them either.

#include "platform/Input.h"

#include "3ds/input/DsInput.h"

#include <cmath>

namespace
{
// The deadzone+rescale Ps2AnalogFilter::apply performs for the PS2 at exactly
// this spot in InputBackend_PS2. DsInput publishes raw -1..1 axes ("deadzones
// are downstream", DsInput.h) and MovementInputFromOptions -- compiled in on
// this platform through PLATFORM_DIRECT_ANALOG_MOVEMENT -- applies no
// deadzone of its own, so without this filter a circle pad resting a few
// counts off centre creeps the player around the world.
constexpr float kStickDeadzone = 0.20f;

float applyStickDeadzone(float value)
{
    if (value > -kStickDeadzone && value < kStickDeadzone)
        return 0.0f;
    const float sign = value < 0.0f ? -1.0f : 1.0f;
    float magnitude = (std::abs(value) - kStickDeadzone) / (1.0f - kStickDeadzone);
    if (magnitude < 0.0f)
        magnitude = 0.0f;
    if (magnitude > 1.0f)
        magnitude = 1.0f;
    return magnitude * sign;
}
}

PlatformTextInputSnapshot platformTextInputSnapshot(int port)
{
    (void)port; // one input source; there is no second pad to address.
    PlatformTextInputSnapshot out;
    const DsInputState& ds = dsInputState();
    out.connected = ds.connected;
    out.held = ds.held;
    out.pressed = dsInputConsumePressed(); // consume-on-read, same as Wii/PS2
    out.pointerValid = ds.pointerActive;
    out.pointerX = ds.pointerX;
    out.pointerY = ds.pointerY;
    // DsInput already maps touch into top-screen pixels, and this panel is
    // fixed 400x240 hardware -- so the pointerX * screenW / pointerWidth
    // rescale VirtualKeyboard applies degenerates to an identity, which is
    // exactly what we want. While a field has focus the coordinates stay in
    // the panel's own 320x240 space instead (DsInput.cpp), so reporting that
    // width keeps the rescale an identity there too -- where it also lines up
    // with the bottom-screen keyboard's 320x240 canvas, and, should that
    // panel ever fall back over the top screen, maps the finger onto the GUI
    // exactly as the scaling above would have.
    out.pointerWidth = platformTextInputExclusive() ? 320 : 400;
    out.pointerHeight = 240;
    return out;
}

PlatformGamepadSnapshot platformGamepadSnapshot(int port)
{
    (void)port;
    PlatformGamepadSnapshot out;
    const DsInputState& ds = dsInputState();
    out.connected = ds.stickConnected;
    // Deadzone applied here, mirroring InputBackend_PS2 -> Ps2AnalogFilter.
    // The Y axis is down-positive (stick up reads negative), which is the
    // raw-joystick contract MovementInputFromOptions negates into "forward" --
    // the same convention the PS2 publishes. GuiScreen's own +-0.20 menu
    // threshold still sees a fully deflected stick as +-1.0 after the
    // rescale, so nothing downstream needs to know the filter moved.
    out.leftX = applyStickDeadzone(ds.stickX);
    out.leftY = applyStickDeadzone(ds.stickY);
    // The New 3DS C-Stick rides the right-stick fields with the same filter
    // and the same axis conventions, so any future consumer of the shared
    // snapshot (EntityRenderer's direct pad camera, menu scroll, ...) finds
    // a PS2-shaped right stick. Zero on Old hardware -- a centred stick --
    // and nothing reads these fields on this platform today.
    out.rightX = applyStickDeadzone(ds.cstickX);
    out.rightY = applyStickDeadzone(ds.cstickY);
    return out;
}

PlatformGamepadSnapshot platformRawGamepadSnapshot(int port)
{
    // Deliberately unfiltered: the GuiDeadzoneSettings calibration screen and
    // any future tuning UI want the pad exactly as the hardware reports it.
    // This is the same filtered/raw split InputBackend_PS2 keeps.
    PlatformGamepadSnapshot out;
    const DsInputState& ds = dsInputState();
    out.connected = ds.stickConnected;
    out.leftX = ds.stickX;
    out.leftY = ds.stickY;
    out.rightX = ds.cstickX;
    out.rightY = ds.cstickY;
    return out;
}

int platformMenuPad()
{
    return 0; // single pad, no PS2-style owner hand-off
}

bool platformMenuPointerActive()
{
    // Finger down = the touch pointer owns the GUI this frame; finger up
    // hands control back to D-pad/stick navigation (same shape as the Wii's
    // dynamic pointer ownership). While a field has focus the finger belongs
    // to the keyboard instead: its coordinates are in the panel's space, and
    // claiming the menu with them would fling every "pointer owns it" caller
    // to a position the GUI never drew (see DsInput.cpp).
    if (platformTextInputExclusive())
        return false;
    return dsInputState().pointerActive;
}

bool platformMenuCursorVisible()
{
    // The dual-screen UI made the bottom panel a touch surface: the finger
    // IS the pointer, so the software cursor drawn under it is pure noise
    // (and is hidden by the finger itself anyway). Returning false here
    // makes menuCursorSuppressed() hide it everywhere while keeping the
    // Wii's shape -- that policy uses this exact hook. The mouse keeps
    // working for hover/click, and containers still draw their own D-pad
    // slot cursor (GuiContainer -> legacyDrawSlotCursor), which does not
    // go through this function.
    return false;
}

// The gameplay touch-HUD crafting button's open-request. DsInput only
// raises the flag (the game side decides which screen to open and under
// which conditions); consume-once.
extern bool g_touchCraftRequested;

bool platformConsumeTouchCraftRequest()
{
    if (!g_touchCraftRequested)
        return false;
    g_touchCraftRequested = false;
    return true;
}

// The gameplay pad's Pocket-Edition tap request, same consume-once shape.
extern bool g_touchPadTapRequested;

bool platformConsumeTouchPadTap()
{
    if (!g_touchPadTapRequested)
        return false;
    g_touchPadTapRequested = false;
    return true;
}

// The crosshair's current target, updated by GuiIngame each frame and read
// by DsInput's pad-hold gesture to route break vs use.
static bool s_crosshairTargetsBlock = false;

void platformSetCrosshairTargetsBlock(bool targetsBlock)
{
    s_crosshairTargetsBlock = targetsBlock;
}

bool platformCrosshairTargetsBlock()
{
    return s_crosshairTargetsBlock;
}

void platformSetMenuCursor(int x, int y)
{
    // phase 1: container-navigation cursor stepping; the absolute touch
    // pointer overwrites the cursor position on its next contact anyway.
    (void)x;
    (void)y;
}

const PlatformKeyboardHints& platformKeyboardHints()
{
    // The on-screen keyboard's legend, i.e. the MENU channel only: these are
    // the PLATFORM_TEXT_* meanings mapTextButtons() assigns. Gameplay bindings
    // live in GameSettings (platformGameSettingsInitialize) and are shown by
    // the Controls screen instead.
    static const PlatformKeyboardHints hints = {
        {
            "A:type B:back D-pad:move",
            "X/L:sp Y:close R:shift",
            "Sel:sp St:enter"
        }, 3
    };
    return hints;
}

const char* platformInputDebugLine()
{
    return dsInputDebugLine();
}
