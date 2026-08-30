#pragma once

#include "Entity.h"

class EntityLiving;
class MovingObjectPosition;
class NBTTagCompound;

// net.minecraft.src.EntityThrowable
class EntityThrowable : public Entity
{
public:
	explicit EntityThrowable(World *world);
	EntityThrowable(World *world, EntityLiving *thrower);
	EntityThrowable(World *world, double x, double y, double z);

	bool isInRangeToRenderDist(double distance) override;
	void setThrowableHeading(double x, double y, double z, float velocity, float inaccuracy);
	void setVelocity(double x, double y, double z) override;
	void onUpdate() override;
	void writeEntityToNBT(NBTTagCompound *tag) override;
	void readEntityFromNBT(NBTTagCompound *tag) override;
	void onCollideWithPlayer(EntityPlayer *player) override;
	float getShadowSize() override;

	EntityLiving *getThrower() const;
	int_t throwableShake = 0;

protected:
	void entityInit() override;
	virtual float getVelocity() const;
	virtual float getInaccuracyPitchOffset() const;
	virtual float getGravityVelocity() const;
	virtual void onImpact(MovingObjectPosition *hit) = 0;

	bool inGround = false;

private:
	int_t xTile = -1;
	int_t yTile = -1;
	int_t zTile = -1;
	int_t inTile = 0;
	int_t throwerEntityId = -1;
	int_t ticksInGround = 0;
	int_t ticksInAir = 0;
};
