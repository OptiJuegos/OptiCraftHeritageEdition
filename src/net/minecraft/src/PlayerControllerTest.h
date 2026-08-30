#pragma once

#include "PlayerController.h"

// net.minecraft.src.PlayerControllerTest
class PlayerControllerTest : public PlayerController
{
public:
	PlayerControllerTest(Minecraft *minecraft);

	void initializePlayer(EntityPlayer *entityplayer) override;
	bool shouldDrawHUD() override;
	void onWorldChanged(World *world) override;
	void updateController() override;
};
