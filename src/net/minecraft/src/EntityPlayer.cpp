#include "EntityPlayer.h"
#include "java/Math.h"
#include "java/Arithmetic.h"

#include <cmath>
#include <vector>

#include "AchievementList.h"
#include "AxisAlignedBB.h"
#include "Block.h"
#include "BlockBed.h"
#include "ChunkCoordinates.h"
#include "Container.h"
#include "ContainerPlayer.h"
#include "DataWatcher.h"
#include "Entity.h"
#include "EntityArrow.h"
#include "EntityBoat.h"
#include "EntityCreeper.h"
#include "EntityFish.h"
#include "EntityGhast.h"
#include "EntityItem.h"
#include "EntityLiving.h"
#include "EntityMinecart.h"
#include "EntityMob.h"
#include "EntityPig.h"
#include "EntityWolf.h"
#include "DamageSource.h"
#include "EnchantmentHelper.h"
#include "Potion.h"
#include "PotionEffect.h"
#include "Vec3D.h"
#include "EnumAction.h"
#include "FoodStats.h"
#include "EnumStatus.h"
#include "IChunkProvider.h"
#include "IInventory.h"
#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "platform/PlatformTuning.h"
#include "Material.h"
#include "NBTTagCompound.h"
#include "NBTTagList.h"
#include "StatBase.h"
#include "StatList.h"
#include "TileEntityDispenser.h"
#include "TileEntityFurnace.h"
#include "TileEntitySign.h"
#include "World.h"
#include "WorldProvider.h"

EntityPlayer::EntityPlayer(World *world)
	: EntityLiving(world)
{
	inventory = new InventoryPlayer(this);
	field_9371_f = 0;
	score = 0;
	field_775_e = 0.0f;
	field_774_f = 0.0f;
	isSwinging = false;
	swingProgressInt = 0;
	dimension = 0;
	field_20066_r = field_20065_s = field_20064_t = 0.0;
	field_20063_u = field_20062_v = field_20061_w = 0.0;
	sleeping = false;
	bedChunkCoordinates = nullptr;
	sleepTimer = 0;
	field_22063_x = field_22062_y = field_22061_z = 0.0f;
	playerSpawnCoordinate = nullptr;
	startMinecartRidingCoordinate = nullptr;
	timeUntilPortal = 20;
	inPortal = false;
	timeInPortal = 0.0f;
	prevTimeInPortal = 0.0f;
	damageRemainder = 0;
	flyToggleTimer = 0;
	speedOnGround = 0.1f;
	speedInAir = 0.02f;
	xpCooldown = 0;
	experienceLevel = 0;
	experienceTotal = 0;
	experience = 0.0f;
	fishEntity = nullptr;
	itemInUse = nullptr;
	itemInUseCount = 0;
	inventorySlots = new ContainerPlayer(inventory, !world->multiplayerWorld);
	craftingInventory = inventorySlots;
	yOffset = 1.62f;
	ChunkCoordinates chunkcoordinates = world->getSpawnPoint();
	setLocationAndAngles((double)chunkcoordinates.x + 0.5, (double)(chunkcoordinates.y + 1), (double)chunkcoordinates.z + 0.5, 0.0f, 0.0f);
	health = 20;
	field_9351_C = "humanoid";
	field_9353_B = 180.0f;
	fireResistance = 20;
	texture = "/mob/char.png";
}

EntityPlayer::~EntityPlayer()
{
	if (craftingInventory != inventorySlots)
		delete craftingInventory;
	delete inventorySlots;
	delete inventory;
	delete bedChunkCoordinates;
	delete playerSpawnCoordinate;
	delete startMinecartRidingCoordinate;
}

void EntityPlayer::entityInit()
{
	EntityLiving::entityInit();
	if (!dataWatcher->hasObject(16))
		dataWatcher->addObject(16, (byte_t)0);
	if (!dataWatcher->hasObject(17))
		dataWatcher->addObject(17, (byte_t)0);
}

ItemStack *EntityPlayer::getItemInUse()
{
	return itemInUse;
}

int_t EntityPlayer::getItemInUseCount() const
{
	return itemInUseCount;
}

bool EntityPlayer::isUsingItem() const
{
	return itemInUse != nullptr;
}

int_t EntityPlayer::getItemInUseDuration()
{
	return isUsingItem() ? itemInUse->getMaxItemUseDuration() - itemInUseCount : 0;
}

void EntityPlayer::stopUsingItem()
{
	if (itemInUse != nullptr)
		itemInUse->onPlayerStoppedUsing(worldObj, this, itemInUseCount);
	clearItemInUse();
}

void EntityPlayer::clearItemInUse()
{
	itemInUse = nullptr;
	itemInUseCount = 0;
	if (!worldObj->multiplayerWorld)
		setEating(false);
}

bool EntityPlayer::isBlocking()
{
	return isUsingItem() && itemInUse->getItemUseAction() == EnumAction::block;
}

void EntityPlayer::onUpdate()
{
	if (itemInUse != nullptr)
	{
		ItemStack *current = inventory->getCurrentItem();
		if (current != itemInUse)
		{
			clearItemInUse();
		}
		else
		{
			if (itemInUseCount <= 25 && itemInUseCount % 4 == 0)
				updateItemUse(current, 5);
			if (--itemInUseCount == 0 && !worldObj->multiplayerWorld)
				handleItemUseFinish();
		}
	}

	if (xpCooldown > 0)
		--xpCooldown;

	if (isPlayerSleeping())
	{
		sleepTimer++;
		if (sleepTimer > 100)
		{
			sleepTimer = 100;
		}
		if (!worldObj->multiplayerWorld)
		{
			if (!isInBed())
			{
				wakeUpPlayer(true, true, false);
			}
			else if (worldObj->isDaytime())
			{
				wakeUpPlayer(false, true, true);
			}
		}
	}
	else if (sleepTimer > 0)
	{
		sleepTimer++;
		if (sleepTimer >= 110)
		{
			sleepTimer = 0;
		}
	}
	EntityLiving::onUpdate();
	if (!worldObj->multiplayerWorld && craftingInventory != nullptr && !craftingInventory->isUsableByPlayer(this))
	{
		closeScreen();
		craftingInventory = inventorySlots;
	}
	if (isBurning() && capabilities.disableDamage)
		extinguish();
	field_20066_r = field_20063_u;
	field_20065_s = field_20062_v;
	field_20064_t = field_20061_w;
	double d  = posX - field_20063_u;
	double d1 = posY - field_20062_v;
	double d2 = posZ - field_20061_w;
	double d3 = 10.0;
	if (d  >  d3) { field_20066_r = field_20063_u = posX; }
	if (d2 >  d3) { field_20064_t = field_20061_w = posZ; }
	if (d1 >  d3) { field_20065_s = field_20062_v = posY; }
	if (d  < -d3) { field_20066_r = field_20063_u = posX; }
	if (d2 < -d3) { field_20064_t = field_20061_w = posZ; }
	if (d1 < -d3) { field_20065_s = field_20062_v = posY; }
	field_20063_u += d  * 0.25;
	field_20061_w += d2 * 0.25;
	field_20062_v += d1 * 0.25;
	addStat(StatList::minutesPlayedStat, 1);
	if (ridingEntity == nullptr)
	{
		// Java dropped the reference and let GC reclaim it.
		delete startMinecartRidingCoordinate;
		startMinecartRidingCoordinate = nullptr;
	}
	if (!worldObj->multiplayerWorld)
		foodStats.onUpdate(this);
}

