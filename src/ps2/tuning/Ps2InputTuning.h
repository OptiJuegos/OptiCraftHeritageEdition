#pragma once

#define PS2_DIRECT_PAD_CAMERA 1
#define PS2_SKIP_CAMERA_FX 1
// Legacy fallback constants retained for platform code that still consumes the
// tuning aliases. Normal PS2 gameplay now uses the user-configurable deadzone
// from Ps2AnalogFilter after boot-time stick-center calibration.
#define PS2_DIRECT_CAMERA_DEADZONE 0.18f
#define PS2_DIRECT_CAMERA_SCALE 96.0f
// Some PS2/PCSX2 mappings report right-stick vertical opposite to the
// Minecraft mouse convention. Keep X normal, invert Y for console look.
#define PS2_DIRECT_CAMERA_INVERT_X 0
#define PS2_DIRECT_CAMERA_INVERT_Y 1

// Feed the left stick directly into MovementInputFromOptions each tick.  The
// D-Pad still synthesizes W/A/S/D, but analog movement should not depend on
// queued keyboard events.
#define PS2_DIRECT_PAD_MOVEMENT 1
#define PS2_DIRECT_MOVE_DEADZONE 0.18f // legacy fallback; Ps2AnalogFilter is canonical
#define PS2_DIRECT_MOVE_SCALE 1.0f

// EntityRenderer runs once per rendered frame.  Scale stick look by elapsed
// time so a slow frame does not change the camera behaviour (or make a stuck
// axis rotate for an entire second at once).
#define PS2_DIRECT_CAMERA_REFERENCE_FPS 60.0f
#define PS2_DIRECT_CAMERA_MAX_DT 0.10f
