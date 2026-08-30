#pragma once

#include "EntityCreature.h"

class World;
class NBTTagCompound;

// net.minecraft.src.EntityWaterMob
class EntityWaterMob : public EntityCreature
{
public:
	EntityWaterMob(World *world) : EntityCreature(world) {}

	bool isWaterMob() const override { return true; }
	bool canBreatheUnderwater() override { return true; }
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override { EntityCreature::writeEntityToNBT(nbttagcompound); }
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override { EntityCreature::readEntityFromNBT(nbttagcompound); }
	bool getCanSpawnHere() override;
	int_t getTalkInterval() override { return 120; }

protected:
	int_t getExperiencePoints(EntityPlayer *player) override;
};
