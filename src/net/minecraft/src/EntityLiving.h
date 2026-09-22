#pragma once

#include <unordered_map>
#include <vector>

#include "Entity.h"
#include "platform/PlatformConfig.h"
#include "java/Type.h"
#include "java/HashSet.h"
#include "java/String.h"
#include "EntityAITasks.h"
#include "ChunkCoordinates.h"
#include "EnumCreatureAttribute.h"

class World;
class Vec3D;
class NBTTagCompound;
class ItemStack;
class MovingObjectPosition;
class EntityPlayer;
class EntityLookHelper;
class EntityMoveHelper;
class EntityJumpHelper;
class EntityBodyHelper;
class EntitySenses;
class PathNavigate;
class PotionEffect;
class Potion;

// net.minecraft.src.EntityLiving
class EntityLiving : public Entity
{
public:
	EntityLiving(World *world);
	~EntityLiving() override;

	bool isLiving() const override { return true; }
	void func_48079_f(float yaw) override;

protected:
	void entityInit() override;

public:
	bool canEntityBeSeen(Entity *entity);
	const char *getEntityTexture() override;
	void setEntityTexture(const std::string &tex) { texture = tex; }
	bool canBeCollidedWith() override;
	bool canBePushed() override;
	float getEyeHeight() override;
	EntityLookHelper *getLookHelper();
	EntityMoveHelper *getMoveHelper();
	EntityJumpHelper *getJumpHelper();
	PathNavigate *getNavigator();
	EntitySenses *getEntitySenses();
	EntitySenses *func_48090_aM();
	// entity as EntityLiving, or nullptr when it is not one.
	static EntityLiving *asLiving(Entity *entity);
	EntityLiving *getAITarget();
	EntityLiving *getLastAttackingEntity();
	void setLastAttackingEntity(Entity *entity);
	EntityLiving *getAttackTarget();
	virtual void setAttackTarget(EntityLiving *target);
	virtual void setRevengeTarget(EntityLiving *target);
	int_t getRevengeTimer() const;
	Random &getRNG();
	int_t getAge() const;
	void setMoveForward(float value);
	void setJumping(bool value);
	void setAIMoveSpeed(float value);
	float getAIMoveSpeed() const;
	void func_48098_g(float value);
	float func_48101_aR() const;
	virtual bool isAIEnabled();
	void setPositionAndUpdate(double x, double y, double z);
	virtual bool isChild();
	virtual bool isBlocking();
	virtual float getRenderSizeModifier();
	virtual void renderBrokenItemStack(ItemStack *itemstack);
	std::vector<PotionEffect *> getActivePotionEffects() const;
	virtual void eatGrassBonus();
	virtual bool attackEntityAsMob(Entity *target);
	virtual bool canAttackEntity(EntityLiving *target);
	virtual bool func_48100_a(const std::type_info &type) const;
	int_t getHealth() const;
	virtual int_t getMaxHealth() const;
	virtual int_t getTotalArmorValue() const;
	virtual void setHealth(int_t value);
	void setEntityHealth(int_t value);
	virtual EnumCreatureAttribute getCreatureAttribute() const;
	bool isEntityUndead() const;
	bool isPotionActive(Potion *potion) const;
	PotionEffect *getActivePotionEffect(Potion *potion) const;
	void addPotionEffect(PotionEffect *effect);
	virtual bool isPotionApplicable(PotionEffect *effect) const;
	void removePotionEffect(int_t potionId);
	void clearActivePotions();
	bool isWithinHomeDistanceCurrentPosition() const;
	bool isWithinHomeDistance(int_t x, int_t y, int_t z) const;
	void setHomeArea(int_t x, int_t y, int_t z, int_t radius);
	ChunkCoordinates getHomePosition() const;
	float getMaximumHomeDistance() const;
	void detachHome();
	bool hasHome() const;
	virtual int_t getVerticalFaceSpeed();

	virtual int_t getTalkInterval();
	virtual void playLivingSound();

	void onEntityUpdate() override;