void EntityPlayer::handleHealthUpdate(byte_t state)
{
	if (state == 9)
		handleItemUseFinish();
	else
		EntityLiving::handleHealthUpdate(state);
}

void EntityPlayer::onItemUseFinish()
{
	handleItemUseFinish();
}

void EntityPlayer::updateItemUse(ItemStack *itemstack, int_t count)
{
	if (itemstack == nullptr)
		return;

	EnumAction action = itemstack->getItemUseAction();
	if (action == EnumAction::drink)
	{
		worldObj->playSoundAtEntity(this, "random.drink", 0.5f, worldObj->rand.nextFloat() * 0.1f + 0.9f);
	}
	else if (action == EnumAction::eat)
	{
		for (int_t i = 0; i < count; ++i)
		{
			Vec3D *velocity = Vec3D::createVector(((double)rand.nextFloat() - 0.5) * 0.1, Math::random() * 0.1 + 0.1, 0.0);
			velocity->rotateAroundX(-rotationPitch * 3.14159265358979323846f / 180.0f);
			velocity->rotateAroundY(-rotationYaw * 3.14159265358979323846f / 180.0f);
			const float positionXRandom = rand.nextFloat();
			const float positionYRandom = rand.nextFloat();
			Vec3D *position = Vec3D::createVector(((double)positionXRandom - 0.5) * 0.3, (double)(-positionYRandom) * 0.6 - 0.3, 0.6);
			position->rotateAroundX(-rotationPitch * 3.14159265358979323846f / 180.0f);
			position->rotateAroundY(-rotationYaw * 3.14159265358979323846f / 180.0f);
			Vec3D *worldPosition = position->addVector(posX, posY + (double)getEyeHeight(), posZ);
			Item *item = itemstack->getItem();
			int_t shiftedIndex = item != nullptr ? item->shiftedIndex : itemstack->itemID;
			worldObj->spawnParticle("iconcrack_" + std::to_string(shiftedIndex), worldPosition->xCoord, worldPosition->yCoord, worldPosition->zCoord, velocity->xCoord, velocity->yCoord + 0.05, velocity->zCoord);
		}

		const int_t volumeRandom = rand.nextInt(2);
		const float pitchRandom = rand.nextFloatDifference();
		worldObj->playSoundAtEntity(this, "random.eat", 0.5f + 0.5f * (float)volumeRandom, pitchRandom * 0.2f + 1.0f);
	}
}

void EntityPlayer::func_6420_o()
{
}

void EntityPlayer::handleItemUseFinish()
{
	if (itemInUse == nullptr)
		return;

	updateItemUse(itemInUse, 16);
	int_t oldSize = itemInUse->stackSize;
	ItemStack *oldStack = itemInUse;
	ItemStack *result = itemInUse->onFoodEaten(worldObj, this);

	if (result != oldStack || (result != nullptr && result->stackSize != oldSize))
	{
		ItemStack *&slot = inventory->mainInventory[inventory->currentItem];
		if (result != oldStack)
		{
			delete slot;
			slot = result;
		}
		if (slot != nullptr && slot->stackSize == 0)
		{
			delete slot;
			slot = nullptr;
		}
	}

	clearItemInUse();
}

bool EntityPlayer::isMovementBlocked()
{
	return health <= 0 || isPlayerSleeping();
}

void EntityPlayer::closeScreen()
{
	craftingInventory = inventorySlots;
}

void EntityPlayer::updateCloak()
{
	playerCloakUrl = "http://s3.amazonaws.com/MinecraftCloaks/" + username + ".png";
	cloakUrl = playerCloakUrl;
}

void EntityPlayer::updateRidden()
{
	double d  = posX;
	double d1 = posY;
	double d2 = posZ;
	EntityLiving::updateRidden();
	field_775_e = field_774_f;
	field_774_f = 0.0f;
	addMountedMovementStat(posX - d, posY - d1, posZ - d2);
}

void EntityPlayer::preparePlayerToSpawn()
{
	yOffset = 1.62f;
	setSize(0.6f, 1.8f);
	EntityLiving::preparePlayerToSpawn();
	setEntityHealth(getMaxHealth());
	deathTime = 0;
}

int_t EntityPlayer::getSwingSpeedModifier()
{
	if (isPotionActive(Potion::digSpeed))
	{
		PotionEffect *effect = getActivePotionEffect(Potion::digSpeed);
		return effect != nullptr ? 6 - (effect->getAmplifier() + 1) : 6;
	}
	if (isPotionActive(Potion::digSlowdown))
	{
		PotionEffect *effect = getActivePotionEffect(Potion::digSlowdown);
		return effect != nullptr ? 6 + (effect->getAmplifier() + 1) * 2 : 6;
	}
	return 6;
}

void EntityPlayer::updateEntityActionState()
{
	const int_t swingSpeed = getSwingSpeedModifier();
	if (isSwinging)
	{
		++swingProgressInt;
		if (swingProgressInt >= swingSpeed)
		{
			swingProgressInt = 0;
			isSwinging = false;
		}
	}
	else
	{
		swingProgressInt = 0;
	}
	swingProgress = (float)swingProgressInt / (float)swingSpeed;
}

void EntityPlayer::updatePlayerActionState()
{
	updateEntityActionState();
}

