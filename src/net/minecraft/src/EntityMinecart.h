#pragma once

#include <string>

#include "Entity.h"
#include "IInventory.h"
#include "java/Type.h"

class AxisAlignedBB;
class EntityPlayer;
class ItemStack;
class NBTTagCompound;
class Vec3D;
class World;

// net.minecraft.src.EntityMinecart
class EntityMinecart : public Entity, public IInventory
{
public:
	EntityMinecart(World *world);
	EntityMinecart(World *world, double d, double d1, double d2, int_t i);
	~EntityMinecart() override;

	static constexpr int_t CLASS_ID = 2;
	int_t getEntityClassID() const override { return CLASS_ID; }

protected:
	bool canTriggerWalking() override;
	void entityInit() override;

public:
	AxisAlignedBB *getCollisionBox(Entity *entity) override;
	AxisAlignedBB *getBoundingBox() override;
	bool canBePushed() override { return true; }
	double getMountedYOffset() override;
	bool attackEntityFrom(Entity *entity, int_t i) override;
	bool attackEntityFrom(const DamageSource &source, int_t damage) override;
	void performHurtAnimation() override;
	bool canBeCollidedWith() override;
	void setEntityDead() override;
	void onUpdate() override;
	void applyEntityCollision(Entity *entity) override;

	// func_514_g — current snapped rail position at (d,d1,d2)
	Vec3D *getRailPosition(double d, double d1, double d2);
	Vec3D *getPos(double d, double d1, double d2) { return getRailPosition(d, d1, d2); }
	// func_515_a — projects a position along the rail by a delta
	Vec3D *getNextRailPosition(double d, double d1, double d2, double d3);
	Vec3D *getPosOffset(double d, double d1, double d2, double d3) { return getNextRailPosition(d, d1, d2, d3); }
	// minecartType getter
	int_t getMinecartType() const { return minecartType; }

public:
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;
	float getShadowSize() override;

	// IInventory
	int_t getSizeInventory() override;
	ItemStack *getStackInSlot(int_t i) override;
	ItemStack *getStackInSlotOnClosing(int_t i) override;
	ItemStack *decrStackSize(int_t i, int_t j) override;
	void setInventorySlotContents(int_t i, ItemStack *itemstack) override;
	std::string getInvName() override;
	int_t getInventoryStackLimit() override;
	void onInventoryChanged() override;
	bool canInteractWith(EntityPlayer *entityplayer) override;

	bool interact(EntityPlayer *entityplayer) override;
	void setPositionAndRotation2(double d, double d1, double d2, float f, float f1, int_t i) override;
	void setVelocity(double d, double d1, double d2) override;

	void setDamageTaken(int_t value);
	int_t getDamageTaken() const;
	void setTimeSinceHit(int_t value);
	int_t getTimeSinceHit() const;
	void setForwardDirection(int_t value);
	int_t getForwardDirection() const;
	bool isMinecartPowered() const;
	void setMinecartPowered(bool powered);

public:
	ItemStack **cargoItems;

private:
	bool field_856_i;

public:
	int_t minecartType;
	int_t fuel;
	double pushX;
	double pushZ;

private:
	// Multiplayer position-interpolation cache (Java field_9415_k … field_9410_p).
	int_t  serverPosTicks;
	double serverPosX_;
	double serverPosY_;
	double serverPosZ_;
	double serverYaw_;
	double serverPitch_;
	// Last-known velocity from the network (Java field_9409_q / r / s).
	double velocityX;
	double velocityY;
	double velocityZ;

	// Rail-curve direction table: [metadata][end][x|y|z]. Java field_855_j.
	static const int_t railDirections[10][2][3];
};
