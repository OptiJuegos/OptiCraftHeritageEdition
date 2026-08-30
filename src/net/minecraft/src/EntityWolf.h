#pragma once

#include "EntityTameable.h"

class World;
class NBTTagCompound;
class Entity;
class EntityPlayer;

// net.minecraft.src.EntityWolf
class EntityWolf : public EntityTameable
{
public:
	EntityWolf(World *world);

	static constexpr int_t CLASS_ID = 1;
	int_t getEntityClassID() const override { return CLASS_ID; }

	bool isAIEnabled() override;
	int_t getMaxHealth() const override;
	void setAttackTarget(EntityLiving *target) override;
	bool attackEntityAsMob(Entity *target) override;
	bool isWheat(ItemStack *itemstack) const override;
	EntityAnimal *spawnBabyAnimal(EntityAnimal *mate) override;
	bool canMateWith(EntityAnimal *mate) const override;
	void setLooksWithInterest(bool value);

protected:
	void entityInit() override;
	bool canTriggerWalking() override;
	void updateAITick() override;

public:
	const char *getEntityTexture() override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;

protected:
	bool canDespawn() override;
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	float getSoundVolume() override;
	int_t getDropItemId() override;
	void updatePlayerActionState() override;

public:
	void onLivingUpdate() override;
	void onUpdate() override;

	bool getWolfShaking();
	float getShadingWhileShaking(float f);
	float getShakeAngle(float f, float f1);
	float getInterestedAngle(float f);

	float getEyeHeight() override;
	int_t getVerticalFaceSpeed() override;

private:
	void getPathOrWalkableBlock(Entity *entity, float f);

protected:
	bool isMovementCeased() override;

public:
	bool attackEntityFrom(Entity *entity, int_t i) override;
	bool attackEntityFrom(const DamageSource &source, int_t damage) override;

protected:
	Entity *findPlayerToAttack() override;
	void attackEntity(Entity *entity, float f) override;

public:
	bool interact(EntityPlayer *entityplayer) override;

private:
	void showHeartsOrSmokeFX(bool flag);

public:
	void handleHealthUpdate(byte_t byte0) override;
	float setTailRotation();
	int_t getMaxSpawnedInChunk() override;
	jstring getWolfOwner();
	void setWolfOwner(const jstring &s);
	bool isWolfSitting();
	void setWolfSitting(bool flag);
	bool isWolfAngry();
	void setWolfAngry(bool flag);
	bool isWolfTamed();
	void setWolfTamed(bool flag);

private:
	bool looksWithInterest;
	float field_25048_b;
	float field_25054_c;
	bool isWolfShaking;
	bool field_25052_g;
	float timeWolfIsShaking;
	float prevTimeWolfIsShaking;
};
