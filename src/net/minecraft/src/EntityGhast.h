#pragma once

#include "EntityFlying.h"
#include "IMob.h"

class World;
class Entity;
class DamageSource;

// net.minecraft.src.EntityGhast
class EntityGhast : public EntityFlying, public IMob
{
public:
	EntityGhast(World *world);

	static constexpr int_t CLASS_ID = 6;
	int_t getEntityClassID() const override { return CLASS_ID; }

protected:
	void entityInit() override;

public:
	int_t getMaxHealth() const override;
	void onUpdate() override;
	bool attackEntityFrom(const DamageSource &source, int_t damage) override;
	void updatePlayerActionState() override;

private:
	bool isCourseTraversable(double d, double d1, double d2, double d3);

protected:
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDropItemId() override;
	void dropFewItems(bool recentlyHitByPlayer, int_t lootingLevel) override;
	float getSoundVolume() override;

public:
	bool getCanSpawnHere() override;
	int_t getMaxSpawnedInChunk() override;

public:
	int courseChangeCooldown = 0;
	double waypointX = 0.0;
	double waypointY = 0.0;
	double waypointZ = 0.0;
	int prevAttackCounter = 0;
	int attackCounter = 0;

private:
	int_t targetedEntityId = -1;
	int aggroCooldown = 0;
};
