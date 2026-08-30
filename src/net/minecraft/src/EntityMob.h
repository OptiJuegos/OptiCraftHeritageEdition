#pragma once

#include "EntityCreature.h"
#include "IMob.h"

class World;
class Entity;
class NBTTagCompound;
class DamageSource;

// net.minecraft.src.EntityMob
class EntityMob : public EntityCreature, public IMob
{
public:
	explicit EntityMob(World *world);

	bool isMob() const override { return true; }
	void onLivingUpdate() override;
	void onUpdate() override;
	bool attackEntityFrom(Entity *entity, int_t damage) override;
	bool attackEntityFrom(const DamageSource &source, int_t damage) override;
	bool attackEntityAsMob(Entity *target) override;
	void writeEntityToNBT(NBTTagCompound *tag) override { EntityCreature::writeEntityToNBT(tag); }
	void readEntityFromNBT(NBTTagCompound *tag) override { EntityCreature::readEntityFromNBT(tag); }
	bool getCanSpawnHere() override;

protected:
	Entity *findPlayerToAttack() override;
	void attackEntity(Entity *entity, float distance) override;
	float getBlockPathWeight(int_t x, int_t y, int_t z) override;
	virtual bool isValidLightLevel();

	int_t attackStrength;
};