void EntityPlayer::onLivingUpdate()
{
	if (flyToggleTimer > 0)
		--flyToggleTimer;

	if (worldObj->difficultySetting == 0 && health < getMaxHealth() && (ticksExisted % 20) * 12 == 0)
	{
		heal(1);
	}
	inventory->decrementAnimations();
	field_775_e = field_774_f;
	EntityLiving::onLivingUpdate();
	landMovementFactor = speedOnGround;
	jumpMovementFactor = speedInAir;
	if (isSprinting())
	{
		landMovementFactor = (float)((double)landMovementFactor + (double)speedOnGround * 0.3);
		jumpMovementFactor = (float)((double)jumpMovementFactor + (double)speedInAir * 0.3);
	}
#if PLATFORM_FLOAT_ENTITY_CORE_MATH
	const float motionXf = static_cast<float>(motionX);
	const float motionYf = static_cast<float>(motionY);
	const float motionZf = static_cast<float>(motionZ);
	float f = MathHelper::sqrt_float(motionXf * motionXf + motionZf * motionZf);
	float f1 = std::atan(-motionYf * 0.2f) * 15.0f;
#else
	float f = MathHelper::sqrt_double(motionX * motionX + motionZ * motionZ);
	float f1 = (float)std::atan(-motionY * 0.20000000298023224) * 15.0f;
#endif
	if (f > 0.1f)
	{
		f = 0.1f;
	}
	if (!onGround || health <= 0)
	{
		f = 0.0f;
	}
	if (onGround || health <= 0)
	{
		f1 = 0.0f;
	}
	field_774_f += (f - field_774_f) * 0.4f;
	field_9328_R += (f1 - field_9328_R) * 0.8f;
	if (health > 0)
	{
		const std::vector<Entity *> list = worldObj->getEntitiesWithinAABBExcludingEntity(this, boundingBox->expand(1.0, 0.0, 1.0));
		for (size_t i = 0; i < list.size(); i++)
		{
			Entity *entity = list[i];
			if (!entity->isDead)
			{
				collideWithPlayer(entity);
			}
		}
	}
}

void EntityPlayer::collideWithPlayer(Entity *entity)
{
	entity->onCollideWithPlayer(this);
}

int_t EntityPlayer::getScore()
{
	return score;
}

void EntityPlayer::onDeath(Entity *entity)
{
	EntityLiving::onDeath(entity);
	setSize(0.2f, 0.2f);
	setPosition(posX, posY, posZ);
	motionY = 0.10000000149011612;
	if (username == "Notch")
	{
		dropPlayerItemWithRandomChoice(new ItemStack(Item::appleRed, 1), true);
	}
	inventory->dropAllItems();
	if (entity != nullptr)
	{
		motionX = -MathHelper::cos(((attackedAtYaw + rotationYaw) * 3.1415927f) / 180.0f) * 0.1f;
		motionZ = -MathHelper::sin(((attackedAtYaw + rotationYaw) * 3.1415927f) / 180.0f) * 0.1f;
	}
	else
	{
		motionX = motionZ = 0.0;
	}
	yOffset = 0.1f;
	addStat(StatList::deathsStat, 1);
}

void EntityPlayer::onDeath(const DamageSource &source)
{
	EntityLiving::onDeath(source);
	setSize(0.2f, 0.2f);
	setPosition(posX, posY, posZ);
	motionY = 0.10000000149011612;
	if (username == "Notch")
		dropPlayerItemWithRandomChoice(new ItemStack(Item::appleRed, 1), true);
	inventory->dropAllItems();
	motionX = -MathHelper::cos(((attackedAtYaw + rotationYaw) * 3.1415927f) / 180.0f) * 0.1f;
	motionZ = -MathHelper::sin(((attackedAtYaw + rotationYaw) * 3.1415927f) / 180.0f) * 0.1f;
	yOffset = 0.1f;
	addStat(StatList::deathsStat, 1);
}

void EntityPlayer::addToPlayerScore(Entity *entity, int_t i)
{
	score += i;
	if (entity == nullptr)
	{
		return;
	}
	if (entity->isPlayer())
	{
		addStat(StatList::playerKillsStat, 1);
	}
	else
	{
		addStat(StatList::mobKillsStat, 1);
	}
}

void EntityPlayer::dropCurrentItem()
{
	(void)dropOneItem();
}

EntityItem *EntityPlayer::dropOneItem()
{
	return dropPlayerItemWithRandomChoice(inventory->decrStackSize(inventory->currentItem, 1), false);
}

EntityItem *EntityPlayer::dropPlayerItem(ItemStack *itemstack)
{
	return dropPlayerItemWithRandomChoice(itemstack, false);
}

EntityItem *EntityPlayer::dropPlayerItemWithRandomChoice(ItemStack *itemstack, bool flag)
{
	if (itemstack == nullptr)
		return nullptr;
	EntityItem *entityitem = new EntityItem(worldObj, posX, (posY - 0.30000001192092896) + (double)getEyeHeight(), posZ, itemstack);
	entityitem->delayBeforeCanPickup = 40;
	float f = 0.1f;
	if (flag)
	{
		float f2 = rand.nextFloat() * 0.5f;
		float f4 = rand.nextFloat() * 3.1415927f * 2.0f;
		entityitem->motionX = -MathHelper::sin(f4) * f2;
		entityitem->motionZ =  MathHelper::cos(f4) * f2;
		entityitem->motionY = 0.20000000298023224;
	}
	else
	{
		float f1 = 0.3f;
		entityitem->motionX = -MathHelper::sin((rotationYaw / 180.0f) * 3.1415927f) * MathHelper::cos((rotationPitch / 180.0f) * 3.1415927f) * f1;
		entityitem->motionZ =  MathHelper::cos((rotationYaw / 180.0f) * 3.1415927f) * MathHelper::cos((rotationPitch / 180.0f) * 3.1415927f) * f1;
		entityitem->motionY = -MathHelper::sin((rotationPitch / 180.0f) * 3.1415927f) * f1 + 0.1f;
		f1 = 0.02f;
		float f3 = rand.nextFloat() * 3.1415927f * 2.0f;
		f1 *= rand.nextFloat();
		entityitem->motionX += JavaMath::cos(f3) * (double)f1;
		entityitem->motionY += (double)(rand.nextFloatDifference() * 0.1f);
		entityitem->motionZ += JavaMath::sin(f3) * (double)f1;
	}
	joinEntityItemWithWorld(entityitem);
	addStat(StatList::dropStat, 1);
	return entityitem;
}

void EntityPlayer::joinEntityItemWithWorld(EntityItem *entityitem)
{
	worldObj->entityJoinedWorld(entityitem);
}

