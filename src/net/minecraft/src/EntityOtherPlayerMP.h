#pragma once

#include "EntityPlayer.h"
#include "java/Type.h"

class World;
class ItemStack;

// net.minecraft.src.EntityOtherPlayerMP
class EntityOtherPlayerMP : public EntityPlayer
{
public:
	EntityOtherPlayerMP(World *world, jstring s);

protected:
	void resetHeight() override;

public:
	bool attackEntityFrom(Entity *entity, int_t i) override;
	bool attackEntityFrom(const DamageSource &source, int_t damage) override;
	void setPositionAndRotation2(double d, double d1, double d2, float f, float f1, int_t i) override;
	void onUpdate() override;
	float getShadowSize() override;
	float getEyeHeight() override;
	void onLivingUpdate() override;
	void outfitWithItem(int_t i, int_t j, int_t k);
	void handleItemUseFinish();

private:
	bool isItemInUse;
	// Java initializes instance fields to zero.  These interpolation values
	// are read before the first network update, so keep the same semantics.
	int_t field_785_bg = 0;
	double field_784_bh = 0.0;
	double field_783_bi = 0.0;
	double field_782_bj = 0.0;
	double field_780_bk = 0.0;
	double field_786_bl = 0.0;
	float field_20924_a = 0.0f;
};
