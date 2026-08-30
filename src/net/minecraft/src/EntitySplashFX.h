#pragma once

#include "EntityRainFX.h"

class World;

// net.minecraft.src.EntitySplashFX
class EntitySplashFX : public EntityRainFX
{
public:
	EntitySplashFX(World *world, double d, double d1, double d2,
	               double d3, double d4, double d5);
};
