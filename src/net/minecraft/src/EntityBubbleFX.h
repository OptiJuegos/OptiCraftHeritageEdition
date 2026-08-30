#pragma once

#include "EntityFX.h"

class World;

// net.minecraft.src.EntityBubbleFX
class EntityBubbleFX : public EntityFX
{
public:
	EntityBubbleFX(World *world, double d, double d1, double d2,
	               double d3, double d4, double d5);

	void onUpdate() override;
};
