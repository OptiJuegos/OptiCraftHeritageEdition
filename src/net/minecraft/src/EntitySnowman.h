#pragma once

#include "EntityGolem.h"

// net.minecraft.src.EntitySnowman
class EntitySnowman : public EntityGolem
{
public:
	explicit EntitySnowman(World *world);

	bool isAIEnabled() override;
	int_t getMaxHealth() const override;
	void onLivingUpdate() override;

protected:
	void writeEntityToNBT(NBTTagCompound *tag) override;
	void readEntityFromNBT(NBTTagCompound *tag) override;
	int_t getDropItemId() override;
	void dropFewItems(bool recentlyHitByPlayer, int_t lootingLevel) override;
};
