#pragma once

#include "Entity.h"
#include "java/Type.h"

class Tessellator;
class NBTTagCompound;
class World;

// net.minecraft.src.EntityFX
class EntityFX : public Entity
{
public:
	EntityFX(World *world, double d, double d1, double d2,
	         double d3, double d4, double d5);

	EntityFX *multiplyVelocity(float f);
	EntityFX *multiplyParticleScaleBy(float f);
	void setParticleColor(float red, float green, float blue);
	float getParticleRed() const;
	float getParticleGreen() const;
	float getParticleBlue() const;
	void setParticleTextureIndex(int_t index);
	int_t getParticleTextureIndex() const;
	bool canAttackWithItem() override;

	void onUpdate() override;

	virtual void renderParticle(Tessellator *tessellator, float f,
	                            float f1, float f2, float f3,
	                            float f4, float f5);
	virtual int_t getFXLayer();

	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;

protected:
	bool canTriggerWalking() override;
	void entityInit() override;
	static float interpolateRenderCoordinate(double previous, double current, double camera, float partialTick);

public:
	static double interpPosX;
	static double interpPosY;
	static double interpPosZ;

protected:
	int_t particleTextureIndex;
	float particleTextureJitterX;
	float particleTextureJitterY;
	int_t particleAge;
	int_t particleMaxAge;
	float particleScale;
	float particleGravity;
	float particleRed;
	float particleGreen;
	float particleBlue;
};
