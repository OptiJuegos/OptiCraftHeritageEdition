#pragma once

#include "EntityMob.h"

class World;
class Entity;
class NBTTagCompound;
class ItemStack;
class DamageSource;

// net.minecraft.src.EntitySkeleton
class EntitySkeleton : public EntityMob
{
public:
	EntitySkeleton(World *world);

	static constexpr int_t CLASS_ID = 7;
	int_t getEntityClassID() const override { return CLASS_ID; }

	void onLivingUpdate() override;
	void onDeath(const DamageSource &source) override;
	bool isAIEnabled() override;

	EnumCreatureAttribute getCreatureAttribute() const override;

protected:
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;

public:
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;

protected:
	int_t getDropItemId() override;
	void dropFewItems(bool recentlyHitByPlayer, int_t lootingLevel) override;
	void dropRareDrop(int_t lootingRoll) override;

public:
	ItemStack *getHeldItem() override;

private:
	static ItemStack *defaultHeldItem;
};
