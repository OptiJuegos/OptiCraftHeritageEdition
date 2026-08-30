#pragma once

#include "EntityAnimal.h"

class World;
class NBTTagCompound;
class EntityPlayer;
class EntityLightningBolt;

// net.minecraft.src.EntityPig
class EntityPig : public EntityAnimal
{
public:
	explicit EntityPig(World *world);

	static constexpr int_t CLASS_ID = 4;
	int_t getEntityClassID() const override { return CLASS_ID; }

	bool isAIEnabled() override;
	int_t getMaxHealth() const;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;
	bool interact(EntityPlayer *entityplayer) override;
	bool getSaddled();
	void setSaddled(bool flag);
	void onStruckByLightning(EntityLightningBolt *entitylightningbolt) override;
	EntityAnimal *spawnBabyAnimal(EntityAnimal *mate) override;

protected:
	void entityInit() override;
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDropItemId() override;
	void fall(float f) override;
};
