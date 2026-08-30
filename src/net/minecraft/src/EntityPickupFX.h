#pragma once

#include "EntityFX.h"

class World;
class Entity;
class Tessellator;

// net.minecraft.src.EntityPickupFX
class EntityPickupFX : public EntityFX
{
public:
	EntityPickupFX(World *world, Entity *entity, Entity *entity1, float f);
	~EntityPickupFX() override;

	void renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5) override;
	void onUpdate() override;
	int_t getFXLayer() override;

private:
	Entity *field_675_a;
	double  collectorLastPosX;
	double  collectorLastPosY;
	double  collectorLastPosZ;
	double  collectorPosX;
	double  collectorPosY;
	double  collectorPosZ;
	int_t   field_678_p;
	int_t   field_677_q;
	float   field_676_r;
	bool    ownsRenderEntity; // true when field_675_a is our private copy (must delete it)
};
