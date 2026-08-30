#include "PlayerControllerSP.h"

#include <cmath>

#include "Block.h"
#include "EntityPlayerSP.h"
#include "GuiIngame.h"
#include "InventoryPlayer.h"
#include "ItemStack.h"
#include "Minecraft.h"
#include "RenderGlobal.h"
#include "SoundManager.h"
#include "StepSound.h"
#include "World.h"

PlayerControllerSP::PlayerControllerSP(Minecraft *minecraft)
	: PlayerController(minecraft)
	, field_1074_c(-1)
	, field_1073_d(-1)
	, field_1072_e(-1)
	, curBlockDamage(0.0f)
	, prevBlockDamage(0.0f)
	, field_1069_h(0.0f)
	, blockHitWait(0)
{
}

void PlayerControllerSP::flipPlayer(EntityPlayer *entityplayer)
{
	entityplayer->rotationYaw = -180.0f;
#ifdef PS2_PLATFORM
	// orientCamera() interpolates prevRotationYaw -> rotationYaw every render.
	// PC normally hides this one-frame 0 -> -180 transition, but a PS2 world
	// frame can take a second and turns it into an apparent 180/360 spin at
	// spawn. Keep both endpoints of the interpolation at the spawn yaw.
	entityplayer->prevRotationYaw = entityplayer->rotationYaw;
	entityplayer->prevRotationPitch = entityplayer->rotationPitch;
#endif
}

bool PlayerControllerSP::sendBlockRemoved(int_t i, int_t j, int_t k, int_t l)
{
	int_t i1 = mc->theWorld->getBlockId(i, j, k);
	int_t j1 = mc->theWorld->getBlockMetadata(i, j, k);
	bool flag = PlayerController::sendBlockRemoved(i, j, k, l);
	ItemStack *itemstack = mc->thePlayer->getCurrentEquippedItem();
	bool flag1 = mc->thePlayer->canHarvestBlock(Block::blocksList[i1]);
	if (itemstack != nullptr)
	{
		itemstack->onDestroyBlock(i1, i, j, k, mc->thePlayer);
		if (itemstack->stackSize == 0)
		{
			itemstack->onItemDestroyedByUse(mc->thePlayer);
			mc->thePlayer->destroyCurrentEquippedItem();
		}
	}
	if (flag && flag1)
		Block::blocksList[i1]->harvestBlock(mc->theWorld, mc->thePlayer, i, j, k, j1);
	return flag;
}

void PlayerControllerSP::clickBlock(int_t i, int_t j, int_t k, int_t l)
{
	if (!mc->thePlayer->canPlayerEdit(i, j, k))
		return;

	mc->theWorld->onBlockHit(mc->thePlayer, i, j, k, l);
	int_t i1 = mc->theWorld->getBlockId(i, j, k);
	if (i1 > 0 && curBlockDamage == 0.0f)
		Block::blocksList[i1]->onBlockClicked(mc->theWorld, i, j, k, mc->thePlayer);
	if (i1 > 0 && Block::blocksList[i1]->blockStrength(mc->thePlayer) >= 1.0f)
		sendBlockRemoved(i, j, k, l);
}

void PlayerControllerSP::resetBlockRemoving()
{
	curBlockDamage = 0.0f;
	blockHitWait = 0;
}

void PlayerControllerSP::sendBlockRemoving(int_t i, int_t j, int_t k, int_t l)
{
	if (blockHitWait > 0)
	{
		blockHitWait--;
		return;
	}
	if (i == field_1074_c && j == field_1073_d && k == field_1072_e)
	{
		if (!mc->thePlayer->canPlayerEdit(i, j, k))
			return;

		int_t i1 = mc->theWorld->getBlockId(i, j, k);
		if (i1 == 0)
			return;
		Block *block = Block::blocksList[i1];
		curBlockDamage += block->blockStrength(mc->thePlayer);
		if (std::fmod(field_1069_h, 4.0f) == 0.0f && block != nullptr)
			mc->sndManager->playSound(block->stepSound->getStepSound(), (float)i + 0.5f, (float)j + 0.5f, (float)k + 0.5f, (block->stepSound->getVolume() + 1.0f) / 8.0f, block->stepSound->getPitch() * 0.5f);
		field_1069_h++;
		if (curBlockDamage >= 1.0f)
		{
			sendBlockRemoved(i, j, k, l);
			curBlockDamage = 0.0f;
			prevBlockDamage = 0.0f;
			field_1069_h = 0.0f;
			blockHitWait = 5;
		}
	}
	else
	{
		curBlockDamage = 0.0f;
		prevBlockDamage = 0.0f;
		field_1069_h = 0.0f;
		field_1074_c = i;
		field_1073_d = j;
		field_1072_e = k;
	}
}

void PlayerControllerSP::setPartialTime(float f)
{
	if (curBlockDamage <= 0.0f)
	{
		mc->ingameGUI->damageGuiPartialTime = 0.0f;
		mc->renderGlobal->damagePartialTime = 0.0f; // drives the block-cracking overlay in drawBlockBreaking
	}
	else
	{
		float f1 = prevBlockDamage + (curBlockDamage - prevBlockDamage) * f;
		mc->ingameGUI->damageGuiPartialTime = f1;
		mc->renderGlobal->damagePartialTime = f1;
	}
}

float PlayerControllerSP::getBlockReachDistance()
{
	return 4.0f;
}

void PlayerControllerSP::onWorldChanged(World *world)
{
	PlayerController::onWorldChanged(world);
}

void PlayerControllerSP::updateController()
{
	prevBlockDamage = curBlockDamage;
	mc->sndManager->playRandomMusicIfReady();
}


bool PlayerControllerSP::func_35642_f()
{
	return true;
}
