#pragma once

#include "EntityLiving.h"
#include "java/Type.h"
#include "java/String.h"
#include "PlayerCapabilities.h"
#include "FoodStats.h"

class World;
class InventoryPlayer;
class Container;
class ChunkCoordinates;
class Entity;
class EntityItem;
class EntityFish;
class EntityLiving;
class ItemStack;
class Block;
class NBTTagCompound;
class IInventory;
class TileEntityFurnace;
class TileEntityBrewingStand;
class TileEntityDispenser;
class TileEntitySign;
class StatBase;
enum class EnumStatus;

// net.minecraft.src.EntityPlayer
class EntityPlayer : public EntityLiving
{
public:
	EntityPlayer(World *world);
	~EntityPlayer() override;

	bool isPlayer() const override { return true; }

protected:
	void entityInit() override;

public:
	void onUpdate() override;
	void handleHealthUpdate(byte_t state) override;
	ItemStack *getItemInUse();
	int_t getItemInUseCount() const;
	bool isUsingItem() const;
	int_t getItemInUseDuration();
	void stopUsingItem();
	void clearItemInUse();
	bool isBlocking() override;

protected:
	bool isMovementBlocked() override;
	void onItemUseFinish();

public:
	virtual void closeScreen();
	void updateCloak() override;
	void updateRidden() override;
	void preparePlayerToSpawn() override;

protected:
	void updatePlayerActionState() override;
	void updateEntityActionState();

public:
	void onLivingUpdate() override;

private:
	void collideWithPlayer(Entity *entity);

public:
	int_t getScore();
	void onDeath(Entity *entity) override;
	void onDeath(const DamageSource &source) override;
	void addToPlayerScore(Entity *entity, int_t i) override;
	virtual void dropCurrentItem();
	virtual EntityItem *dropOneItem();
	EntityItem *dropPlayerItem(ItemStack *itemstack);
	EntityItem *dropPlayerItemWithRandomChoice(ItemStack *itemstack, bool flag);

protected:
	virtual void joinEntityItemWithWorld(EntityItem *entityitem);

public:
	float getCurrentPlayerStrVsBlock(Block *block);
	bool canHarvestBlock(Block *block);

	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;

	virtual void displayGUIChest(IInventory *iinventory);
	virtual void displayWorkbenchGUI(int_t i, int_t j, int_t k);
	virtual void displayGUIEnchantment(int_t i, int_t j, int_t k);
	virtual void onItemPickup(Entity *entity, int_t i);

