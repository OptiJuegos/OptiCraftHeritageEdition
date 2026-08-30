#pragma once

#include "EntityMob.h"

class World;
class NBTTagCompound;
class DamageSource;
class EntityLightningBolt;
class Entity;

// net.minecraft.src.EntityCreeper
class EntityCreeper : public EntityMob
{
public:
	EntityCreeper(World *world);

	static constexpr int_t CLASS_ID = 5;
	int_t getEntityClassID() const override { return CLASS_ID; }

	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;
	void onUpdate() override;
	bool isAIEnabled() override;
	bool attackEntityAsMob(Entity *target) override;
	void onDeath(Entity *entity) override;
	void onDeath(const DamageSource &source) override;
	bool getPowered();
	int_t getCreeperState();
	void setCreeperState(int_t state);
	float setCreeperFlashTime(float f);
	void onStruckByLightning(EntityLightningBolt *entitylightningbolt) override;

	int_t timeSinceIgnited;
	int_t lastActiveTime;

protected:
	void entityInit() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDropItemId() override;

};