float EntityPlayer::getCurrentPlayerStrVsBlock(Block *block)
{
	Potion::initPotions();
	float baseStrength = inventory->getStrVsBlock(block);
	float strength = baseStrength;

	const int_t efficiency = EnchantmentHelper::getEfficiencyModifier(inventory);
	if (efficiency > 0 && inventory->canHarvestBlock(block))
		strength = baseStrength + (float)(efficiency * efficiency + 1);

	if (isPotionActive(Potion::digSpeed))
	{
		PotionEffect *effect = getActivePotionEffect(Potion::digSpeed);
		if (effect != nullptr)
			strength *= 1.0f + (float)(effect->getAmplifier() + 1) * 0.2f;
	}
	if (isPotionActive(Potion::digSlowdown))
	{
		PotionEffect *effect = getActivePotionEffect(Potion::digSlowdown);
		if (effect != nullptr)
			strength *= 1.0f - (float)(effect->getAmplifier() + 1) * 0.2f;
	}
	if (isInsideOfMaterial(Material::water) && !EnchantmentHelper::getAquaAffinityModifier(inventory))
		strength /= 5.0f;
	if (!onGround)
		strength /= 5.0f;
	return strength;
}

bool EntityPlayer::canHarvestBlock(Block *block)
{
	return inventory->canHarvestBlock(block);
}

void EntityPlayer::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	EntityLiving::readEntityFromNBT(nbttagcompound);
	NBTTagList *nbttaglist = nbttagcompound->getTagList("Inventory");
	inventory->readFromNBT(nbttaglist);
	dimension = nbttagcompound->getInteger("Dimension");
	sleeping = nbttagcompound->getBoolean("Sleeping");
	sleepTimer = nbttagcompound->getShort("SleepTimer");
	if (sleeping)
	{
		delete bedChunkCoordinates;
		bedChunkCoordinates = new ChunkCoordinates(MathHelper::floor_double(posX), MathHelper::floor_double(posY), MathHelper::floor_double(posZ));
		wakeUpPlayer(true, true, false);
	}
	if (nbttagcompound->hasKey("SpawnX") && nbttagcompound->hasKey("SpawnY") && nbttagcompound->hasKey("SpawnZ"))
	{
		delete playerSpawnCoordinate;
		playerSpawnCoordinate = new ChunkCoordinates(nbttagcompound->getInteger("SpawnX"), nbttagcompound->getInteger("SpawnY"), nbttagcompound->getInteger("SpawnZ"));
	}
	experience = nbttagcompound->getFloat("XpP");
	experienceLevel = nbttagcompound->getInteger("XpLevel");
	experienceTotal = nbttagcompound->getInteger("XpTotal");
	foodStats.readNBT(nbttagcompound);
	capabilities.readCapabilitiesFromNBT(nbttagcompound);
}

void EntityPlayer::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	EntityLiving::writeEntityToNBT(nbttagcompound);
	nbttagcompound->setTag("Inventory", inventory->writeToNBT(new NBTTagList()));
	nbttagcompound->setInteger("Dimension", dimension);
	nbttagcompound->setBoolean("Sleeping", sleeping);
	nbttagcompound->setShort("SleepTimer", JavaArithmetic::shortFromBits(static_cast<ushort_t>(sleepTimer)));
	if (playerSpawnCoordinate != nullptr)
	{
		nbttagcompound->setInteger("SpawnX", playerSpawnCoordinate->x);
		nbttagcompound->setInteger("SpawnY", playerSpawnCoordinate->y);
		nbttagcompound->setInteger("SpawnZ", playerSpawnCoordinate->z);
	}
	nbttagcompound->setFloat("XpP", experience);
	nbttagcompound->setInteger("XpLevel", experienceLevel);
	nbttagcompound->setInteger("XpTotal", experienceTotal);
	foodStats.writeNBT(nbttagcompound);
	capabilities.writeCapabilitiesToNBT(nbttagcompound);
}

void EntityPlayer::displayGUIChest(IInventory *iinventory)
{
}

void EntityPlayer::displayWorkbenchGUI(int_t i, int_t j, int_t k)
{
}

void EntityPlayer::displayGUIEnchantment(int_t i, int_t j, int_t k)
{
}

void EntityPlayer::onItemPickup(Entity *entity, int_t i)
{
}

float EntityPlayer::getEyeHeight()
{
	return 0.12f;
}

void EntityPlayer::resetHeight()
{
	yOffset = 1.62f;
}

bool EntityPlayer::attackEntityFrom(Entity *entity, int_t damage)
{
	if (EntityArrow *arrow = dynamic_cast<EntityArrow *>(entity))
	{
		DamageSource source = DamageSource::causeArrowDamage(arrow, arrow->getShootingEntity());
		return attackEntityFrom(source, damage);
	}
	if (EntityPlayer *player = dynamic_cast<EntityPlayer *>(entity))
	{
		DamageSource source = DamageSource::causePlayerDamage(player);
		return attackEntityFrom(source, damage);
	}
	if (EntityLiving *living = dynamic_cast<EntityLiving *>(entity))
	{
		DamageSource source = DamageSource::causeMobDamage(living);
		return attackEntityFrom(source, damage);
	}
	return EntityLiving::attackEntityFrom(entity, damage);
}

bool EntityPlayer::attackEntityFrom(const DamageSource &source, int_t damage)
{
	if (capabilities.disableDamage && !source.canHarmInCreative())
		return false;
	entityAge = 0;
	if (getHealth() <= 0)
		return false;
	if (isPlayerSleeping() && !worldObj->multiplayerWorld)
		wakeUpPlayer(true, true, false);

	Entity *attacker = source.getEntity();
	if (dynamic_cast<EntityMob *>(attacker) != nullptr || dynamic_cast<EntityArrow *>(attacker) != nullptr || dynamic_cast<EntityArrow *>(source.getSourceOfDamage()) != nullptr)
	{
		if (worldObj->difficultySetting == 0) damage = 0;
		if (worldObj->difficultySetting == 1) damage = damage / 2 + 1;
		if (worldObj->difficultySetting == 3) damage = damage * 3 / 2;
	}
	if (damage == 0)
		return false;

	Entity *responsible = attacker;
	if (EntityArrow *arrow = dynamic_cast<EntityArrow *>(source.getSourceOfDamage()))
	{
		if (arrow->getShootingEntity() != nullptr)
			responsible = arrow->getShootingEntity();
	}
	if (EntityLiving *living = dynamic_cast<EntityLiving *>(responsible))
		alertWolves(living, false);

	addStat(StatList::damageTakenStat, damage);
	return EntityLiving::attackEntityFrom(source, damage);
}

