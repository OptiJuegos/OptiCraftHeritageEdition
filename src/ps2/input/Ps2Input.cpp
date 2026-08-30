#ifdef PS2_PLATFORM

#include "ps2/input/Ps2Input.h"
#include "ps2/input/Ps2PadRuntime.h"
#include "ps2/input/Ps2InputMapper.h"
#include "ps2/input/Ps2Pointer.h"

namespace Ps2Input {
void initialize() { Ps2PadRuntime::initialize(); }
bool waitUntilReady() { return Ps2PadRuntime::waitUntilReady(); }
void poll(bool inMenu) { Ps2PadRuntime::poll(); Ps2InputMapper::update(inMenu); }
void setMenuCursor(int x, int y) { Ps2Pointer::setPosition(x, y); }
}

#endif
