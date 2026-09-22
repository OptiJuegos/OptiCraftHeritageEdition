#include "EntityPlayerSP.h"

#include "GameSettings.h"
#include "skin/SkinManager.h"
#include "GuiIngame.h"
#include "Material.h"
#include "MathHelper.h"
#include "Minecraft.h"
#include "MovementInput.h"
#include "PlayerController.h"
#include "Session.h"
#include "SoundManager.h"
#include "StatFileWriter.h"
#include "World.h"
#include "java/String.h"
#include "AchievementList.h"
#include "AxisAlignedBB.h"
#include "EntityPickupFX.h"
#include "EntityCrit2FX.h"
#include "GuiChest.h"
#include "GuiCrafting.h"
#include "GuiDispenser.h"
#include "GuiEnchantment.h"
#include "GuiEditSign.h"
#include "GuiFurnace.h"
#include "GuiBrewingStand.h"
#include "GuiWinGame.h"
#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "Achievement.h"
#include "EffectRenderer.h"
#include "DamageSource.h"
#include "GuiAchievement.h"
#include "NBTTagCompound.h"
#include "Potion.h"
#include "PotionEffect.h"

EntityPlayerSP::EntityPlayerSP(Minecraft *minecraft, World *world, Session *session, int_t i)
	: EntityPlayer(world)
	, movementInput(nullptr)
	, sprintingTicksLeft(0)
	, renderArmYaw(0.0f)
	, renderArmPitch(0.0f)
	, prevRenderArmYaw(0.0f)
	, prevRenderArmPitch(0.0f)
	, mc(minecraft)
	, sprintToggleTimer(0)
{
	ensureEntityInit();
	dimension = i;
	if (session != nullptr)
	{
		if (!session->username.empty())
			skinUrl = "http://s3.amazonaws.com/MinecraftSkins/" + session->username + ".png";
		username = session->username;
	}

	const std::string activeSkin = SkinManager::getActiveSkinTexture();
	if (!activeSkin.empty())
	{
		texture = activeSkin;
		skinUrl = "";
	}
}

EntityPlayerSP::~EntityPlayerSP()
{
	delete movementInput;
}

void EntityPlayerSP::moveEntity(double d, double d1, double d2)
{
	EntityPlayer::moveEntity(d, d1, d2);
}

void EntityPlayerSP::updatePlayerActionState()
{
	EntityPlayer::updatePlayerActionState();
	if (movementInput != nullptr)
	{
		moveStrafing = movementInput->moveStrafe;
		moveForward = movementInput->moveForward;
		isJumping = movementInput->jump;
		prevRenderArmYaw = renderArmYaw;
		prevRenderArmPitch = renderArmPitch;
		renderArmPitch += (rotationPitch - renderArmPitch) * 0.5f;
		renderArmYaw += (rotationYaw - renderArmYaw) * 0.5f;
	}
}

