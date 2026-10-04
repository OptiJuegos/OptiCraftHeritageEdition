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
#include "net/minecraft/src/legacy/LegacyCraftingScreen.h"
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
#include "Block.h"

#if defined(CTR_PLATFORM)
namespace
{
// Auto-jump probes (see EntityPlayerSP::queueAutoJump). The step block must be
// genuinely solid — leaves, plants, circuits and liquids report a non-solid
// material — and the two blocks above it must be clear.
bool autoJumpIsSolidTile(World *world, int_t x, int_t y, int_t z)
{
	if (world == nullptr)
		return false;
	const int_t blockId = world->getBlockId(x, y, z);
	if (blockId <= 0 || blockId >= Block::BLOCK_REGISTRY_SIZE)
		return false;
	const Block *block = Block::blocksList[blockId];
	return block != nullptr && block->blockMaterial != nullptr && block->blockMaterial->isSolid();
}

// Whether it is worth auto-jumping onto this block. Decided by the real top
// of the obstacle's collision box measured against the player's feet, not by
// block type: walking (EntityLiving's 0.5 stepHeight) already climbs a bottom
// slab at foot level and both halves of a stair block, so those must never
// arm a hop; anything between that and the jump apex (~1.3 blocks) -- a full
// one-block step, a stacked double slab, a top-half slab -- is exactly what
// the hop is for; anything higher (fences read 1.5) is unreachable and must
// not fire. The old type blacklist answered neither question: it let the
// player get stuck against top-half slabs and one-block slab treads, while
// every "slab" report of a spurious hop traced to a tread that really was a
// full block high. Fences, gates, trapdoors and signs stay excluded
// regardless of height: even where their box reads reachable there is no
// usable surface to land on. Stairs (render type 10) always walk up in two
// 0.5 steps no matter which side they are met from, so they never hop.
bool autoJumpIsJumpable(World *world, int_t x, int_t y, int_t z, double feetY)
{
	if (world == nullptr)
		return false;
	const int_t blockId = world->getBlockId(x, y, z);
	if (blockId <= 0 || blockId >= Block::BLOCK_REGISTRY_SIZE)
		return false;
	Block *block = Block::blocksList[blockId];
	if (block == nullptr)
		return false;
	if (block == Block::fence || block == Block::fenceIron || block == Block::fenceGate ||
		block == Block::trapdoor ||
		block == Block::signPost || block == Block::signWall)
		return false;
	if (block->getRenderType() == 10) // 10 == SHAPE_STAIRS
		return false;
	// The obstacle's real top: setBlockBoundsBasedOnState is the same dance
	// the collision path runs before reading a cell's box (BlockStep picks
	// the half its metadata says), and the bounds members are shared
	// per-block state that every other physics query rewrites anyway, so
	// touching them here is exactly as safe as any tick doing it.
	block->setBlockBoundsBasedOnState(world, x, y, z);
	const double rise = (double)y + block->maxY - feetY;
	// stepHeight territory: walking gets there, a hop would only look wrong
	// (the 0.01 absorbs float noise on the exact 0.5 slab rise).
	if (rise <= 0.5 + 0.01)
		return false;
	// Above the jump apex: not worth arming, the hop would slam into the
	// face and fall back.
	if (rise > 1.25)
		return false;
	return true;
}
} // namespace
#endif

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
#if defined(CTR_PLATFORM)
	autoJumpTime = 0;
	autoJumpPending = false;
#endif
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
#if defined(CTR_PLATFORM)
	const double prevX = posX;
	const double prevZ = posZ;
#endif
	EntityPlayer::moveEntity(d, d1, d2);
#if defined(CTR_PLATFORM)
	queueAutoJump(prevX, prevZ, d, d2);
