#pragma once

#ifdef PS2_PLATFORM

void ps2_viewport_set(int x, int y, int width, int height);
void ps2_viewport_get(int* values);
float ps2_viewport_width();
float ps2_viewport_height();

#endif // PS2_PLATFORM
