#pragma once

// -----------------------------------------------------------------------------
// Input policy aliases
// -----------------------------------------------------------------------------
#if PLATFORM_PS2
#  define PLATFORM_ANALOG_MOVE_DEADZONE       PS2_DIRECT_MOVE_DEADZONE
#  define PLATFORM_ANALOG_MOVE_SCALE          PS2_DIRECT_MOVE_SCALE
#  define PLATFORM_DIRECT_CAMERA_ENABLED      PS2_DIRECT_PAD_CAMERA
#  define PLATFORM_DIRECT_CAMERA_DEADZONE     PS2_DIRECT_CAMERA_DEADZONE
#  define PLATFORM_DIRECT_CAMERA_SCALE        PS2_DIRECT_CAMERA_SCALE
#  define PLATFORM_DIRECT_CAMERA_INVERT_X     PS2_DIRECT_CAMERA_INVERT_X
#  define PLATFORM_DIRECT_CAMERA_INVERT_Y     PS2_DIRECT_CAMERA_INVERT_Y
#  define PLATFORM_DIRECT_CAMERA_REFERENCE_FPS PS2_DIRECT_CAMERA_REFERENCE_FPS
#  define PLATFORM_DIRECT_CAMERA_MAX_DT       PS2_DIRECT_CAMERA_MAX_DT
#else
#  define PLATFORM_ANALOG_MOVE_DEADZONE       0.20f
#  define PLATFORM_ANALOG_MOVE_SCALE          1.0f
#  define PLATFORM_DIRECT_CAMERA_ENABLED      0
#  define PLATFORM_DIRECT_CAMERA_DEADZONE     0.18f
#  define PLATFORM_DIRECT_CAMERA_SCALE        96.0f
#  define PLATFORM_DIRECT_CAMERA_INVERT_X     0
#  define PLATFORM_DIRECT_CAMERA_INVERT_Y     0
#  define PLATFORM_DIRECT_CAMERA_REFERENCE_FPS 60.0f
#  define PLATFORM_DIRECT_CAMERA_MAX_DT       0.10f
#endif

// The mouse-queue look path (everything without a direct pad camera) turns
// accumulated pointer pixels into angles as (sensitivity*0.6+0.2)^3 * scale.
// Vanilla's cube curve is 8.0; the 3DS panel-look carries a +10% owner bump in
// DsWorldTuning.h. Same structural slot as PS2's DIRECT_CAMERA_SCALE above.
#ifndef PLATFORM_MOUSE_CAMERA_SCALE
#  define PLATFORM_MOUSE_CAMERA_SCALE         8.0f
#endif
