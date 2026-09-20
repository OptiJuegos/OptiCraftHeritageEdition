#include "platform/Input.h"

#include "ps2/input/Ps2AnalogFilter.h"
#include "ps2/input/Ps2Input.h"
#include "ps2/input/Ps2PadState.h"

namespace
{
std::uint32_t mapTextButtons(unsigned short bits)
{
    std::uint32_t value = 0;
    if (bits & PS2_PAD_LEFT) value |= PLATFORM_TEXT_LEFT;
    if (bits & PS2_PAD_RIGHT) value |= PLATFORM_TEXT_RIGHT;
    if (bits & PS2_PAD_UP) value |= PLATFORM_TEXT_UP;
    if (bits & PS2_PAD_DOWN) value |= PLATFORM_TEXT_DOWN;
    if (bits & PS2_PAD_CROSS) value |= PLATFORM_TEXT_TYPE;
    if (bits & PS2_PAD_SQUARE) value |= PLATFORM_TEXT_BACK;
    if (bits & PS2_PAD_SELECT) value |= PLATFORM_TEXT_SPACE;
    if (bits & PS2_PAD_TRIANGLE) value |= PLATFORM_TEXT_SHIFT;
    if (bits & PS2_PAD_START) value |= PLATFORM_TEXT_ENTER;
    if (bits & PS2_PAD_CIRCLE) value |= PLATFORM_TEXT_CLOSE;
    return value;
}
}

PlatformTextInputSnapshot platformTextInputSnapshot(int port)
{
    PlatformTextInputSnapshot out;
    const Ps2PadSnapshot& pad = ps2PadGetSnapshot(port);
    out.connected = pad.connected;
    out.held = mapTextButtons(pad.held);
    out.pressed = mapTextButtons(ps2PadConsumePressed(port));
    return out;
}

PlatformGamepadSnapshot platformGamepadSnapshot(int port)
{
    PlatformGamepadSnapshot out;
    const Ps2PadSnapshot& pad = ps2PadGetSnapshot(port);
    out.connected = pad.connected;
    out.leftX = Ps2AnalogFilter::apply(pad.leftX);
    out.leftY = Ps2AnalogFilter::apply(pad.leftY);
    out.rightX = Ps2AnalogFilter::apply(pad.rightX);
    out.rightY = Ps2AnalogFilter::apply(pad.rightY);
    return out;
}

PlatformGamepadSnapshot platformRawGamepadSnapshot(int port)
{
    PlatformGamepadSnapshot out;
    const Ps2PadSnapshot& pad = ps2PadGetSnapshot(port);
    out.connected = pad.connected;
    out.leftX = pad.leftX;
    out.leftY = pad.leftY;
    out.rightX = pad.rightX;
    out.rightY = pad.rightY;
    return out;
}

int platformMenuPad()
{
    const int owner = ps2GetMenuOwnerPad();
    return owner >= 0 ? owner : ps2GetMenuPad();
}

bool platformMenuPointerActive()
{
    return true;
}

bool platformMenuCursorVisible()
{
    return true;
}

void platformSetMenuCursor(int x, int y)
{
    Ps2Input::setMenuCursor(x, y);
}

const PlatformKeyboardHints& platformKeyboardHints()
{
    static const PlatformKeyboardHints hints = {
        {
            "X:type  Sq:del  Tri:shift  Sel:space  Start:ok  O:close",
            "Right stick: move keyboard", nullptr
        }, 2
    };
    return hints;
}

const char* platformInputDebugLine()
{
    return "";
}
