#pragma once

#include "Entity.h"
#include "java/Type.h"

class World;
class NBTTagCompound;

// net.minecraft.src.EntityTNTPrimed
class EntityTNTPrimed : public Entity
{
public:
	EntityTNTPrimed(World *world);
	EntityTNTPrimed(World *world, double d, double d1, double d2);

protected:
	void entityInit() override;
	bool canTriggerWalking() override;

public:
	bool canBeCollidedWith() override;
	void onUpdate() override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;
	float getShadowSize() override;

private:
	void explode();

public:
	int_t fuse;
};
