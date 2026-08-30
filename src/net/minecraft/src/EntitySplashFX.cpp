#include "EntitySplashFX.h"

#include "World.h"

EntitySplashFX::EntitySplashFX(World *world, double d, double d1, double d2,
                               double d3, double d4, double d5)
	: EntityRainFX(world, d, d1, d2)
{
	particleGravity = 0.04f;
	particleTextureIndex++;
	if (d4 == 0.0 && (d3 != 0.0 || d5 != 0.0))
	{
		motionX = d3;
		motionY = d4 + 0.1;
		motionZ = d5;
	}
}
