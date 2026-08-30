#pragma once

#include "EntityWeatherEffect.h"

class World;
class Vec3D;
class NBTTagCompound;

// net.minecraft.src.EntityLightningBolt
class EntityLightningBolt : public EntityWeatherEffect
{
public:
	EntityLightningBolt(World *world, double d, double d1, double d2);

	static constexpr int_t CLASS_ID = 14;
	int_t getEntityClassID() const override { return CLASS_ID; }

	void onUpdate() override;
	bool isInRangeToRenderVec3D(Vec3D *vec3d) override;

protected:
	void entityInit() override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;

private:
	int_t field_27028_b;
public:
	long_t field_27029_a;
private:
	int_t field_27030_c;
};
