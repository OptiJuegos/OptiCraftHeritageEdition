#pragma once

#include "EntityFX.h"

class Material;
class World;

// net.minecraft.src.EntityDropParticleFX
class EntityDropParticleFX : public EntityFX
{
public:
	EntityDropParticleFX(World *world, double x, double y, double z, Material *material);
	int_t getBrightnessForRender(float partialTick) override;
	float getEntityBrightness(float partialTick) override;
	void onUpdate() override;

private:
	Material *materialType;
	int_t bobTimer;
};