int_t EntityPlayer::decreaseAirSupply(int_t airSupply)
{
	const int_t respiration = EnchantmentHelper::getRespiration(inventory);
	return respiration > 0 && rand.nextInt(respiration + 1) > 0 ? airSupply : EntityLiving::decreaseAirSupply(airSupply);
}

bool EntityPlayer::isPVPEnabled()
{
	return false;
}

bool EntityPlayer::shouldWolvesAttackPlayers()
{
	return isPVPEnabled();
}

void EntityPlayer::alertWolves(EntityLiving *entityliving, bool flag)
{
	if (dynamic_cast<EntityCreeper*>(entityliving) != nullptr || dynamic_cast<EntityGhast*>(entityliving) != nullptr)
	{
		return;
	}
	EntityWolf *wolfCheck = dynamic_cast<EntityWolf*>(entityliving);
	if (wolfCheck != nullptr)
	{
		if (wolfCheck->isWolfTamed() && username == wolfCheck->getWolfOwner())
		{
			return;
		}
	}
	if (entityliving->isPlayer() && !shouldWolvesAttackPlayers())
	{
		return;
	}
	const auto& list = worldObj->getEntitiesWithinAABB(typeid(EntityWolf), AxisAlignedBB::getBoundingBoxFromPool(posX, posY, posZ, posX + 1.0, posY + 1.0, posZ + 1.0)->expand(16.0, 4.0, 16.0));
	for (size_t i = 0; i < list.size(); i++)
	{
		Entity *entity = list[i];
		EntityWolf *entitywolf1 = static_cast<EntityWolf*>(entity);
		if (entitywolf1->isWolfTamed() && entitywolf1->getAttackTarget() == nullptr && username == entitywolf1->getWolfOwner() && (!flag || !entitywolf1->isWolfSitting()))
		{
			entitywolf1->setWolfSitting(false);
			entitywolf1->setAttackTarget(entityliving);
		}
	}
}

void EntityPlayer::damageArmor(int_t damage)
{
	inventory->damageArmor(damage);
}

int_t EntityPlayer::getTotalArmorValue() const
{
	return inventory != nullptr ? inventory->getTotalArmorValue() : 0;
}


int_t EntityPlayer::applyPotionDamageCalculations(const DamageSource &source, int_t damage)
{
	damage = EntityLiving::applyPotionDamageCalculations(source, damage);
	if (damage <= 0)
		return 0;
	int_t enchantmentModifier = EnchantmentHelper::getEnchantmentModifierDamage(inventory, source);
	if (enchantmentModifier > 20)
		enchantmentModifier = 20;
	if (enchantmentModifier > 0)
	{
		const int_t armorFactor = 25 - enchantmentModifier;
		const int_t scaledDamage = damage * armorFactor + carryoverDamage;
		damage = scaledDamage / 25;
		carryoverDamage = scaledDamage % 25;
	}
	return damage;
}

void EntityPlayer::damageEntity(int_t damage)
{
	int_t armorFactor = 25 - inventory->getTotalArmorValue();
	int_t scaledDamage = damage * armorFactor + damageRemainder;
	inventory->damageArmor(damage);
	damage = scaledDamage / 25;
	damageRemainder = scaledDamage % 25;
	EntityLiving::damageEntity(damage);
}

void EntityPlayer::damageEntity(const DamageSource &source, int_t damage)
{
	if (!source.isUnblockable() && isBlocking())
		damage = (1 + damage) >> 1;
	damage = applyArmorCalculations(source, damage);
	damage = applyPotionDamageCalculations(source, damage);
	addExhaustion(source.getHungerDamage());
	health -= damage;
}

void EntityPlayer::displayGUIFurnace(TileEntityFurnace *tileentityfurnace) {}
void EntityPlayer::displayGUIBrewingStand(TileEntityBrewingStand *tileentitybrewingstand) {}
void EntityPlayer::displayGUIDispenser(TileEntityDispenser *tileentitydispenser) {}
void EntityPlayer::displayGUIEditSign(TileEntitySign *tileentitysign) {}

void EntityPlayer::useCurrentItemOnEntity(Entity *entity)
{
	if (entity == nullptr)
	{
		return;
	}
	if (entity->interact(this))
	{
		return;
	}
	ItemStack *itemstack = getCurrentEquippedItem();
	EntityLiving *living = entity->isLiving() ? static_cast<EntityLiving*>(entity) : nullptr;
	if (itemstack != nullptr && living != nullptr)
	{
		itemstack->useItemOnEntity(living);
		if (itemstack->stackSize <= 0)
		{
			itemstack->onItemDestroyedByUse(this);
			destroyCurrentEquippedItem();
		}
	}
}

ItemStack *EntityPlayer::getCurrentEquippedItem()
{
	return inventory->getCurrentItem();
}

void EntityPlayer::destroyCurrentEquippedItem()
{
	inventory->setInventorySlotContents(inventory->currentItem, nullptr);
}

double EntityPlayer::getYOffset()
{
	return (double)(yOffset - 0.5f);
}

void EntityPlayer::swingItem()
{
	if (!isSwinging || swingProgressInt >= getSwingSpeedModifier() / 2 || swingProgressInt < 0)
	{
		swingProgressInt = -1;
		isSwinging = true;
	}
}

