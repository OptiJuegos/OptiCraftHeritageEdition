#pragma once

#include "Entity.h"

class DamageSource;
class EntityPlayer;
class NBTTagCompound;
class World;

// net.minecraft.src.EntityXPOrb
class EntityXPOrb : public Entity
{
public:
	explicit EntityXPOrb(World *world);
	EntityXPOrb(World *world, double x, double y, double z, int_t value);

	bool canTriggerWalking() override;
	void entityInit() override;
	int_t getBrightnessForRender(float partialTick) override;
	void onUpdate() override;
	bool handleWaterMovement() override;
	void dealFireDamage(int_t damage) override;
	bool attackEntityFrom(const DamageSource &source, int_t damage) override;
	void writeEntityToNBT(NBTTagCompound *compound) override;
	void readEntityFromNBT(NBTTagCompound *compound) override;
	void onCollideWithPlayer(EntityPlayer *player) override;
	bool canAttackWithItem() override;

	int_t getXpValue() const;
	int_t getTextureByXP() const;
	static int_t getXPSplit(int_t value);

	int_t xpColor = 0;
	int_t xpOrbAge = 0;
	int_t field_35126_c = 0;

private:
	int_t xpOrbHealth = 5;
	int_t xpValue = 0;
};
