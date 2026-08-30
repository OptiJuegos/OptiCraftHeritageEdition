#pragma once

#include "Entity.h"
#include "java/Type.h"

class World;
class EnumArt;
class EntityPlayer;
class NBTTagCompound;
class ItemStack;

// net.minecraft.src.EntityPainting
class EntityPainting : public Entity
{
public:
	EntityPainting(World *world);
	EntityPainting(World *world, int_t i, int_t j, int_t k, int_t l);
	EntityPainting(World *world, int_t i, int_t j, int_t k, int_t l, jstring s);

	static constexpr int_t CLASS_ID = 10;
	int_t getEntityClassID() const override { return CLASS_ID; }

protected:
	void entityInit() override;

public:
	void setDirection(int_t i);
	void onUpdate() override;
	bool onValidSurface();
	bool canBeCollidedWith() override;
	bool attackEntityFrom(Entity *entity, int_t i) override;
	bool attackEntityFrom(const DamageSource &source, int_t damage) override;
	void moveEntity(double d, double d1, double d2) override;
	void addVelocity(double d, double d1, double d2) override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;

private:
	float getOffsetForSize(int_t i);
	void dropPaintingItem();

public:
	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	int_t direction;
	EnumArt *art = nullptr;

private:
	int_t field_695_c;
};
