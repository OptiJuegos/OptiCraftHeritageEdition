#pragma once

// Public include shim.
// Common game code uses #include "lwjgl/Keyboard.h" while the real shared
// declaration lives under src/pc/lwjgl. Keeping this forwarding header under
// src/ makes the include work even if a stale PS2 CMake cache has not picked up
// -I src/pc yet.
#include "../pc/lwjgl/Keyboard.h"
