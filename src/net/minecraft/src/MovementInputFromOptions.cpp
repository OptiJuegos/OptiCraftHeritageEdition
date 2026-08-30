#include "MovementInputFromOptions.h"
#include "platform/Log.h"

#include "GameSettings.h"
#include "KeyBinding.h"

#include "platform/Input.h"
#include "platform/PlatformConfig.h"
#include "platform/PlatformTuning.h"

namespace
{
float clampMovement(float value)
{
    if (value < -1.0f) return -1.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}
}

MovementInputFromOptions::MovementInputFromOptions(GameSettings *gamesettings)
    : gameSettings(gamesettings)
{
}

void MovementInputFromOptions::checkKeyForMovementInput(int i, bool flag)
{
    KeyBinding::setKeyBindState(i, flag);
}

void MovementInputFromOptions::resetKeyState()
{
    KeyBinding::unPressAllKeys();
}

void MovementInputFromOptions::updatePlayerMoveState(EntityPlayer *entityplayer)
{
    (void)entityplayer;
    moveStrafe = 0.0f;
    moveForward = 0.0f;
    if (gameSettings->keyBindForward->pressed) moveForward++;
    if (gameSettings->keyBindBack->pressed) moveForward--;
    if (gameSettings->keyBindLeft->pressed) moveStrafe++;
    if (gameSettings->keyBindRight->pressed) moveStrafe--;
    jump = gameSettings->keyBindJump->pressed;
    sneak = gameSettings->keyBindSneak->pressed;

#if PLATFORM_DIRECT_ANALOG_MOVEMENT
    const PlatformGamepadSnapshot pad = platformGamepadSnapshot(0);
    if (pad.connected)
    {
        // PlatformGamepadSnapshot normalizes axes. Minecraft wants positive
        // forward and positive strafe-left, matching the W/A/S/D states.
        moveStrafe += -pad.leftX * PLATFORM_ANALOG_MOVE_SCALE;
        moveForward += -pad.leftY * PLATFORM_ANALOG_MOVE_SCALE;
        moveStrafe = clampMovement(moveStrafe);
        moveForward = clampMovement(moveForward);

#if MC_LOG_LEVEL >= 2
        if (pad.leftX != 0.0f || pad.leftY != 0.0f)
        {
            static int moveLogFrames = 0;
            if (++moveLogFrames >= 30)
            {
                moveLogFrames = 0;
                MC_LOG_DEBUG("input", "move input L=%.3f,%.3f -> strafe=%.3f forward=%.3f jump=%d sneak=%d\n",
                             pad.leftX, pad.leftY, moveStrafe, moveForward, jump ? 1 : 0, sneak ? 1 : 0);
            }
        }
#endif
    }
#endif

    if (sneak)
    {
        moveStrafe *= 0.29999999999999999;
        moveForward *= 0.29999999999999999;
    }
}