void EntityPlayerSP::onLivingUpdate()
{
	if (sprintingTicksLeft > 0)
	{
		--sprintingTicksLeft;
		if (sprintingTicksLeft == 0)
			setSprinting(false);
	}
	if (sprintToggleTimer > 0)
		--sprintToggleTimer;

	if (mc->playerController->func_35643_e())
	{
		posX = posZ = 0.5;
		posX = 0.0;
		posZ = 0.0;
		rotationYaw = (float)ticksExisted / 12.0f;
		rotationPitch = 10.0f;
		posY = 68.5;
		return;
	}

	// The "Press E to open your inventory" reminder is a Java Edition prompt;
	// the Legacy UI has its own control prompts on the HUD, so it is not shown
	// there. Earned achievements still pop up as usual.
	if (!mc->gameSettings->legacyUI &&
	    !mc->statFileWriter->hasAchievementUnlocked(AchievementList::openInventory))
		mc->guiAchievement->queueAchievementInformation(AchievementList::openInventory);

	prevTimeInPortal = timeInPortal;
	if (inPortal)
	{
		if (!worldObj->multiplayerWorld && ridingEntity != nullptr)
			mountEntity(nullptr);
		if (mc->currentScreen != nullptr)
			mc->displayGuiScreen(nullptr);
		if (timeInPortal == 0.0f)
			mc->sndManager->playSoundFX("portal.trigger", 1.0f, rand.nextFloat() * 0.4f + 0.8f);
		timeInPortal += 0.0125f;
		if (timeInPortal >= 1.0f)
		{
			timeInPortal = 1.0f;
			if (!worldObj->multiplayerWorld)
			{
				timeUntilPortal = 10;
				mc->sndManager->playSoundFX("portal.travel", 1.0f, rand.nextFloat() * 0.4f + 0.8f);
				const int_t targetDimension = dimension == -1 ? 0 : -1;
				mc->usePortal(targetDimension);
				triggerAchievement(AchievementList::portal);
			}
		}
		inPortal = false;
	}
	else if (isPotionActive(Potion::confusion) && getActivePotionEffect(Potion::confusion)->getDuration() > 60)
	{
		timeInPortal += (2.0f / 3.0f) * 0.01f;
		if (timeInPortal > 1.0f)
			timeInPortal = 1.0f;
	}
	else
	{
		if (timeInPortal > 0.0f)
			timeInPortal -= 0.05f;
		if (timeInPortal < 0.0f)
			timeInPortal = 0.0f;
	}
	if (timeUntilPortal > 0)
		timeUntilPortal--;

	bool wasJumping = movementInput != nullptr && movementInput->jump;
	const float sprintThreshold = 0.8f;
	bool wasMovingForward = movementInput != nullptr && movementInput->moveForward >= sprintThreshold;
	if (movementInput != nullptr)
		movementInput->updatePlayerMoveState(this);
#ifdef PS2_PLATFORM
	// While a screen is open the player must stand still. On PC that happens by
	// itself: movement comes from key events and opening a screen releases them
	// (setIngameNotInFocus -> resetPlayerKeyState). The PS2 analog stick is read
	// straight from the pad snapshot in MovementInputFromOptions, so it never saw
	// that release and kept walking the player around behind the inventory.
	// Clear it here, where the Minecraft pointer is in scope; this also covers the
	// D-Pad, whose synthesized key events still reach handleKeyPress.
	if (movementInput != nullptr && mc != nullptr && mc->currentScreen != nullptr)
	{
		movementInput->moveStrafe  = 0.0f;
		movementInput->moveForward = 0.0f;
		movementInput->jump        = false;
		movementInput->sneak       = false;
	}
#endif
	if (isUsingItem() && movementInput != nullptr)
	{
		movementInput->moveStrafe *= 0.2f;
		movementInput->moveForward *= 0.2f;
		sprintToggleTimer = 0;
	}
	if (movementInput != nullptr && movementInput->sneak && ySize < 0.2f)
		ySize = 0.2f;
	pushOutOfBlocks(posX - (double)width * 0.35, boundingBox->minY + 0.5, posZ + (double)width * 0.35);
	pushOutOfBlocks(posX - (double)width * 0.35, boundingBox->minY + 0.5, posZ - (double)width * 0.35);
	pushOutOfBlocks(posX + (double)width * 0.35, boundingBox->minY + 0.5, posZ - (double)width * 0.35);
	pushOutOfBlocks(posX + (double)width * 0.35, boundingBox->minY + 0.5, posZ + (double)width * 0.35);

	const bool hasFoodForSprinting = (float)getFoodStats()->getFoodLevel() > 6.0f;
	if (movementInput != nullptr && onGround && !wasMovingForward && movementInput->moveForward >= sprintThreshold &&
		!isSprinting() && hasFoodForSprinting && !isUsingItem() && !isPotionActive(Potion::blindness))
	{
		if (sprintToggleTimer == 0)
			sprintToggleTimer = 7;
		else
		{
			setSprinting(true);
			sprintToggleTimer = 0;
		}
	}

	if (isSneaking())
		sprintToggleTimer = 0;
	if (movementInput != nullptr && isSprinting() &&
		(movementInput->moveForward < sprintThreshold || isCollidedHorizontally || !hasFoodForSprinting))
	{
		setSprinting(false);
	}

	if (movementInput != nullptr && capabilities.allowFlying && !wasJumping && movementInput->jump)
	{
		if (flyToggleTimer == 0)
			flyToggleTimer = 7;
		else
		{
			capabilities.isFlying = !capabilities.isFlying;
			func_50009_aI();
			flyToggleTimer = 0;
		}
	}

	if (movementInput != nullptr && capabilities.isFlying)
	{
		if (movementInput->sneak)
			motionY -= 0.15;
		if (movementInput->jump)
			motionY += 0.15;
	}

	EntityPlayer::onLivingUpdate();
	if (onGround && capabilities.isFlying)
	{
		capabilities.isFlying = false;
		func_50009_aI();
	}
}

