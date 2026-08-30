#pragma once

#include "Entity.h"
#include "java/Type.h"

class World;
class NBTTagCompound;
class AxisAlignedBB;
class EntityPlayer;

// net.minecraft.src.EntityBoat
class EntityBoat : public Entity
{
public:
	EntityBoat(World *world);
	EntityBoat(World *world, double d, double d1, double d2);

	static constexpr int_t CLASS_ID = 3;
	int_t getEntityClassID() const override { return CLASS_ID; }

protected:
	void entityInit() override;
	bool canTriggerWalking() override { return false; }

public:
	AxisAlignedBB *getCollisionBox(Entity *entity) override;
	AxisAlignedBB *getBoundingBox() override { return boundingBox; }
	bool canBeCollidedWith() override;
	bool attackEntityFrom(Entity *entity, int_t i) override;
	bool attackEntityFrom(const DamageSource &source, int_t damage) override;
	void performHurtAnimation() override;
	bool canBePushed() override { return true; }
	double getMountedYOffset() override;
	bool interact(EntityPlayer *entityplayer) override;
	void updateRiderPosition() override;

	int_t getForwardDirection() const;
	void setForwardDirection(int_t value);
	int_t getTimeSinceHit() const;
	void setTimeSinceHit(int_t value);
	int_t getDamageTaken() const;
	void setDamageTaken(int_t value);

protected:
	void setPositionAndRotation2(double d, double d1, double d2, float f, float f1, int_t i) override;

public:
	void setVelocity(double d, double d1, double d2) override;
	void onUpdate() override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;
	float getShadowSize() override;


private:
	int_t boatTicks;        // field_9394_d
	double boatX;           // field_9393_e
	double boatY;           // field_9392_f
	double boatZ;           // field_9391_g
	double boatYaw;         // field_9390_h
	double boatPitch;       // field_9389_i
	double velocityX;       // field_9388_j
	double velocityY;       // field_9387_k
	double velocityZ;       // field_9386_l
};
