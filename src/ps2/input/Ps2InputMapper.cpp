#ifdef PS2_PLATFORM

#include "platform/Log.h"
#include "ps2/input/Ps2InputMapper.h"
#include "ps2/input/Ps2PadState.h"
#include "ps2/input/Ps2AnalogFilter.h"
#include "ps2/input/Ps2Pointer.h"
#include "ps2/render/Ps2Tuning.h"
#include "platform/Input.h"
#include "ps2/input/Ps2PadKeyCodes.h"
#include "lwjgl/Keyboard.h"
#include "lwjgl/Mouse.h"
#include <libpad.h>
#include <stdio.h>

namespace {
static const float MENU_DEADZONE = 0.70f;
static const float MENU_SCROLL_THRESHOLD = 0.18f;
static const float CAM_SCALE = 14.0f;
static const int CAMERA_WARMUP_FRAMES = 18;
static bool s_gameplayKeyDown[256] = {};
static bool s_previousMenu = false;
static int s_cameraWarmup = 0;

float deadzone(float v, float dz) { return (v > -dz && v < dz) ? 0.0f : v; }
float deadzoneScaled(float v, float dz) {
    if (v > -dz && v < dz) return 0.0f;
    const float sign = v < 0.0f ? -1.0f : 1.0f;
    float out = ((v * sign) - dz) / (1.0f - dz);
    if (out < 0.0f) out = 0.0f;
    if (out > 1.0f) out = 1.0f;
    return out * sign;
}
void setKey(int key, bool down) {
    if (key < 0 || key >= 256 || s_gameplayKeyDown[key] == down) return;
    s_gameplayKeyDown[key] = down;
    lwjgl::Keyboard::detail::pushKey(key, down);
}
void releaseGameplayKeys() {
    setKey(PS2_KEY_DPAD_UP, false);   setKey(PS2_KEY_DPAD_DOWN, false);
    setKey(PS2_KEY_DPAD_LEFT, false); setKey(PS2_KEY_DPAD_RIGHT, false);
    setKey(PS2_KEY_CROSS, false);     setKey(PS2_KEY_CIRCLE, false);
    setKey(PS2_KEY_TRIANGLE, false);  setKey(PS2_KEY_SQUARE, false);
    setKey(PS2_KEY_L1, false);        setKey(PS2_KEY_R1, false);
    setKey(PS2_KEY_L2, false);        setKey(PS2_KEY_R2, false);
    setKey(PS2_KEY_L3, false);        setKey(PS2_KEY_R3, false);
}

const Ps2PadSnapshot& menuPad(const Ps2PadSnapshot& primary) {
    int owner = ps2GetMenuOwnerPad();
    if (owner < 0) owner = ps2GetMenuPad();
    if (owner == 1) {
        const Ps2PadSnapshot& second = ps2PadGetSnapshot(1);
        if (second.connected) return second;
    }
    return primary;
}

// Buttons a Controls-menu binding can learn. Start/Select are deliberately
// left out -- see Ps2PadKeyCodes.h.
const struct { unsigned mask; int code; } kRemappableButtons[] = {
    { PAD_CROSS, PS2_KEY_CROSS },       { PAD_CIRCLE, PS2_KEY_CIRCLE },
    { PAD_TRIANGLE, PS2_KEY_TRIANGLE }, { PAD_SQUARE, PS2_KEY_SQUARE },
    { PAD_L1, PS2_KEY_L1 },             { PAD_R1, PS2_KEY_R1 },
    { PAD_L2, PS2_KEY_L2 },             { PAD_R2, PS2_KEY_R2 },
    { PAD_L3, PS2_KEY_L3 },             { PAD_R3, PS2_KEY_R3 },
    { PAD_UP, PS2_KEY_DPAD_UP },        { PAD_DOWN, PS2_KEY_DPAD_DOWN },
    { PAD_LEFT, PS2_KEY_DPAD_LEFT },    { PAD_RIGHT, PS2_KEY_DPAD_RIGHT },
};

void updateMenu(const Ps2PadSnapshot& primary) {
    if (platformTextInputExclusive()) return;
    const Ps2PadSnapshot& p = menuPad(primary);

    if (platformPadRebindExclusive()) {
        // GuiControls is listening for a new binding. Suspend the normal
        // confirm/cancel/scroll synthesis below and report exactly one
        // pressed button as a synthetic key-down event -- GuiControls'
        // existing keyTyped() capture stores it unchanged, exactly like it
        // would a real keyboard key.
        for (const auto &entry : kRemappableButtons) {
            if (p.pressed & entry.mask) {
                lwjgl::Keyboard::detail::pushKey(entry.code, true);
                lwjgl::Keyboard::detail::pushKey(entry.code, false);
                break;
            }
        }
        return;
    }
    const float dt = Ps2Pointer::beginMenuFrame();
    const float dpadSpeed = 300.0f, analogSpeed = 420.0f;
    const bool r3 = (p.held & PAD_R3) != 0;
    const bool slotNav = platformContainerNavigationActive();
    float dx = 0.0f, dy = 0.0f;
    if (!r3 && !slotNav && (p.held & PAD_UP)) dy -= dpadSpeed * dt;
    if (!r3 && !slotNav && (p.held & PAD_DOWN)) dy += dpadSpeed * dt;
    if (!slotNav && (p.held & PAD_LEFT)) dx -= dpadSpeed * dt;
    if (!slotNav && (p.held & PAD_RIGHT)) dx += dpadSpeed * dt;
    const float menuX = Ps2AnalogFilter::apply(p.leftX);
    const float menuY = Ps2AnalogFilter::apply(p.leftY);
    dx += deadzone(menuX, MENU_DEADZONE) * analogSpeed * dt;
    dy += deadzone(menuY, MENU_DEADZONE) * analogSpeed * dt;
    Ps2Pointer::move(dx, dy);
    Ps2Pointer::publish();
    const int cx = Ps2Pointer::x(), cy = Ps2Pointer::y();

    static float scrollRepeat = 0.0f;
    if (r3 && (p.held & (PAD_UP | PAD_DOWN))) {
        bool fire = (p.pressed & (PAD_UP | PAD_DOWN)) != 0;
        scrollRepeat -= dt;
        if (scrollRepeat <= 0.0f) { fire = true; scrollRepeat = 0.12f; }
        if (fire) lwjgl::Mouse::detail::pushWheel((p.held & PAD_UP) ? 1 : -1, cx, cy);
    } else scrollRepeat = 0.0f;

    static float stickScroll = 0.0f;
    const float rsv = deadzone(Ps2AnalogFilter::apply(p.rightY), MENU_SCROLL_THRESHOLD);
    if (rsv != 0.0f) {
        const float mag = rsv < 0.0f ? -rsv : rsv;
        const float interval = 0.20f - 0.16f * mag;
        stickScroll -= dt;
        if (stickScroll <= 0.0f) { stickScroll = interval; lwjgl::Mouse::detail::pushWheel(rsv < 0.0f ? 1 : -1, cx, cy); }
    } else stickScroll = 0.0f;

    if (p.pressed & PAD_CROSS) lwjgl::Mouse::detail::pushButton(0, true, cx, cy);
    if (p.released & PAD_CROSS) lwjgl::Mouse::detail::pushButton(0, false, cx, cy);
    if (p.pressed & PAD_SQUARE) lwjgl::Mouse::detail::pushButton(1, true, cx, cy);
    if (p.released & PAD_SQUARE) lwjgl::Mouse::detail::pushButton(1, false, cx, cy);
    if (p.pressed & (PAD_CIRCLE | PAD_TRIANGLE)) lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_ESCAPE, true);
    if (p.released & (PAD_CIRCLE | PAD_TRIANGLE)) lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_ESCAPE, false);
    if (p.pressed & PAD_START) lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_RETURN, true);
    if (p.released & PAD_START) lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_RETURN, false);
    if (p.pressed & PAD_SELECT) lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_TAB, true);
    if (p.released & PAD_SELECT) lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_TAB, false);
}

