#pragma once

#include <cstdint>

enum PlatformTextAction : std::uint32_t
{
    PLATFORM_TEXT_LEFT   = 1u << 0,
    PLATFORM_TEXT_RIGHT  = 1u << 1,
    PLATFORM_TEXT_UP     = 1u << 2,
    PLATFORM_TEXT_DOWN   = 1u << 3,
    PLATFORM_TEXT_TYPE   = 1u << 4,
    PLATFORM_TEXT_BACK   = 1u << 5,
    PLATFORM_TEXT_SPACE  = 1u << 6,
    PLATFORM_TEXT_SHIFT  = 1u << 7,
    PLATFORM_TEXT_ENTER      = 1u << 8,
    PLATFORM_TEXT_CLOSE      = 1u << 9,
    PLATFORM_TEXT_PREV_PAGE  = 1u << 10,
    PLATFORM_TEXT_NEXT_PAGE  = 1u << 11,
};

struct PlatformTextInputSnapshot
{
    bool connected = false;
    std::uint32_t held = 0;
    std::uint32_t pressed = 0;
    bool pointerValid = false;
    int pointerX = 0;
    int pointerY = 0;
    int pointerWidth = 0;
    int pointerHeight = 0;
};

struct PlatformGamepadSnapshot
{
    bool connected = false;
    float leftX = 0.0f;
    float leftY = 0.0f;
    float rightX = 0.0f;
    float rightY = 0.0f;
};

struct PlatformKeyboardHints
{
    const char* lines[3] = { nullptr, nullptr, nullptr };
    int lineCount = 0;
};

PlatformTextInputSnapshot platformTextInputSnapshot(int port = 0);
PlatformGamepadSnapshot platformGamepadSnapshot(int port = 0);
PlatformGamepadSnapshot platformRawGamepadSnapshot(int port = 0);
int platformMenuPad();
bool platformMenuPointerActive();
bool platformMenuCursorVisible();
void platformSetMenuCursor(int x, int y);

// 3DS dual-screen HUD: the touch crafting button's open-request
// (consume-once; the game side owns which screen it opens).
bool platformConsumeTouchCraftRequest();

// 3DS dual-screen HUD: the camera pad's Pocket-Edition tap request
// (consume-once; the game side decides place vs swing by the crosshair).
bool platformConsumeTouchPadTap();

// 3DS dual-screen HUD: whether the crosshair targets a block right now.
// GuiIngame updates this every frame; DsInput's pad-hold gesture routes
// to button 0 (break) on a block and button 1 (use item: eat, draw bow)
// on air — Pocket Edition's split.
void platformSetCrosshairTargetsBlock(bool targetsBlock);
bool platformCrosshairTargetsBlock();

// Shared console GUI routing state. Game code owns these modes; platform
// backends only use them to decide whether normal gameplay bindings should be
// emitted while a text field or container navigation owns controller buttons.
void platformSetTextInputExclusive(bool active);
bool platformTextInputExclusive();
void platformSetContainerNavigationActive(bool active);
bool platformContainerNavigationActive();

// Set while a Controls-menu binding is "listening" for a new key. Console pad
// pollers check this to suspend their normal button-to-UI synthesis (menu
// confirm/cancel/scroll) and instead report a raw button press as a synthetic
// key event, so the existing keyboard-capture flow can store it unchanged.
void platformSetPadRebindExclusive(bool active);
bool platformPadRebindExclusive();

const PlatformKeyboardHints& platformKeyboardHints();
const char* platformInputDebugLine();
