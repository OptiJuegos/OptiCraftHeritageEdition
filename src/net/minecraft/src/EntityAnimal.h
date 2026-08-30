#pragma once

#include "EntityAgeable.h"

class World;
class NBTTagCompound;
class EntityPlayer;
class ItemStack;

// net.minecraft.src.EntityAnimal
class EntityAnimal : public EntityAgeable
{
public:
	explicit EntityAnimal(World *world);

	bool isAnimal() const override { return true; }
	void onLivingUpdate() override;
	bool attackEntityFrom(Entity *entity, int_t damage) override;
	bool attackEntityFrom(const DamageSource &source, int_t damage) override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;
	bool getCanSpawnHere() override;
	int_t getTalkInterval() override { return 120; }
	bool interact(EntityPlayer *entityplayer) override;

	virtual EntityAnimal *spawnBabyAnimal(EntityAnimal *mate);
	virtual bool isWheat(ItemStack *itemstack) const;
	bool isInLove() const;
	void resetInLove();
	virtual bool canMateWith(EntityAnimal *mate) const;

protected:
	void updateAITick() override;
	void attackEntity(Entity *entity, float distance) override;
	void attackBlockedEntity(Entity *entity, float distance) override;
	Entity *findPlayerToAttack() override;
	float getBlockPathWeight(int_t i, int_t j, int_t k) override;
	bool canDespawn() override;
	int_t getExperiencePoints(EntityPlayer *player) override;

private:
	void procreate(EntityAnimal *mate);

	int_t inLove;
	int_t breeding;
};