#endif
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
#if defined(CTR_PLATFORM)
	// Consume the jump armed by queueAutoJump on the previous tick: the real hop
	// still goes through EntityLiving's isJumping/jumpTicks path, so all this
	// does is hold isJumping for as long as the armed window lasts.
	if (autoJumpTime > 0)
	{
		const bool stillArmed = mc != nullptr && mc->gameSettings != nullptr && mc->gameSettings->autoJump &&
			onGround && !capabilities.isFlying && !isSneaking() && moveForward > 0.0f;
		if (stillArmed)
		{
			isJumping = true;
			// Mark the pending jump as ours: jump() uses this to drop
			// EntityLiving's sprint boost (see below).
			autoJumpPending = true;
			--autoJumpTime;
		}
		else
		{
			autoJumpTime = 0;
			autoJumpPending = false;
		}
	}
	else
	{
		// The armed window ran out without our hop firing (e.g. jumpTicks
		// still cooling down from a manual jump): retire the pending flag
		// too, or the next manual jump would lose its sprint boost once.
		autoJumpPending = false;
	}
#endif
}

#if defined(CTR_PLATFORM)
// Arm an auto-jump when this tick's move crossed the middle of a tile and the
// tile ahead is a one-block step with two clear blocks above it. Detection runs
// right after the move (so the crossing is known) and consumption on the next
// tick's input pass; the small window absorbs one missed condition (a fresh
// jump, a frame of sneak) without swallowing the hop.
void EntityPlayerSP::queueAutoJump(double prevX, double prevZ, double moveX, double moveZ)
{
	if (autoJumpTime > 0)
		return;
	if (mc == nullptr || mc->gameSettings == nullptr || !mc->gameSettings->autoJump)
		return;
	if (!onGround || capabilities.isFlying || isSneaking() || moveForward <= 0.0f)
		return;
	// Only when the move crossed the middle of a tile (a half-block boundary).
	if (MathHelper::floor_double(prevX * 2.0) == MathHelper::floor_double(posX * 2.0) &&
		MathHelper::floor_double(prevZ * 2.0) == MathHelper::floor_double(posZ * 2.0))
		return;

	const double dist = MathHelper::sqrt_double(moveX * moveX + moveZ * moveZ);
	if (dist <= 0.0)
		return;
	// One block ahead along the movement direction, as in the reference.
	const int_t blockX = MathHelper::floor_double(posX + moveX / dist);
	const int_t blockZ = MathHelper::floor_double(posZ + moveZ / dist);
	// Measure from the TRUE feet. posY carries the ySize step-smoothing
	// offset (Entity::moveEntity subtracts it so the camera glides up after
	// every 0.5 step), and for the ~2-4 ticks after stepping onto a
	// half-height tread it depressed both the probe cell (stepY, one level
	// low) and the rise measurement (minY - ySize, up to 0.51 low) at once:
	// the next tread of a perfectly smooth +0.5 slab staircase then read as
	// a ~1.0 step and armed a phantom hop -- "running up slab stairs gives
	// exactly 2 jumps". The bounding box's minY is the feet the physics
	// actually steps on, untouched by the smoothing.
	const double feetY = boundingBox->minY;
	// (int)(y-1) in the reference convention: with posY = feet + yOffset
	// (ySize aside) that is the cell the feet are in.
	const int_t stepY = MathHelper::floor_double(feetY + (double)yOffset - 1.0);
	if (!autoJumpIsSolidTile(worldObj, blockX, stepY, blockZ))
		return;
	// Two blocks of headroom above the step.
	if (autoJumpIsSolidTile(worldObj, blockX, stepY + 1, blockZ) ||
		autoJumpIsSolidTile(worldObj, blockX, stepY + 2, blockZ))
		return;
	if (!autoJumpIsJumpable(worldObj, blockX, stepY, blockZ, feetY))
		return;

	autoJumpTime = 2;
}

// The auto-hop goes through EntityLiving's isJumping path so the real jump
// keeps its cooldown, potion handling and stats — but it must not inherit the
// sprint boost living in EntityLiving::jump(): those +0.2 of extra horizontal
// velocity are the player's manual sprint-jump skill, and on them an armed hop
// overshoots the step it was armed for. On staircases every boosted landing
// lines up the next one-block tread and the game chains hop after hop — the
// "keeps jumping while running up the slab stairs" report. Reference
// auto-jumps land you ON the step: drop the boost (and with it the
// sprint-jump exhaustion rate) for this hop only, then put sprinting back.
void EntityPlayerSP::jump()
{
	const bool suppressBoost = autoJumpPending;
	autoJumpPending = false;
	const bool wasSprinting = isSprinting();
	if (suppressBoost && wasSprinting)
		setSprinting(false);
	EntityPlayer::jump();
	if (suppressBoost && wasSprinting)
		setSprinting(true);
}
#endif

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
	if (mc != nullptr && mc->isSplitScreenActive())
	{
		if (this == mc->thePlayer2)
		{
			mc->closePlayerScreen(1);
			return;
		}
		else if (mc->isPlayerScreenActive(0))
		{
			mc->closePlayerScreen(0);
			return;
		}
	}
	mc->displayGuiScreen(nullptr);
}

