#pragma once

#include "EntityMob.h"

// net.minecraft.src.EntityBlaze
class EntityBlaze : public EntityMob
{
public:
	explicit EntityBlaze(World *world);

	int_t getMaxHealth() const override;
	int_t getBrightnessForRender(float partialTick) override;
	float getEntityBrightness(float partialTick) override;
	void onLivingUpdate() override;
	bool attackEntityFrom(const DamageSource &source, int_t damage) override;
	void writeEntityToNBT(NBTTagCompound *tag) override;
	void readEntityFromNBT(NBTTagCompound *tag) override;
	bool isBurning() override;
	bool isBlazing() const;
	void setBlazing(bool blazing);

protected:
	void entityInit() override;
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	void attackEntity(Entity *entity, float distance) override;
	void fall(float distance) override;
	int_t getDropItemId() override;
	void dropFewItems(bool recentlyHitByPlayer, int_t lootingLevel) override;
	bool isValidLightLevel() override;

private:
	float heightOffset;
	int_t heightOffsetUpdateTime;
	int_t attackStep;
};
