#include "platform/Input.h"

#include "wii/input/WiiPadState.h"

PlatformTextInputSnapshot platformTextInputSnapshot(int port)
{
    (void)port;
    PlatformTextInputSnapshot out;
    const WiiTextInputSnapshot pad = wiiTextInputSnapshot();
    out.connected = true;
    out.held = pad.held;
    out.pressed = pad.pressed;
    out.pointerValid = pad.irValid;
    out.pointerX = pad.irX;
    out.pointerY = pad.irY;
    out.pointerWidth = pad.irWidth;
    out.pointerHeight = pad.irHeight;
    return out;
}

PlatformGamepadSnapshot platformGamepadSnapshot(int port)
{
    (void)port;
    PlatformGamepadSnapshot out;
    const WiiStickSnapshot stick = wiiStickSnapshot();
    out.connected = stick.connected;
    out.leftX = stick.x;
    out.leftY = stick.y;
    return out;
}

PlatformGamepadSnapshot platformRawGamepadSnapshot(int port)
{
    return platformGamepadSnapshot(port);
}

int platformMenuPad()
{
    return 0;
}

void platformSetMenuCursor(int x, int y)
{
    wiiSetCursorPosition(x, y);
}

const PlatformKeyboardHints& platformKeyboardHints()
{
    static const PlatformKeyboardHints hints = {
        { "A:type B:del D-pad:move",
          "Wii 2:cap -=sp +=ok 1:close",
          "GC Z:cap X:sp St:ok Y:close" }, 3
    };
    return hints;
}

const char* platformInputDebugLine()
{
    extern const char* wiiPadDebugLine();
    return wiiPadDebugLine();
}
