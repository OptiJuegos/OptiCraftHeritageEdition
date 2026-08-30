#pragma once

#include "EntityMob.h"

// net.minecraft.src.EntitySilverfish
class EntitySilverfish : public EntityMob
{
public:
	explicit EntitySilverfish(World *world);

	int_t getMaxHealth() const;
	void onUpdate() override;
	bool getCanSpawnHere() override;

	EnumCreatureAttribute getCreatureAttribute() const override;

protected:
	bool canTriggerWalking() override;
	Entity *findPlayerToAttack() override;
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	void attackEntity(Entity *entity, float distance) override;
	void playStepSound(int_t x, int_t y, int_t z, int_t blockId) override;
	int_t getDropItemId() override;
	void updatePlayerActionState() override;
	float getBlockPathWeight(int_t x, int_t y, int_t z) override;
	bool isValidLightLevel() override;

public:
	bool attackEntityFrom(Entity *entity, int_t damage) override;
	bool attackEntityFrom(const DamageSource &source, int_t damage) override;

private:
	int_t allySummonCooldown;
};
