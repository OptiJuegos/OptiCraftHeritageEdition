#pragma once

#include "EntityFX.h"

class World;

// net.minecraft.src.EntityEnchantmentTableParticleFX
class EntityEnchantmentTableParticleFX : public EntityFX
{
public:
	EntityEnchantmentTableParticleFX(World *world, double x, double y, double z,
	                                 double motionX, double motionY, double motionZ);
	int_t getBrightnessForRender(float partialTick) override;
	float getEntityBrightness(float partialTick) override;
	void onUpdate() override;

private:
	float initialScale;
	double originX;
	double originY;
	double originZ;
};
