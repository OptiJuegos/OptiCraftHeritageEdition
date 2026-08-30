#pragma once

#include "EntityTameable.h"

class EntityAITempt;
class EntityPlayer;
class ItemStack;

// net.minecraft.src.EntityOcelot
class EntityOcelot : public EntityTameable
{
public:
	explicit EntityOcelot(World *world);

	static constexpr int_t CLASS_ID = 30;
	int_t getEntityClassID() const override { return CLASS_ID; }

	bool isAIEnabled() override;
	int_t getMaxHealth() const override;
	bool attackEntityAsMob(Entity *target) override;
	bool attackEntityFrom(Entity *source, int_t damage) override;
	bool attackEntityFrom(const DamageSource &source, int_t damage) override;
	bool interact(EntityPlayer *player) override;
	EntityAnimal *spawnBabyAnimal(EntityAnimal *mate) override;
	bool isWheat(ItemStack *itemstack) const override;
	bool canMateWith(EntityAnimal *mate) const override;
	bool getCanSpawnHere() override;
	int_t getCatType() const;
	void setCatType(int_t type);

protected:
	void entityInit() override;
	void updateAITick() override;
	bool canDespawn() override;
	void fall(float distance) override;
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	float getSoundVolume() override;
	int_t getDropItemId() override;
	void dropFewItems(bool recentlyHitByPlayer, int_t lootingLevel) override;

public:
	const char *getEntityTexture() override;
	void writeEntityToNBT(NBTTagCompound *compound) override;
	void readEntityFromNBT(NBTTagCompound *compound) override;

private:
	EntityAITempt *aiTempt;
};
