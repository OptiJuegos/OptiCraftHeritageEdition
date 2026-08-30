#pragma once

#include "EntityZombie.h"

class World;
class Entity;
class NBTTagCompound;
class DamageSource;

// net.minecraft.src.EntityPigZombie
class EntityPigZombie : public EntityZombie
{
public:
	EntityPigZombie(World *world);

	static constexpr int_t CLASS_ID = 15;
	int_t getEntityClassID() const override { return CLASS_ID; }

	void onUpdate() override;
	bool isAIEnabled() override;
	bool getCanSpawnHere() override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;

protected:
	Entity *findPlayerToAttack() override;

public:
	void onLivingUpdate() override;
	bool attackEntityFrom(Entity *entity, int_t i) override;
	bool attackEntityFrom(const DamageSource &source, int_t damage) override;

protected:
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDropItemId() override;
	void dropFewItems(bool recentlyHitByPlayer, int_t lootingLevel) override;
	void dropRareDrop(int_t lootingRoll) override;

public:
	ItemStack *getHeldItem() override;

private:
	void becomeAngryAt(Entity *entity);

	int angerLevel;
	int randomSoundDelay;
	static ItemStack *defaultHeldItem;
};
