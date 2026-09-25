#pragma once

#include "MovementInput.h"

class GameSettings;

// net.minecraft.src.MovementInputFromOptions
class MovementInputFromOptions : public MovementInput
{
public:
	MovementInputFromOptions(GameSettings *gamesettings, int port = 0);

	void checkKeyForMovementInput(int i, bool flag) override;
	void resetKeyState() override;
	void updatePlayerMoveState(EntityPlayer *entityplayer) override;

	int getPadPort() const { return padPort; }
	void setPadPort(int port) { padPort = port; }

private:
	GameSettings *gameSettings;
	int padPort;
};
