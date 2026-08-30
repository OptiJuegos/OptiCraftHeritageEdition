#pragma once

#include "Entity.h"
#include "java/Type.h"

class World;
class EntityPlayer;
class EntityLiving;
class NBTTagCompound;
class ItemStack;

// net.minecraft.src.EntityFish
class EntityFish : public Entity
{
public:
	bool isInRangeToRenderDist(double d) override;
	EntityFish(World *world);
	EntityFish(World *world, double d, double d1, double d2);
	EntityFish(World *world, EntityPlayer *entityplayer);

protected:
	void entityInit() override;

public:
	void onUpdate() override;
	void setEntityDead() override;
	void handleHealthUpdate(byte_t byte0) override;
	int_t catchFish();
	EntityPlayer *getAngler() const;
	Entity *getBobber() const;
	void setFishHeading(double d, double d1, double d2, float f, float f1);
	void calculateVelocity(double d, double d1, double d2, float speed, float inaccuracy) { setFishHeading(d, d1, d2, speed, inaccuracy); }
	void setVelocity(double d, double d1, double d2) override;
	void setPositionAndRotation2(double d, double d1, double d2, float f, float f1, int_t i) override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;
	bool canBeCollidedWith() override;
	float getShadowSize() override;
public:
	int_t xTile;
	int_t yTile;
	int_t zTile;
	int_t inTile;
	bool  inGround;
	int_t shake;
private:
	void setBobber(Entity *entity);
	int_t anglerEntityId;
	int_t bobberEntityId;
	int ticksInGround;
	int ticksInAir;
	int ticksCatchable;
	int fishPosRotationIncrements;
	double fishX = 0.0;
	double fishY = 0.0;
	double fishZ = 0.0;
	double fishYaw = 0.0;
	double fishPitch = 0.0;
	double velocityX = 0.0;
	double velocityY = 0.0;
	double velocityZ = 0.0;
};