void updateGameplay(const Ps2PadSnapshot& p) {
    if (s_cameraWarmup > 0) { --s_cameraWarmup; lwjgl::Mouse::clearDeltas(); }
#if !PS2_DIRECT_PAD_CAMERA
    else {
        float camX = Ps2AnalogFilter::apply(p.rightX);
        float camY = Ps2AnalogFilter::apply(p.rightY);
        int dx = (int)(camX * CAM_SCALE), dy = (int)(camY * CAM_SCALE);
        if (dx > -2 && dx < 2) dx = 0;
        if (dy > -2 && dy < 2) dy = 0;
        if (dx || dy) lwjgl::Mouse::detail::pushMotion(0, 0, dx, dy);
    }
#endif
    // Every remappable button gets its own synthetic key state unconditionally.
    // Which of these codes actually drives an action (Jump, Sneak, Forward...)
    // is entirely up to what GameSettings has bound it to -- Ps2InputMapper no
    // longer hardcodes "Cross means jump" anywhere.
#if PS2_DIRECT_PAD_MOVEMENT
    setKey(PS2_KEY_DPAD_UP, (p.held & PAD_UP) != 0);
    setKey(PS2_KEY_DPAD_DOWN, (p.held & PAD_DOWN) != 0);
    setKey(PS2_KEY_DPAD_LEFT, (p.held & PAD_LEFT) != 0);
    setKey(PS2_KEY_DPAD_RIGHT, (p.held & PAD_RIGHT) != 0);
#else
    const float moveX = Ps2AnalogFilter::apply(p.leftX);
    const float moveY = Ps2AnalogFilter::apply(p.leftY);
    setKey(PS2_KEY_DPAD_UP, moveY < -0.05f || (p.held & PAD_UP));
    setKey(PS2_KEY_DPAD_DOWN, moveY >  0.05f || (p.held & PAD_DOWN));
    setKey(PS2_KEY_DPAD_LEFT, moveX < -0.05f || (p.held & PAD_LEFT));
    setKey(PS2_KEY_DPAD_RIGHT, moveX >  0.05f || (p.held & PAD_RIGHT));
#endif
    setKey(PS2_KEY_CROSS, (p.held & PAD_CROSS) != 0);
    setKey(PS2_KEY_CIRCLE, (p.held & PAD_CIRCLE) != 0);
    setKey(PS2_KEY_TRIANGLE, (p.held & PAD_TRIANGLE) != 0);
    setKey(PS2_KEY_SQUARE, (p.held & PAD_SQUARE) != 0);
    setKey(PS2_KEY_L1, (p.held & PAD_L1) != 0);
    setKey(PS2_KEY_R1, (p.held & PAD_R1) != 0);
    setKey(PS2_KEY_L2, (p.held & PAD_L2) != 0);
    setKey(PS2_KEY_R2, (p.held & PAD_R2) != 0);
    setKey(PS2_KEY_L3, (p.held & PAD_L3) != 0);
    setKey(PS2_KEY_R3, (p.held & PAD_R3) != 0);

    if (p.pressed & PAD_R2) lwjgl::Mouse::detail::pushButton(0, true, 0, 0);
    if (p.released & PAD_R2) lwjgl::Mouse::detail::pushButton(0, false, 0, 0);
    if (p.pressed & PAD_L2) lwjgl::Mouse::detail::pushButton(1, true, 0, 0);
    if (p.released & PAD_L2) lwjgl::Mouse::detail::pushButton(1, false, 0, 0);
    if (p.pressed & PAD_START) { ps2SetMenuPad(0); ps2SetMenuOwnerPad(0); lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_ESCAPE, true); }
    if (p.released & PAD_START) lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_ESCAPE, false);
    if (p.pressed & PAD_SQUARE) ps2SetMenuPad(0);
    if (p.pressed & PAD_R1) lwjgl::Mouse::detail::pushWheel(-1, 0, 0);
    if (p.pressed & PAD_L1) lwjgl::Mouse::detail::pushWheel(1, 0, 0);
    if (p.pressed & PAD_R3) lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_F5, true);
    if (p.released & PAD_R3) lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_F5, false);
    if (p.pressed & PAD_SELECT) lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_F3, true);
    if (p.released & PAD_SELECT) lwjgl::Keyboard::detail::pushKey(lwjgl::Keyboard::KEY_F3, false);
}
}

namespace Ps2InputMapper {
void update(bool inMenu) {
    const Ps2PadSnapshot& primary = ps2PadGetSnapshot(0);
    if (inMenu && !s_previousMenu) { releaseGameplayKeys(); Ps2Pointer::enterMenu(); }
    if (!inMenu && s_previousMenu) { s_cameraWarmup = CAMERA_WARMUP_FRAMES; lwjgl::Mouse::clearDeltas(); Ps2Pointer::leaveMenu(); MC_LOG_INFO("input", "[PS2] camera warmup: dropping stale mouse deltas\n"); }
    s_previousMenu = inMenu;
    if (!primary.connected) { if (!inMenu) releaseGameplayKeys(); return; }
    if (inMenu) updateMenu(primary); else updateGameplay(primary);
}
}

#endif
