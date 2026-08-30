#pragma once

#include "EntityMob.h"

class World;
class NBTTagCompound;

// net.minecraft.src.EntityZombie
class EntityZombie : public EntityMob
{
public:
	EntityZombie(World *world);

	void onLivingUpdate() override;
	bool isAIEnabled() override;
	int_t getMaxHealth() const override;
	int_t getTotalArmorValue() const override;

	EnumCreatureAttribute getCreatureAttribute() const override;

protected:
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDropItemId() override;
	void dropRareDrop(int_t lootingRoll) override;
};
