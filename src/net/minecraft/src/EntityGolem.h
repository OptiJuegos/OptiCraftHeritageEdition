#pragma once

#include "EntityCreature.h"

// net.minecraft.src.EntityGolem
class EntityGolem : public EntityCreature
{
public:
	explicit EntityGolem(World *world);

	int_t getTalkInterval() override;

protected:
	void fall(float distance) override;
	void writeEntityToNBT(NBTTagCompound *tag) override;
	void readEntityFromNBT(NBTTagCompound *tag) override;
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	bool canDespawn() override;
};