void EntityPlayer::attackTargetEntityWithCurrentItem(Entity *entity)
{
	if (entity == nullptr || !entity->canAttackWithItem())
		return;

	Potion::initPotions();
	int_t damage = inventory->getDamageVsEntity(entity);
	if (isPotionActive(Potion::damageBoost))
	{
		PotionEffect *effect = getActivePotionEffect(Potion::damageBoost);
		if (effect != nullptr)
			damage = JavaArithmetic::intAdd(damage, JavaArithmetic::intShl(3, effect->getAmplifier()));
	}
	if (isPotionActive(Potion::weakness))
	{
		PotionEffect *effect = getActivePotionEffect(Potion::weakness);
		if (effect != nullptr)
			damage = JavaArithmetic::intSub(damage, JavaArithmetic::intShl(2, effect->getAmplifier()));
	}

	EntityLiving *living = dynamic_cast<EntityLiving *>(entity);
	int_t knockback = 0;
	int_t enchantmentDamage = 0;
	if (living != nullptr)
	{
		enchantmentDamage = EnchantmentHelper::getEnchantmentModifierLiving(inventory, living);
		knockback = EnchantmentHelper::getKnockbackModifier(inventory, living);
	}
	if (isSprinting())
		knockback = JavaArithmetic::intAdd(knockback, 1);

	if (damage <= 0 && enchantmentDamage <= 0)
		return;

	bool critical = fallDistance > 0.0f && !onGround && !isOnLadder() && !isInWater() &&
		!isPotionActive(Potion::blindness) && ridingEntity == nullptr && living != nullptr;
	if (critical)
	{
		const int_t criticalBound = JavaArithmetic::intAdd(JavaArithmetic::intDiv(damage, 2), 2);
		damage = JavaArithmetic::intAdd(damage, rand.nextInt(criticalBound));
	}
	damage = JavaArithmetic::intAdd(damage, enchantmentDamage);

	bool hit = entity->attackEntityFrom(DamageSource::causePlayerDamage(this), damage);
	if (hit)
	{
		if (knockback > 0)
		{
			entity->addVelocity((double)(-MathHelper::sin(rotationYaw * 3.1415927f / 180.0f) * (float)knockback * 0.5f),
				0.1, (double)(MathHelper::cos(rotationYaw * 3.1415927f / 180.0f) * (float)knockback * 0.5f));
			motionX *= 0.6;
			motionZ *= 0.6;
			setSprinting(false);
		}
		if (critical)
			onCriticalHit(entity);
		if (enchantmentDamage > 0)
			onEnchantmentCritical(entity);
		if (damage >= 18)
			triggerAchievement(AchievementList::overkill);
		setLastAttackingEntity(entity);
	}

	ItemStack *itemstack = getCurrentEquippedItem();
	if (itemstack != nullptr && living != nullptr)
	{
		itemstack->hitEntity(living, this);
		if (itemstack->stackSize <= 0)
		{
			itemstack->onItemDestroyedByUse(this);
			destroyCurrentEquippedItem();
		}
	}

	if (living != nullptr)
	{
		if (entity->isEntityAlive())
			alertWolves(living, true);
		addStat(StatList::damageDealtStat, damage);
		int_t fireAspect = EnchantmentHelper::getFireAspectModifier(inventory, living);
		if (fireAspect > 0)
			entity->setFire(JavaArithmetic::intMul(fireAspect, 4));
	}
	addExhaustion(0.3f);
}

void EntityPlayer::onCriticalHit(Entity *)
{
}

void EntityPlayer::onEnchantmentCritical(Entity *)
{
}

void EntityPlayer::respawnPlayer()
{
}

void EntityPlayer::onItemStackChanged(ItemStack *itemstack)
{
}

void EntityPlayer::setEntityDead()
{
	EntityLiving::setEntityDead();
	if (inventorySlots != nullptr)
		inventorySlots->onCraftGuiClosed(this);
	if (craftingInventory != nullptr && craftingInventory != inventorySlots)
		craftingInventory->onCraftGuiClosed(this);
}

void EntityPlayer::setDead()
{
	setEntityDead();
}

bool EntityPlayer::isEntityInsideOpaqueBlock()
{
	return !sleeping && Entity::isEntityInsideOpaqueBlock();
}

EnumStatus EntityPlayer::sleepInBedAt(int_t i, int_t j, int_t k)
{
	if (!worldObj->multiplayerWorld)
	{
		if (isPlayerSleeping() || !isEntityAlive())
		{
			return EnumStatus::OTHER_PROBLEM;
		}
		if (worldObj->worldProvider == nullptr || !worldObj->worldProvider->func_48217_e())
		{
			return EnumStatus::NOT_POSSIBLE_HERE;
		}
		if (worldObj->isDaytime())
		{
			return EnumStatus::NOT_POSSIBLE_NOW;
		}
		if (std::abs(posX - (double)i) > 3.0 || std::abs(posY - (double)j) > 2.0 || std::abs(posZ - (double)k) > 3.0)
		{
			return EnumStatus::TOO_FAR_AWAY;
		}

		AxisAlignedBB *dangerArea = AxisAlignedBB::getBoundingBoxFromPool(
			(double)i - 8.0, (double)j - 5.0, (double)k - 8.0,
			(double)i + 8.0, (double)j + 5.0, (double)k + 8.0);
		const std::vector<Entity *> &hostiles = worldObj->getEntitiesWithinAABB(typeid(EntityMob), dangerArea);
		if (!hostiles.empty())
			return EnumStatus::NOT_SAFE;
	}
	setSize(0.2f, 0.2f);
	yOffset = 0.2f;
	if (worldObj->blockExists(i, j, k))
	{
		int_t l = worldObj->getBlockMetadata(i, j, k);
		int_t i1 = BlockBed::getDirectionFromMetadata(l);
		float f  = 0.5f;
		float f1 = 0.5f;
		switch (i1)
		{
			case 0: f1 = 0.9f; break;
			case 2: f1 = 0.1f; break;
			case 1: f  = 0.1f; break;
			case 3: f  = 0.9f; break;
		}
		setRenderOffsetForSleep(i1);
		setPosition((double)((float)i + f), (double)((float)j + 0.9375f), (double)((float)k + f1));
	}
	else
	{
		setPosition((double)((float)i + 0.5f), (double)((float)j + 0.9375f), (double)((float)k + 0.5f));
	}
	sleeping = true;
	sleepTimer = 0;
	delete bedChunkCoordinates;
	bedChunkCoordinates = new ChunkCoordinates(i, j, k);
	motionX = motionZ = motionY = 0.0;
	if (!worldObj->multiplayerWorld)
	{
		worldObj->updateAllPlayersSleepingFlag();
	}
	return EnumStatus::OK;
}

void EntityPlayer::setRenderOffsetForSleep(int_t i)
{
	field_22063_x = 0.0f;
	field_22061_z = 0.0f;
	switch (i)
	{
		case 0: field_22061_z = -1.8f; break;
		case 2: field_22061_z =  1.8f; break;
		case 1: field_22063_x =  1.8f; break;
		case 3: field_22063_x = -1.8f; break;
	}
}