void EntityPlayerSP::setSprinting(bool value)
{
	EntityPlayer::setSprinting(value);
	sprintingTicksLeft = value ? 600 : 0;
}

float EntityPlayerSP::getFOVMultiplier()
{
	float multiplier = 1.0f;
	if (capabilities.isFlying)
		multiplier *= 1.1f;
	multiplier *= (landMovementFactor * getSpeedModifier() / speedOnGround + 1.0f) / 2.0f;
	if (isUsingItem() && getItemInUse() != nullptr && Item::bow != nullptr && getItemInUse()->itemID == Item::bow->shiftedIndex)
	{
		float use = (float)getItemInUseDuration() / 20.0f;
		if (use > 1.0f)
			use = 1.0f;
		else
			use *= use;
		multiplier *= 1.0f - use * 0.15f;
	}
	return multiplier;
}

void EntityPlayerSP::resetPlayerKeyState()
{
	if (movementInput != nullptr)
		movementInput->resetKeyState();
}

void EntityPlayerSP::handleKeyPress(int_t i, bool flag)
{
	if (movementInput != nullptr)
		movementInput->checkKeyForMovementInput(i, flag);
}

void EntityPlayerSP::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	EntityPlayer::writeEntityToNBT(nbttagcompound);
	nbttagcompound->setInteger("Score", score);
}

void EntityPlayerSP::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	EntityPlayer::readEntityFromNBT(nbttagcompound);
	score = nbttagcompound->getInteger("Score");
}

void EntityPlayerSP::closeScreen()
{
	EntityPlayer::closeScreen();
	mc->displayGuiScreen(nullptr);
}

void EntityPlayerSP::displayGUIEditSign(TileEntitySign *tileentitysign)
{
	mc->displayGuiScreen(new GuiEditSign(tileentitysign));
}

void EntityPlayerSP::displayGUIChest(IInventory *iinventory)
{
	mc->displayGuiScreen(new GuiChest(inventory, iinventory));
}

void EntityPlayerSP::displayWorkbenchGUI(int_t i, int_t j, int_t k)
{
	mc->displayGuiScreen(new GuiCrafting(inventory, worldObj, i, j, k));
}

void EntityPlayerSP::displayGUIFurnace(TileEntityFurnace *tileentityfurnace)
{
	mc->displayGuiScreen(new GuiFurnace(inventory, tileentityfurnace));
}

void EntityPlayerSP::displayGUIDispenser(TileEntityDispenser *tileentitydispenser)
{
	mc->displayGuiScreen(new GuiDispenser(inventory, tileentitydispenser));
}

void EntityPlayerSP::displayGUIEnchantment(int_t i, int_t j, int_t k)
{
	mc->displayGuiScreen(new GuiEnchantment(inventory, worldObj, i, j, k));
}

void EntityPlayerSP::displayGUIBrewingStand(TileEntityBrewingStand *tileentitybrewingstand)
{
	mc->displayGuiScreen(new GuiBrewingStand(inventory, tileentitybrewingstand));
}

void EntityPlayerSP::onCriticalHit(Entity *entity)
{
	if (entity != nullptr && mc != nullptr && mc->effectRenderer != nullptr)
		mc->effectRenderer->addEffect(new EntityCrit2FX(mc->theWorld, entity));
}

void EntityPlayerSP::onEnchantmentCritical(Entity *entity)
{
	if (entity != nullptr && mc != nullptr && mc->effectRenderer != nullptr)
		mc->effectRenderer->addEffect(new EntityCrit2FX(mc->theWorld, entity, "magicCrit"));
}

void EntityPlayerSP::onItemPickup(Entity *entity, int_t i)
{
	(void)i;
	mc->effectRenderer->addEffect(new EntityPickupFX(mc->theWorld, entity, this, -0.5f));
}

int_t EntityPlayerSP::getPlayerArmorValue()
{
	return inventory->getTotalArmorValue();
}

void EntityPlayerSP::sendChatMessage(const std::string &)
{
}

