#pragma once

#include "EntityFX.h"

class Tessellator;
class World;

// net.minecraft.src.EntitySpellParticleFX
class EntitySpellParticleFX : public EntityFX
{
public:
	EntitySpellParticleFX(World *world, double x, double y, double z,
	                      double motionX, double motionY, double motionZ);

	void renderParticle(Tessellator *tessellator, float partialTick,
	                    float rotationX, float rotationXZ, float rotationZ,
	                    float rotationYZ, float rotationXY) override;
	void onUpdate() override;
	void setBaseSpellTextureIndex(int_t index);

private:
	int_t baseSpellTextureIndex;
};