void EntityPlayer::wakeUpPlayer(bool flag, bool flag1, bool flag2)
{
	setSize(0.6f, 1.8f);
	resetHeight();
	ChunkCoordinates *chunkcoordinates = bedChunkCoordinates;
	if (chunkcoordinates != nullptr && worldObj->getBlockId(chunkcoordinates->x, chunkcoordinates->y, chunkcoordinates->z) == Block::blockBed->blockID)
	{
		BlockBed::setBedOccupied(worldObj, chunkcoordinates->x, chunkcoordinates->y, chunkcoordinates->z, false);
		ChunkCoordinates *chunkcoordinates2 = BlockBed::getNearestEmptyChunkCoordinates(worldObj, chunkcoordinates->x, chunkcoordinates->y, chunkcoordinates->z, 0);
		if (chunkcoordinates2 == nullptr)
		{
			chunkcoordinates2 = new ChunkCoordinates(chunkcoordinates->x, chunkcoordinates->y + 1, chunkcoordinates->z);
		}
		setPosition((double)((float)chunkcoordinates2->x + 0.5f), (double)((float)chunkcoordinates2->y + yOffset + 0.1f), (double)((float)chunkcoordinates2->z + 0.5f));
		delete chunkcoordinates2;
	}
	sleeping = false;
	if (!worldObj->multiplayerWorld && flag1)
	{
		worldObj->updateAllPlayersSleepingFlag();
	}
	sleepTimer = flag ? 0 : 100;
	if (flag2)
	{
		setPlayerSpawnCoordinate(bedChunkCoordinates);
	}
}

bool EntityPlayer::isInBed()
{
	return worldObj->getBlockId(bedChunkCoordinates->x, bedChunkCoordinates->y, bedChunkCoordinates->z) == Block::blockBed->blockID;
}

ChunkCoordinates *EntityPlayer::getNearestBedSpawnLocation(World *world, ChunkCoordinates *chunkcoordinates)
{
	IChunkProvider *ichunkprovider = world->getIChunkProvider();
	const int_t minX = JavaArithmetic::intSub(chunkcoordinates->x, 3);
	const int_t maxX = JavaArithmetic::intAdd(chunkcoordinates->x, 3);
	const int_t minZ = JavaArithmetic::intSub(chunkcoordinates->z, 3);
	const int_t maxZ = JavaArithmetic::intAdd(chunkcoordinates->z, 3);
	ichunkprovider->loadChunk(JavaArithmetic::intShr(minX, 4), JavaArithmetic::intShr(minZ, 4));
	ichunkprovider->loadChunk(JavaArithmetic::intShr(maxX, 4), JavaArithmetic::intShr(minZ, 4));
	ichunkprovider->loadChunk(JavaArithmetic::intShr(minX, 4), JavaArithmetic::intShr(maxZ, 4));
	ichunkprovider->loadChunk(JavaArithmetic::intShr(maxX, 4), JavaArithmetic::intShr(maxZ, 4));
	if (world->getBlockId(chunkcoordinates->x, chunkcoordinates->y, chunkcoordinates->z) != Block::blockBed->blockID)
	{
		return nullptr;
	}
	return BlockBed::getNearestEmptyChunkCoordinates(world, chunkcoordinates->x, chunkcoordinates->y, chunkcoordinates->z, 0);
}

ChunkCoordinates *EntityPlayer::verifyRespawnCoordinates(World *world, ChunkCoordinates *chunkcoordinates)
{
	return getNearestBedSpawnLocation(world, chunkcoordinates);
}

float EntityPlayer::getBedOrientationInDegrees()
{
	if (bedChunkCoordinates != nullptr)
	{
		int_t i = worldObj->getBlockMetadata(bedChunkCoordinates->x, bedChunkCoordinates->y, bedChunkCoordinates->z);
		int_t j = BlockBed::getDirectionFromMetadata(i);
		switch (j)
		{
			case 0: return 90.0f;
			case 1: return 0.0f;
			case 2: return 270.0f;
			case 3: return 180.0f;
		}
	}
	return 0.0f;
}

bool EntityPlayer::isPlayerSleeping()
{
	return sleeping;
}

bool EntityPlayer::isPlayerFullyAsleep()
{
	return sleeping && sleepTimer >= 100;
}

int_t EntityPlayer::getSleepTimer()
{
	return sleepTimer;
}

void EntityPlayer::addChatMessage(const std::string &s)
{
}

ChunkCoordinates *EntityPlayer::getPlayerSpawnCoordinate()
{
	return playerSpawnCoordinate;
}

void EntityPlayer::setPlayerSpawnCoordinate(ChunkCoordinates *chunkcoordinates)
{
	delete playerSpawnCoordinate;
	playerSpawnCoordinate = nullptr;
	if (chunkcoordinates != nullptr)
	{
		playerSpawnCoordinate = new ChunkCoordinates(chunkcoordinates);
	}
}

ChunkCoordinates *EntityPlayer::getSpawnChunk()
{
	return getPlayerSpawnCoordinate();
}

void EntityPlayer::setSpawnChunk(ChunkCoordinates *chunkcoordinates)
{
	setPlayerSpawnCoordinate(chunkcoordinates);
}

void EntityPlayer::triggerAchievement(StatBase *statbase)
{
	addStat(statbase, 1);
}

void EntityPlayer::addStat(StatBase *statbase, int_t i)
{
}

void EntityPlayer::jump()
{
	EntityLiving::jump();
	addStat(StatList::jumpStat, 1);
	addExhaustion(isSprinting() ? 0.8f : 0.2f);
}

void EntityPlayer::moveEntityWithHeading(float f, float f1)
{
	double d  = posX;
	double d1 = posY;
	double d2 = posZ;
	if (capabilities.isFlying)
	{
		const double previousMotionY = motionY;
		const float previousJumpMovementFactor = jumpMovementFactor;
		jumpMovementFactor = 0.05f;
		EntityLiving::moveEntityWithHeading(f, f1);
		motionY = previousMotionY * 0.6;
		jumpMovementFactor = previousJumpMovementFactor;
	}
	else
	{
		EntityLiving::moveEntityWithHeading(f, f1);
	}
	addMovementStat(posX - d, posY - d1, posZ - d2);
}

void EntityPlayer::addMovementStat(double d, double d1, double d2)
{
	if (ridingEntity != nullptr)
	{
		return;
	}
	if (isInsideOfMaterial(Material::water))
	{
		int_t i = JavaArithmetic::roundFloat(MathHelper::sqrt_double(d * d + d1 * d1 + d2 * d2) * 100.0f);
		if (i > 0)
		{
			addStat(StatList::distanceDoveStat, i);
			addExhaustion(0.015f * (float)i * 0.01f);
		}
	}
	else if (isInWater())
	{
		int_t j = JavaArithmetic::roundFloat(MathHelper::sqrt_double(d * d + d2 * d2) * 100.0f);
		if (j > 0)
		{
			addStat(StatList::distanceSwumStat, j);
			addExhaustion(0.015f * (float)j * 0.01f);
		}
	}
	else if (isOnLadder())
	{
		if (d1 > 0.0)
		{
			addStat(StatList::distanceClimbedStat, JavaArithmetic::longToInt(JavaArithmetic::roundDouble(d1 * 100.0)));
		}
	}
	else if (onGround)
	{
		int_t k = JavaArithmetic::roundFloat(MathHelper::sqrt_double(d * d + d2 * d2) * 100.0f);
		if (k > 0)
		{
			addStat(StatList::distanceWalkedStat, k);
			if (isSprinting())
				addExhaustion(10.0f * 0.01f * (float)k * 0.01f);
			else
				addExhaustion(0.01f * (float)k * 0.01f);
		}
	}
	else
	{
		int_t l = JavaArithmetic::roundFloat(MathHelper::sqrt_double(d * d + d2 * d2) * 100.0f);
		if (l > 25)
		{
			addStat(StatList::distanceFlownStat, l);
		}
	}
}

