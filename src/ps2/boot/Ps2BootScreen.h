#pragma once

#ifdef PS2_PLATFORM

void ps2BootFlash(unsigned char r, unsigned char g, unsigned char b);
[[noreturn]] void ps2HaltNoData();
[[noreturn]] void ps2HaltBlack();

#endif // PS2_PLATFORM
