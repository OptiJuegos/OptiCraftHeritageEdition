#pragma once

#include <cstdint>

#include "platform/Input.h" // PLATFORM_TEXT_* action masks

// Touchscreen + circle-pad input for the 3DS port (src/3ds/input/DsInput.cpp).
//
// One snapshot per frame -- what platform/InputBackend_3DS.cpp feeds into the
// shared Input API -- plus the key/mouse/wheel edges the engine plays with.
// Every physical button has up to two channels, because an open screen and a
// closed one give it different meanings; DsInput.cpp's header carries the
// full button -> channel table.
//
// Coordinates: the game surface is the top screen (400x240), the touch panel
// the bottom screen (320x240). Touch positions are mapped to top-screen
// pixels inside DsInput (x * 400/320, y unchanged), so callers can compare
// them against the 400x240 GUI directly. The exception is text entry: while
// a field has focus the coordinates stay in the panel's own 320x240 space
// and no mouse events are forwarded at all, because the on-screen keyboard
// (or the system keyboard it may open instead) reads the panel itself -- see
// platformTextInputExclusive() in the .cpp.

struct DsInputState
{
    bool connected = true;   // the console itself is always "attached"
    std::uint32_t held = 0;  // PLATFORM_TEXT_* actions held right now

    bool pointerActive = false; // finger down during this poll
    int pointerX = 0;           // top-screen pixels (raw panel pixels while a
    int pointerY = 0;           // field has focus); valid while pointerActive

    bool stickConnected = true; // circle pad present (always true on 3DS)
    float stickX = 0.0f;        // -1..1, raw (deadzone handling is downstream)
    float stickY = 0.0f;

    // New 3DS C-Stick, same raw -1..1 axis contract as the circle pad fields
    // (Y down-positive, nub up reads negative). On an Old 3DS ir:rst never
    // initialises (hidInit only starts it on New hardware), so
    // irrstCstickRead reports a zeroed position and these stay 0 -- the
    // gameplay look channel is inert rather than absent, and the shared
    // snapshot's right-stick fields read a centred stick.
    float cstickX = 0.0f;
    float cstickY = 0.0f;
};

// One-time setup: remember the top-screen size used for the touch mapping.
// Called from lwjgl::Display::create().
void dsInputInit(int screenW, int screenH);

// hidScanInput() + refresh the snapshot; called once per frame from
// lwjgl::Display::processMessages(). Also forwards touch -> mouse, START ->
// KEY_ESCAPE (or ENTER while a field has focus), and the gameplay channel
// (jump/inventory/sneak keys; attack from X or R and use from B or L as
// mouse buttons; hotbar wheel from D-pad LEFT/RIGHT, ZL/ZR on a New 3DS,
// and the C-Stick as a look pad there; chat from D-pad UP). While a field
// has focus the menu navigation and the mouse forwarding stand down -- see
// DsInput.cpp.
//
// inMenu is "a GuiScreen is currently open", which the input layer cannot
// work out for itself -- the same reason WiiPadState::wiiPadPoll() and
// Ps2Input::update() take it as a parameter (see the header comment in
// DsInput.cpp for what it gates).
void dsInputPoll(bool inMenu);

// Face-button camera toggle (OptiCraft Options): while on, gameplay gives
// the A/B/X/Y diamond to the camera (Y/A/X/B = look left/right/up/down),
// jump moves to SELECT-tap or double-tap-B, sneak to holding SELECT, and
// inventory to double-tap-Y; attack/use stay on the shoulders. Called from
// GameSettings whenever the option is (re)loaded or changed.
void dsInputSetFaceButtonCamera(bool enabled);

// Pocket-Edition touch gestures toggle (OptiCraft Options, "Touch Click"):
// while on, a short stationary touch on the camera pad taps (place/swing,
// routed by the crosshair target) and a hold of 180 ms or more breaks/uses.
// While off the pad is the plain camera drag it was before those gestures
// existed -- the widgets (hotbar, inventory/crafting/pause) and the menus
// are unaffected either way. Called from GameSettings whenever the option
// is (re)loaded or changed.
void dsInputSetPocketTouch(bool enabled);

const DsInputState& dsInputState();

// PLATFORM_TEXT_* actions that went down since the last call: returns the
// accumulated edge mask and clears it (consume-on-read, same contract as the
// Wii's wiiTextInputConsumePressed()).
std::uint32_t dsInputConsumePressed();

// One-line human-readable state for the debug overlay, or nullptr when idle.
const char* dsInputDebugLine();