bool EntityPlayerSP::isSneaking()
{
	return movementInput != nullptr && movementInput->sneak && !sleeping;
}

void EntityPlayerSP::setHealth(int_t i)
{
	int_t j = health - i;
	if (j <= 0)
	{
		setEntityHealth(i);
		if (j < 0)
			heartsLife = heartsHalvesLife / 2;
	}
	else
	{
		field_9346_af = j;
		setEntityHealth(getHealth());
		heartsLife = heartsHalvesLife;
		damageEntity(DamageSource::generic, j);
		hurtTime = maxHurtTime = 10;
	}
}

void EntityPlayerSP::respawnPlayer()
{
	mc->respawn(false, 0, false);
}

void EntityPlayerSP::travelToTheEnd(int_t targetDimension)
{
	if (worldObj == nullptr || worldObj->multiplayerWorld)
		return;

	if (dimension == 1 && targetDimension == 1)
	{
		if (AchievementList::theEnd2 != nullptr)
			triggerAchievement(AchievementList::theEnd2);
		mc->displayGuiScreen(new GuiWinGame());
	}
	else
	{
		if (AchievementList::theEnd != nullptr)
			triggerAchievement(AchievementList::theEnd);
		mc->sndManager->playSoundFX("portal.travel", 1.0f, rand.nextFloat() * 0.4f + 0.8f);
		mc->usePortal(1);
	}
}

void EntityPlayerSP::handleItemUseFinish()
{
	EntityPlayer::handleItemUseFinish();
}

void EntityPlayerSP::addChatMessage(const std::string &s)
{
	mc->ingameGUI->addChatMessageTranslate(s);
}


void EntityPlayerSP::addStat(StatBase *statbase, int_t i)
{
	if (statbase == nullptr)
		return;
	if (statbase->isAchievement())
	{
		Achievement *achievement = static_cast<Achievement *>(statbase);
		if (achievement->parentAchievement == nullptr || mc->statFileWriter->hasAchievementUnlocked(achievement->parentAchievement))
		{
			if (!mc->statFileWriter->hasAchievementUnlocked(achievement))
				mc->guiAchievement->queueTakenAchievement(achievement);
			mc->statFileWriter->readStat(statbase, i);
		}
	}
	else
	{
		mc->statFileWriter->readStat(statbase, i);
	}
}

bool EntityPlayerSP::isClientWorld() const
{
	return true;
}

bool EntityPlayerSP::isBlockTranslucent(int_t i, int_t j, int_t k)
{
	return worldObj->isBlockNormalCube(i, j, k);
}

bool EntityPlayerSP::pushOutOfBlocks(double d, double d1, double d2)
{
	int_t i = MathHelper::floor_double(d);
	int_t j = MathHelper::floor_double(d1);
	int_t k = MathHelper::floor_double(d2);
	double d3 = d - (double)i;
	double d4 = d2 - (double)k;
	if (isBlockTranslucent(i, j, k) || isBlockTranslucent(i, j + 1, k))
	{
		bool flag = !isBlockTranslucent(i - 1, j, k) && !isBlockTranslucent(i - 1, j + 1, k);
		bool flag1 = !isBlockTranslucent(i + 1, j, k) && !isBlockTranslucent(i + 1, j + 1, k);
		bool flag2 = !isBlockTranslucent(i, j, k - 1) && !isBlockTranslucent(i, j + 1, k - 1);
		bool flag3 = !isBlockTranslucent(i, j, k + 1) && !isBlockTranslucent(i, j + 1, k + 1);
		byte_t byte0 = -1;
		double d5 = 9999.0;
		if (flag && d3 < d5) { d5 = d3; byte0 = 0; }
		if (flag1 && 1.0 - d3 < d5) { d5 = 1.0 - d3; byte0 = 1; }
		if (flag2 && d4 < d5) { d5 = d4; byte0 = 4; }
		if (flag3 && 1.0 - d4 < d5) { byte0 = 5; }
		float f = 0.1f;
		if (byte0 == 0) motionX = -f;
		if (byte0 == 1) motionX = f;
		if (byte0 == 4) motionZ = -f;
		if (byte0 == 5) motionZ = f;
	}
	return false;
}

void EntityPlayerSP::setXPStats(float progress, int_t total, int_t level)
{
	experience = progress;
	experienceTotal = total;
	experienceLevel = level;
}
