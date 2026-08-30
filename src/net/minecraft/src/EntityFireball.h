#pragma once

#include "Entity.h"
#include "java/Type.h"

class World;
class EntityLiving;
class NBTTagCompound;
class MovingObjectPosition;

// net.minecraft.src.EntityFireball
class EntityFireball : public Entity
{
public:
	EntityFireball(World *world);
	EntityFireball(World *world, EntityLiving *shooter, double xAcceleration, double yAcceleration, double zAcceleration);
	EntityFireball(World *world, double x, double y, double z, double xAcceleration, double yAcceleration, double zAcceleration);

	bool isInRangeToRenderDist(double distance) override;

protected:
	void entityInit() override;
	virtual void onImpact(MovingObjectPosition *hit);

public:
	void onUpdate() override;
	void writeEntityToNBT(NBTTagCompound *compound) override;
	void readEntityFromNBT(NBTTagCompound *compound) override;
	bool canBeCollidedWith() override;
	float getCollisionBorderSize() override;
	bool attackEntityFrom(Entity *entity, int_t damage) override;
	bool attackEntityFrom(const DamageSource &source, int_t damage) override;
	float getShadowSize() override;
	float getBrightness(float partialTick) override;
	int_t getBrightnessForRender(float partialTick) override;

	EntityLiving *getShootingEntity();
	void setShootingEntity(EntityLiving *shooter);

public:
	// Compatibility alias for older callers. Use getShootingEntity() for lifetime-safe access.
	EntityLiving *shootingEntity;
	double accelerationX;
	double accelerationY;
	double accelerationZ;

private:
	int_t xTile;
	int_t yTile;
	int_t zTile;
	int_t inTile;
	bool inGround;
	int_t ticksAlive;
	int_t ticksInAir;
	int_t shootingEntityId;
};
