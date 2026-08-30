#pragma once

class EntityPlayer;

// net.minecraft.src.MovementInput
class MovementInput
{
public:
	MovementInput();
	virtual ~MovementInput() = default;

	virtual void updatePlayerMoveState(EntityPlayer *entityplayer);
	virtual void resetKeyState();
	virtual void checkKeyForMovementInput(int i, bool flag);

	float moveStrafe;
	float moveForward;
	bool field_1177_c;
	bool jump;
	bool sneak;
};
