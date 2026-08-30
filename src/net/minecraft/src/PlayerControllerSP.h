#pragma once

#include "PlayerController.h"

// net.minecraft.src.PlayerControllerSP
class PlayerControllerSP : public PlayerController
{
public:
	PlayerControllerSP(Minecraft *minecraft);

	void flipPlayer(EntityPlayer *entityplayer) override;
	bool sendBlockRemoved(int_t i, int_t j, int_t k, int_t l) override;
	void clickBlock(int_t i, int_t j, int_t k, int_t l) override;
	void resetBlockRemoving() override;
	void sendBlockRemoving(int_t i, int_t j, int_t k, int_t l) override;
	void setPartialTime(float f) override;
	float getBlockReachDistance() override;
	void onWorldChanged(World *world) override;
	void updateController() override;
	bool func_35642_f() override;

private:
	int_t field_1074_c;
	int_t field_1073_d;
	int_t field_1072_e;
	float curBlockDamage;
	float prevBlockDamage;
	float field_1069_h;
	int_t blockHitWait;
};