	float getEyeHeight() override;

protected:
	virtual void resetHeight();

public:
	bool attackEntityFrom(Entity *entity, int_t i) override;
	bool attackEntityFrom(const DamageSource &source, int_t damage) override;

protected:
	int_t decreaseAirSupply(int_t airSupply) override;
	virtual bool isPVPEnabled();
	virtual bool shouldWolvesAttackPlayers();
	virtual void alertWolves(EntityLiving *entityliving, bool flag);
	void damageArmor(int_t damage) override;
	int_t getTotalArmorValue() const override;
	int_t applyPotionDamageCalculations(const DamageSource &source, int_t damage) override;
	void damageEntity(int_t i) override;
	void damageEntity(const DamageSource &source, int_t damage) override;

public:
	virtual void displayGUIFurnace(TileEntityFurnace *tileentityfurnace);
	virtual void displayGUIBrewingStand(TileEntityBrewingStand *tileentitybrewingstand);
	virtual void displayGUIDispenser(TileEntityDispenser *tileentitydispenser);
	virtual void displayGUIEditSign(TileEntitySign *tileentitysign);
	void useCurrentItemOnEntity(Entity *entity);
	ItemStack *getCurrentEquippedItem();
	void destroyCurrentEquippedItem();
	double getYOffset() override;
	virtual void swingItem();
	void attackTargetEntityWithCurrentItem(Entity *entity);
	virtual void onCriticalHit(Entity *entity);
	virtual void onEnchantmentCritical(Entity *entity);
	virtual void respawnPlayer();
	virtual void func_6420_o();
	virtual void handleItemUseFinish();
	virtual void onItemStackChanged(ItemStack *itemstack);
	void setEntityDead() override;
	void setDead() override;
	bool isEntityInsideOpaqueBlock() override;
	EnumStatus sleepInBedAt(int_t i, int_t j, int_t k);

private:
	void setRenderOffsetForSleep(int_t i);

public:
	void wakeUpPlayer(bool flag, bool flag1, bool flag2);

private:
	bool isInBed();

public:
	static ChunkCoordinates *getNearestBedSpawnLocation(World *world, ChunkCoordinates *chunkcoordinates);
	static ChunkCoordinates *verifyRespawnCoordinates(World *world, ChunkCoordinates *chunkcoordinates);
	float getBedOrientationInDegrees();
	bool isPlayerSleeping() override;
	bool isPlayerFullyAsleep();
	int_t getSleepTimer();
	int_t getSleepTimer() const { return sleepTimer; }
	virtual void addChatMessage(const std::string &s);
	ChunkCoordinates *getPlayerSpawnCoordinate();
	void setPlayerSpawnCoordinate(ChunkCoordinates *chunkcoordinates);
	ChunkCoordinates *getSpawnChunk();
	void setSpawnChunk(ChunkCoordinates *chunkcoordinates);
	void triggerAchievement(StatBase *statbase);
	virtual void addStat(StatBase *statbase, int_t i);

protected:
	void jump() override;
	bool canTriggerWalking() override;

public:
	void moveEntityWithHeading(float f, float f1) override;

private:
	void addMovementStat(double d, double d1, double d2);
	void addMountedMovementStat(double d, double d1, double d2);

protected:
	void fall(float f) override;
	int_t getExperiencePoints(EntityPlayer *player) override;

public:
	void onKillEntity(EntityLiving *entityliving) override;
	int_t getItemIcon(ItemStack *itemstack) override;
	int_t getItemIcon(ItemStack *itemstack, int_t renderPass) override;
	void setInPortal() override;

	virtual int_t getPlayerArmorValue() { return 0; }
	int_t getHealth() const { return health; }
	virtual int_t getMaxHealth() const { return 20; }
	virtual void setHealth(int_t i) { health = i; }
	void addExperience(int_t amount);
	void removeExperience(int_t amount);
	void increaseLevel();
	int_t xpBarCap() const;
	void addExhaustion(float exhaustion);
	FoodStats *getFoodStats();
	bool canEat(bool alwaysEdible);
	bool shouldHeal() const;
	void setItemInUse(ItemStack *itemstack, int_t duration);
	bool canPlayerEdit(int_t x, int_t y, int_t z) const;
	virtual void travelToTheEnd(int_t dimension);
	virtual void copyPlayer(EntityPlayer *player);
	virtual void func_50009_aI();

	InventoryPlayer *inventory;
	Container *inventorySlots;
	Container *craftingInventory;
	byte_t  field_9371_f;
	int_t   score;
	float   field_775_e;
	float   field_774_f;
	bool    isSwinging;
	int_t   swingProgressInt;
	jstring username;
	int_t   dimension;
	jstring playerCloakUrl;
	double  field_20066_r;
	double  field_20065_s;
	double  field_20064_t;
	double  field_20063_u;
	double  field_20062_v;
	double  field_20061_w;
protected:
	bool    sleeping;
public:
	ChunkCoordinates *bedChunkCoordinates;
private:
	int_t   sleepTimer;
public:
	float   field_22063_x;
	float   field_22062_y;
	float   field_22061_z;
private:
	ChunkCoordinates *playerSpawnCoordinate;
	ChunkCoordinates *startMinecartRidingCoordinate;
public:
	int_t   timeUntilPortal;
protected:
	bool    inPortal;
public:
	float   timeInPortal;
	float   prevTimeInPortal;
private:
	int_t   damageRemainder;
protected:
	int_t   flyToggleTimer;
 	float   speedOnGround;
	float   speedInAir;
public:
	PlayerCapabilities capabilities;
	int_t xpCooldown;
	int_t experienceLevel;
	int_t experienceTotal;
	float experience;
	FoodStats foodStats;
	EntityFish *fishEntity;
private:
	ItemStack *itemInUse;
	int_t itemInUseCount;

protected:
	void updateItemUse(ItemStack *itemstack, int_t count);

private:
	int_t getSwingSpeedModifier();
};