	void spawnExplosionParticle();
	void updateRidden() override;
	void setPositionAndRotation2(double d, double d1, double d2, float f, float f1, int_t i) override;
	void onUpdate() override;

protected:
	void setSize(float f, float f1) override;

public:
	virtual void heal(int_t i);
	bool attackEntityFrom(Entity *entity, int_t i) override;
	bool attackEntityFrom(const DamageSource &source, int_t damage) override;
	void performHurtAnimation() override;

protected:
	virtual void damageEntity(int_t i);
	virtual void damageEntity(const DamageSource &source, int_t damage);
	virtual void damageArmor(int_t damage);
	virtual int_t applyArmorCalculations(const DamageSource &source, int_t damage);
	virtual int_t applyPotionDamageCalculations(const DamageSource &source, int_t damage);
	virtual float getSoundVolume();
	virtual float getSoundPitch();
	virtual jstring getLivingSound();
	virtual jstring getHurtSound();
	virtual jstring getDeathSound();

public:
	virtual void knockBack(Entity *entity, int_t i, double d, double d1);
	virtual void onDeath(Entity *entity);
	virtual void onDeath(const DamageSource &source);

protected:
	virtual void dropFewItems();
	virtual void dropFewItems(bool recentlyHitByPlayer, int_t lootingLevel);
	virtual void dropRareDrop(int_t lootingRoll);
	virtual int_t getDropItemId();
	void fall(float f) override;

public:
	virtual void moveEntityWithHeading(float f, float f1);
	virtual bool isOnLadder();

	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;
	bool isEntityAlive() override;
	virtual bool canBreatheUnderwater();

protected:
	virtual int_t decreaseAirSupply(int_t airSupply);
	virtual int_t getExperiencePoints(EntityPlayer *player);

public:
	virtual void onLivingUpdate();

protected:
	virtual bool isMovementBlocked();
	virtual bool isClientWorld() const;
	virtual void jump();
	virtual bool canDespawn();
	virtual void despawnEntity();
	virtual void updateAITasks();
	virtual void updateAITick();
	virtual void onDeathUpdate();
	void updatePotionEffects();
	virtual void onNewPotionEffect(PotionEffect *effect);
	virtual void onChangedPotionEffect(PotionEffect *effect);
	virtual void onFinishedPotionEffect(PotionEffect *effect);
	virtual float getSpeedModifier();
	virtual void updatePlayerActionState();
	virtual void updateEntityActionState();
#if PLATFORM_CACHE_NEAREST_PLAYER
	EntityPlayer *getCachedNearestPlayer();
	float getCachedNearestPlayerDistanceSq();
#endif
#if PLATFORM_THROTTLE_ENTITY_AI
	bool shouldRunEntityDecisionAI();
#endif

public:
	void faceEntity(Entity *entity, float f, float f1);
	bool hasCurrentTarget();
	Entity *getCurrentTarget();

private:
	float updateRotation(float f, float f1, float f2);

public:
	virtual void onEntityDeath();
	virtual bool getCanSpawnHere();

protected:
	void kill() override;

public:
	float getSwingProgress(float f);
	Vec3D *getPosition(float f);
	Vec3D *getLookVec() override;
	Vec3D *getLook(float f);
	MovingObjectPosition *rayTrace(double d, float f);
	virtual int_t getMaxSpawnedInChunk();
	virtual ItemStack *getHeldItem();
	void handleHealthUpdate(byte_t byte0) override;
	virtual bool isPlayerSleeping();
	virtual int_t getItemIcon(ItemStack *itemstack);
	virtual int_t getItemIcon(ItemStack *itemstack, int_t renderPass);

public:
	int_t  heartsHalvesLife;
	float  renderYawOffset;
	float  prevRenderYawOffset;
	float  rotationYawHead;
	float  prevRotationYawHead;
protected:
	float  field_9362_u;
	float  field_9361_v;
	float  field_9360_w;
	float  field_9359_x;
	jstring texture;
	float  field_9353_B;
	jstring field_9351_C;
	int_t  scoreValue;
	int_t  experienceValue;
public:
	bool   isMultiplayerEntity;
	float  prevSwingProgress;
	float  swingProgress;
	int_t  health;
	int_t  prevHealth;
private:
	int_t  livingSoundTime;
public:
	int_t  hurtTime;
	int_t  maxHurtTime;
	float  attackedAtYaw;
	int_t  deathTime;
	int_t  attackTime;
	int_t  arrowHitTempCounter;
	int_t  arrowHitTimer;
	float  cameraPitch;
	float  field_9328_R;
protected:
public:
	float  field_705_Q;
	float  field_704_R;
	float  field_703_S;
protected:
	int_t  newPosRotationIncrements;
	double newPosX;
	double newPosY;
	double newPosZ;
	double newRotationYaw;
	double newRotationPitch;
	int_t  field_9346_af;
	int_t  entityAge;
public:
	float  moveStrafing;
	float  moveForward;
	float  landMovementFactor;
	float  jumpMovementFactor;
protected:
	float  randomYawVelocity;
public:
	bool   isJumping;
private:
	int_t  jumpTicks;
protected:
	float  defaultPitch;
	float  moveSpeed;
private:
	EntityLookHelper *lookHelper;
	EntityMoveHelper *moveHelper;
	EntityJumpHelper *jumpHelper;
	EntityBodyHelper *bodyHelper;
	PathNavigate *navigator;
	EntitySenses *entitySenses;
	float aiMoveSpeed;
	ChunkCoordinates homePosition;
	float maximumHomeDistance;
	int_t attackTargetEntityId;
	int_t lastAttackingEntityId;
	int_t revengeTargetEntityId;
	int_t revengeTimer;
	int_t attackingPlayerEntityId;
	int_t recentlyHit;

protected:
	int_t carryoverDamage;

private:
	struct PotionIdHash
	{
		std::uint32_t operator()(int_t value) const
		{
			// java.lang.Integer.hashCode() returns the primitive int value.
			return static_cast<std::uint32_t>(value);
		}
	};

	struct PotionIdEqual
	{
		bool operator()(int_t lhs, int_t rhs) const { return lhs == rhs; }
	};

	std::unordered_map<int_t, PotionEffect *> activePotionsMap;
	JavaHashSet<int_t, PotionIdHash, PotionIdEqual> activePotionOrder;
	bool potionsNeedUpdate;

protected:
	EntityAITasks tasks;
	EntityAITasks targetTasks;

private:
	Entity *currentTarget;
protected:
	int_t  numTicksToChaseTarget;
#if PLATFORM_CACHE_NEAREST_PLAYER
private:
	void refreshNearestPlayerCache();
	EntityPlayer *cachedNearestPlayer;
	int_t cachedNearestPlayerTick;
	float cachedNearestPlayerDistanceSq;
#endif
};
