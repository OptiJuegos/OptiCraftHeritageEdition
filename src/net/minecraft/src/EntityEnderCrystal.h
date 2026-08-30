#pragma once

#include "Entity.h"

class NBTTagCompound;
class DamageSource;

// net.minecraft.src.EntityEnderCrystal
class EntityEnderCrystal : public Entity
{
public:
	explicit EntityEnderCrystal(World *world);
	EntityEnderCrystal(World *world, double x, double y, double z);

	void onUpdate() override;
	float getShadowSize() override;
	bool canBeCollidedWith() override;
	bool attackEntityFrom(const DamageSource &source, int_t damage) override;

	int_t innerRotation;
	int_t health;

protected:
	bool canTriggerWalking() override;
	void entityInit() override;
	void writeEntityToNBT(NBTTagCompound *tag) override;
	void readEntityFromNBT(NBTTagCompound *tag) override;
};