void EntityPlayer::addMountedMovementStat(double d, double d1, double d2)
{
	if (ridingEntity != nullptr)
	{
		int_t i = JavaArithmetic::roundFloat(MathHelper::sqrt_double(d * d + d1 * d1 + d2 * d2) * 100.0f);
		if (i > 0)
		{
			if (dynamic_cast<EntityMinecart*>(ridingEntity) != nullptr)
			{
				addStat(StatList::distanceByMinecartStat, i);
				if (startMinecartRidingCoordinate == nullptr)
				{
					startMinecartRidingCoordinate = new ChunkCoordinates(MathHelper::floor_double(posX), MathHelper::floor_double(posY), MathHelper::floor_double(posZ));
				}
				else if (startMinecartRidingCoordinate->getEuclideanDistanceTo(MathHelper::floor_double(posX), MathHelper::floor_double(posY), MathHelper::floor_double(posZ)) >= 1000.0)
				{
					addStat(AchievementList::onARail, 1);
				}
			}
			else if (dynamic_cast<EntityBoat*>(ridingEntity) != nullptr)
			{
				addStat(StatList::distanceByBoatStat, i);
			}
			else if (dynamic_cast<EntityPig*>(ridingEntity) != nullptr)
			{
				addStat(StatList::distanceByPigStat, i);
			}
		}
	}
}

void EntityPlayer::fall(float f)
{
	if (f >= 2.0f)
	{
		addStat(StatList::distanceFallenStat, JavaArithmetic::longToInt(JavaArithmetic::roundDouble((double)f * 100.0)));
	}
	EntityLiving::fall(f);
}

int_t EntityPlayer::getExperiencePoints(EntityPlayer *player)
{
	(void)player;
	const int_t value = JavaArithmetic::intMul(experienceLevel, 7);
	return value > 100 ? 100 : value;
}

void EntityPlayer::onKillEntity(EntityLiving *entityliving)
{
	if (entityliving != nullptr && entityliving->isMob())
	{
		triggerAchievement(AchievementList::killEnemy);
	}
}

int_t EntityPlayer::getItemIcon(ItemStack *itemstack)
{
	return getItemIcon(itemstack, 0);
}

int_t EntityPlayer::getItemIcon(ItemStack *itemstack, int_t renderPass)
{
	int_t icon = EntityLiving::getItemIcon(itemstack, renderPass);
	if (itemstack->itemID == Item::fishingRod->shiftedIndex && fishEntity != nullptr)
	{
		return itemstack->getIconIndex() + 16;
	}

	Item *item = itemstack->getItem();
	if (item != nullptr && item->func_46058_c())
	{
		return item->func_46057_a(itemstack->getItemDamage(), renderPass);
	}

	if (itemInUse != nullptr && itemstack->itemID == Item::bow->shiftedIndex)
	{
		int_t useTicks = itemstack->getMaxItemUseDuration() - itemInUseCount;
		if (useTicks >= 18)
			return 133;
		if (useTicks > 13)
			return 117;
		if (useTicks > 0)
			return 101;
	}

	return icon;
}

void EntityPlayer::setInPortal()
{
	if (timeUntilPortal > 0)
	{
		timeUntilPortal = 10;
		return;
	}
	inPortal = true;
}

void EntityPlayer::addExperience(int_t amount)
{
	score = JavaArithmetic::intAdd(score, amount);
	const int_t remaining = JavaArithmetic::intSub(0x7fffffff, experienceTotal);
	if (amount > remaining)
		amount = remaining;

	experience += (float)amount / (float)xpBarCap();
	experienceTotal = JavaArithmetic::intAdd(experienceTotal, amount);
	while (experience >= 1.0f)
	{
		experience = (experience - 1.0f) * (float)xpBarCap();
		increaseLevel();
		experience /= (float)xpBarCap();
	}
}

void EntityPlayer::increaseLevel()
{
	experienceLevel = JavaArithmetic::intAdd(experienceLevel, 1);
}

void EntityPlayer::removeExperience(int_t amount)
{
	experienceLevel = JavaArithmetic::intSub(experienceLevel, amount);
	if (experienceLevel < 0)
		experienceLevel = 0;
}

int_t EntityPlayer::xpBarCap() const
{
	return JavaArithmetic::intAdd(7, JavaArithmetic::intShr(JavaArithmetic::intMul(experienceLevel, 7), 1));
}

void EntityPlayer::addExhaustion(float exhaustion)
{
	if (!capabilities.disableDamage && !worldObj->multiplayerWorld)
		foodStats.addExhaustion(exhaustion);
}

FoodStats *EntityPlayer::getFoodStats()
{
	return &foodStats;
}

bool EntityPlayer::canEat(bool alwaysEdible)
{
	return (alwaysEdible || foodStats.needFood()) && !capabilities.disableDamage;
}

bool EntityPlayer::shouldHeal() const
{
	return health > 0 && health < getMaxHealth();
}

void EntityPlayer::setItemInUse(ItemStack *itemstack, int_t duration)
{
	if (itemstack == itemInUse)
		return;
	itemInUse = itemstack;
	itemInUseCount = duration;
	if (!worldObj->multiplayerWorld)
		setEating(true);
}

bool EntityPlayer::canPlayerEdit(int_t, int_t, int_t) const
{
	return true;
}

void EntityPlayer::travelToTheEnd(int_t)
{
}

void EntityPlayer::copyPlayer(EntityPlayer *player)
{
	if (player == nullptr)
		return;
	inventory->copyInventory(player->inventory);
	health = player->health;
	foodStats = player->foodStats;
	experienceLevel = player->experienceLevel;
	experienceTotal = player->experienceTotal;
	experience = player->experience;
	score = player->score;
}

bool EntityPlayer::canTriggerWalking()
{
	return !capabilities.isFlying;
}

void EntityPlayer::func_50009_aI()
{
}
