#pragma once

#include "EntityWaterMob.h"

class World;
class EntityPlayer;
class ItemStack;
class NBTTagCompound;

// net.minecraft.src.EntitySquid
class EntitySquid : public EntityWaterMob
{
public:
	EntitySquid(World *world);

	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;

protected:
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	float getSoundVolume() override;
	int_t getDropItemId() override;
	void dropFewItems(bool recentlyHitByPlayer, int_t lootingLevel) override;

public:
	bool interact(EntityPlayer *entityplayer) override;
	bool isInWater() override;
	void onLivingUpdate() override;
	void moveEntityWithHeading(float f, float f1) override;
	int_t getMaxHealth() const override;
	bool getCanSpawnHere() override;

protected:
	void updatePlayerActionState() override;

public:
	float field_21089_a;
	float field_21088_b;
	float field_21087_c;
	float field_21086_f;
	float field_21085_g;
	float field_21084_h;
	float field_21083_i;
	float field_21082_j;

private:
	float randomMotionSpeed;
	float field_21080_l;
	float field_21079_m;
	float randomMotionVecX;
	float randomMotionVecY;
	float randomMotionVecZ;
};
