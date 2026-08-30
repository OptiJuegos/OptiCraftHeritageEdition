#pragma once

#include "EntitySlime.h"

// net.minecraft.src.EntityMagmaCube
class EntityMagmaCube : public EntitySlime
{
public:
	explicit EntityMagmaCube(World *world);

	bool getCanSpawnHere() override;
	int_t getTotalArmorValue() const override;
	int_t getBrightnessForRender(float partialTick) override;
	float getEntityBrightness(float partialTick) override;
	bool isBurning() override;
	bool handleLavaMovement() override;

protected:
	jstring getSlimeParticle() override;
	EntitySlime *createInstance() override;
	int_t getDropItemId() override;
	void dropFewItems(bool recentlyHitByPlayer, int_t lootingLevel) override;
	int_t getJumpDelay() override;
	void updateSquish() override;
	void jump() override;
	void fall(float distance) override;
	bool canDamagePlayer() override;
	int_t getAttackStrength() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	jstring getSlimeSound() override;
	bool makesSoundOnLand() override;
};
