#pragma once

#include "EntityCreature.h"

class NBTTagCompound;

// net.minecraft.src.EntityAgeable
class EntityAgeable : public EntityCreature
{
public:
	explicit EntityAgeable(World *world);

	int_t getGrowingAge();
	void setGrowingAge(int_t age);
	void writeEntityToNBT(NBTTagCompound *tag) override;
	void readEntityFromNBT(NBTTagCompound *tag) override;
	void onLivingUpdate() override;
	bool isChild() override;

protected:
	void entityInit() override;
};