void EntityPlayerSP::displayGUIEditSign(TileEntitySign *tileentitysign)
{
	mc->displayGuiScreen(new GuiEditSign(tileentitysign));
}

void EntityPlayerSP::displayGUIChest(IInventory *iinventory)
{
	if (mc != nullptr && mc->isSplitScreenActive())
	{
		const int pIdx = (this == mc->thePlayer2) ? 1 : 0;
		mc->displayPlayerScreen(pIdx, new GuiChest(inventory, iinventory, this));
		return;
	}
	mc->displayGuiScreen(new GuiChest(inventory, iinventory, this));
}

void EntityPlayerSP::displayWorkbenchGUI(int_t i, int_t j, int_t k)
{
	if (mc != nullptr && mc->gameSettings != nullptr && mc->gameSettings->legacyUI && mc->gameSettings->legacyCrafting)
	{
		if (mc->isSplitScreenActive())
		{
			const int pIdx = (this == mc->thePlayer2) ? 1 : 0;
			mc->displayPlayerScreen(pIdx, new LegacyCraftingScreen(inventory, worldObj, i, j, k, false, this));
			return;
		}
		mc->displayGuiScreen(new LegacyCraftingScreen(inventory, worldObj, i, j, k, false, this));
		return;
	}
	if (mc != nullptr && mc->isSplitScreenActive())
	{
		const int pIdx = (this == mc->thePlayer2) ? 1 : 0;
		mc->displayPlayerScreen(pIdx, new GuiCrafting(inventory, worldObj, i, j, k, this));
		return;
	}
	mc->displayGuiScreen(new GuiCrafting(inventory, worldObj, i, j, k, this));
}

void EntityPlayerSP::displayGUIFurnace(TileEntityFurnace *tileentityfurnace)
{
	if (mc != nullptr && mc->isSplitScreenActive())
	{
		const int pIdx = (this == mc->thePlayer2) ? 1 : 0;
		mc->displayPlayerScreen(pIdx, new GuiFurnace(inventory, tileentityfurnace, this));
		return;
	}
	mc->displayGuiScreen(new GuiFurnace(inventory, tileentityfurnace, this));
}

void EntityPlayerSP::displayGUIDispenser(TileEntityDispenser *tileentitydispenser)
{
	if (mc != nullptr && mc->isSplitScreenActive())
	{
		const int pIdx = (this == mc->thePlayer2) ? 1 : 0;
		mc->displayPlayerScreen(pIdx, new GuiDispenser(inventory, tileentitydispenser, this));
		return;
	}
	mc->displayGuiScreen(new GuiDispenser(inventory, tileentitydispenser, this));
}

void EntityPlayerSP::displayGUIEnchantment(int_t i, int_t j, int_t k)
{
	if (mc != nullptr && mc->isSplitScreenActive())
	{
		const int pIdx = (this == mc->thePlayer2) ? 1 : 0;
		mc->displayPlayerScreen(pIdx, new GuiEnchantment(inventory, worldObj, i, j, k, this));
		return;
	}
	mc->displayGuiScreen(new GuiEnchantment(inventory, worldObj, i, j, k, this));
}

void EntityPlayerSP::displayGUIBrewingStand(TileEntityBrewingStand *tileentitybrewingstand)
{
	if (mc != nullptr && mc->isSplitScreenActive())
	{
		const int pIdx = (this == mc->thePlayer2) ? 1 : 0;
		mc->displayPlayerScreen(pIdx, new GuiBrewingStand(inventory, tileentitybrewingstand, this));
		return;
	}
	mc->displayGuiScreen(new GuiBrewingStand(inventory, tileentitybrewingstand, this));
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
	if (mc != nullptr && mc->effectRenderer != nullptr && mc->theWorld != nullptr && entity != nullptr)
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
