#pragma once
#ifdef PS2_PLATFORM

// Shows the Mojang + Minecraft title splash screens before the save prompt.
// Skippable at any point with any face button or Start.
// Restores the graphics VRAM allocation pointer before returning.
void ps2_run_startup_logos();

#endif // PS2_PLATFORM
