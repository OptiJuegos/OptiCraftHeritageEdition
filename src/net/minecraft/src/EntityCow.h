#pragma once

#include "EntityAnimal.h"

class World;
class NBTTagCompound;
class EntityPlayer;

// net.minecraft.src.EntityCow
class EntityCow : public EntityAnimal
{
public:
	explicit EntityCow(World *world);

	static constexpr int_t CLASS_ID = 11;
	int_t getEntityClassID() const override { return CLASS_ID; }

	bool isAIEnabled() override;
	int_t getMaxHealth() const;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override { EntityAnimal::writeEntityToNBT(nbttagcompound); }
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override { EntityAnimal::readEntityFromNBT(nbttagcompound); }
	bool interact(EntityPlayer *entityplayer) override;
	EntityAnimal *spawnBabyAnimal(EntityAnimal *mate) override;

protected:
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	float getSoundVolume() override;
	int_t getDropItemId() override;
	void dropFewItems(bool recentlyHitByPlayer, int_t lootingLevel) override;
};
