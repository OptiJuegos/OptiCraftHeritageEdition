#pragma once

#include "EntityMob.h"

class World;
class Entity;
class NBTTagCompound;
class PotionEffect;

// net.minecraft.src.EntitySpider
class EntitySpider : public EntityMob
{
public:
	EntitySpider(World *world);

	static constexpr int_t CLASS_ID = 13;
	int_t getEntityClassID() const override { return CLASS_ID; }

	double getMountedYOffset() override;
	int_t getMaxHealth() const override;
	void onUpdate() override;

	EnumCreatureAttribute getCreatureAttribute() const override;

protected:
	void entityInit() override;
	bool canTriggerWalking() override;
	Entity *findPlayerToAttack() override;
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	void attackEntity(Entity *entity, float f) override;

public:
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;

protected:
	int_t getDropItemId() override;
	void dropFewItems(bool recentlyHitByPlayer, int_t lootingLevel) override;

public:
	bool isOnLadder() override;
	void setInWeb() override;
	bool isPotionApplicable(PotionEffect *effect) const override;
	virtual float spiderScaleAmount();
	bool isBesideClimbableBlock() const;
	void setBesideClimbableBlock(bool climbing);
};
