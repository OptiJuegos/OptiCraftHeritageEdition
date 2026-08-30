#pragma once

#include "EntityAnimal.h"

class World;
class NBTTagCompound;

// net.minecraft.src.EntityChicken
class EntityChicken : public EntityAnimal
{
public:
	explicit EntityChicken(World *world);

	bool isAIEnabled() override;
	int_t getMaxHealth() const;
	void onLivingUpdate() override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override { EntityAnimal::writeEntityToNBT(nbttagcompound); }
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override { EntityAnimal::readEntityFromNBT(nbttagcompound); }
	EntityAnimal *spawnBabyAnimal(EntityAnimal *mate) override;

protected:
	void fall(float f) override { (void)f; }
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDropItemId() override;
	void dropFewItems(bool recentlyHitByPlayer, int_t lootingLevel) override;

public:
	bool field_753_a;
	float field_752_b;
	float destPos;
	float field_757_d;
	float field_756_e;
	float field_755_h;
	int_t timeUntilNextEgg;
};
