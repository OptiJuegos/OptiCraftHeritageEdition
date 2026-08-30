#pragma once

#include "EntityFX.h"
#include "java/String.h"

class Entity;
class Tessellator;
class World;

// net.minecraft.src.EntityCrit2FX
class EntityCrit2FX : public EntityFX
{
public:
	EntityCrit2FX(World *world, Entity *entity);
	EntityCrit2FX(World *world, Entity *entity, const jstring &particleName);

	void renderParticle(Tessellator *tessellator, float partialTick,
	                    float rotationX, float rotationXZ, float rotationZ,
	                    float rotationYZ, float rotationXY) override;
	void onUpdate() override;
	int_t getFXLayer() override;

private:
	int_t targetEntityId;
	int_t currentLife;
	int_t maximumLife;
	jstring particleName;
};
