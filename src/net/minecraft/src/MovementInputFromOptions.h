#pragma once

#include "MovementInput.h"

class GameSettings;

// net.minecraft.src.MovementInputFromOptions
class MovementInputFromOptions : public MovementInput
{
public:
	MovementInputFromOptions(GameSettings *gamesettings);

	void checkKeyForMovementInput(int i, bool flag) override;
	void resetKeyState() override;
	void updatePlayerMoveState(EntityPlayer *entityplayer) override;

private:
	GameSettings *gameSettings;
};
