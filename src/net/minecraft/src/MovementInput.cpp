#include "MovementInput.h"

MovementInput::MovementInput()
{
	moveStrafe = 0.0f;
	moveForward = 0.0f;
	field_1177_c = false;
	jump = false;
	sneak = false;
}

void MovementInput::updatePlayerMoveState(EntityPlayer *entityplayer)
{
}

void MovementInput::resetKeyState()
{
}

void MovementInput::checkKeyForMovementInput(int i, bool flag)
{
}
