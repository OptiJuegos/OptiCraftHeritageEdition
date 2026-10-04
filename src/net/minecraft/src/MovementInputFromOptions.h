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
	// Toggle-sneak state (GameSettings::toggleShift): a rising edge of the raw
	// sneak input latches sneaking on until the next edge, instead of holding.
	bool toggleSneakLatched;
	bool toggleSneakRawHeld;
};
