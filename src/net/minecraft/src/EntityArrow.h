#pragma once

#include "Entity.h"
#include "java/Type.h"

class World;
class EntityLiving;
class EntityPlayer;
class NBTTagCompound;

// net.minecraft.src.EntityArrow
class EntityArrow : public Entity
{
public:
	EntityArrow(World *world);
	EntityArrow(World *world, double x, double y, double z);
	EntityArrow(World *world, EntityLiving *shooter);
	EntityArrow(World *world, EntityLiving *shooter, float velocity);
	EntityArrow(World *world, EntityLiving *shooter, EntityLiving *target, float speed, float spread);

	static constexpr int_t CLASS_ID = 8;
	int_t getEntityClassID() const override { return CLASS_ID; }

protected:
	void entityInit() override;

public:
	void setArrowHeading(double x, double y, double z, float velocity, float inaccuracy);
	void setVelocity(double x, double y, double z) override;
	void onUpdate() override;
	void writeEntityToNBT(NBTTagCompound *compound) override;
	void readEntityFromNBT(NBTTagCompound *compound) override;
	void onCollideWithPlayer(EntityPlayer *player) override;
	float getShadowSize() override;
	bool canAttackWithItem() override;

	void setDamage(double value);
	double getDamage() const;
	void setKnockbackStrength(int_t strength);
	EntityLiving *getShootingEntity();
	void setShootingEntity(EntityLiving *shooter);

public:
	int_t xTile;
	int_t yTile;
	int_t zTile;
	int_t inTile;
	int_t inData;
	bool inGround;
	bool doesArrowBelongToPlayer;
	int_t arrowShake;
	bool arrowCritical;
	// Compatibility alias for older callers. Use getShootingEntity() for lifetime-safe access.
	EntityLiving *owner;

private:
	int_t ticksInGround;
	int_t ticksInAir;
	double damage;
	int_t knockbackStrength;
	int_t shootingEntityId;
};
