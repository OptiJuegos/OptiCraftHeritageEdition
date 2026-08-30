#include "EntityHugeExplodeFX.h"

#include "World.h"

EntityHugeExplodeFX::EntityHugeExplodeFX(World *world, double x, double y, double z,
                                         double, double, double)
	: EntityFX(world, x, y, z, 0.0, 0.0, 0.0), timeSinceStart(0), maximumTime(8)
{
}

void EntityHugeExplodeFX::renderParticle(Tessellator *, float, float, float, float, float, float)
{
}

void EntityHugeExplodeFX::onUpdate()
{
	for (int_t i = 0; i < 6; ++i)
	{
		const double x = posX + (rand.nextDoubleDifference()) * 4.0;
		const double y = posY + (rand.nextDoubleDifference()) * 4.0;
		const double z = posZ + (rand.nextDoubleDifference()) * 4.0;
		worldObj->spawnParticle("largeexplode", x, y, z,
		                        static_cast<double>(static_cast<float>(timeSinceStart) /
		                                            static_cast<float>(maximumTime)),
		                        0.0, 0.0);
	}
	++timeSinceStart;
	if (timeSinceStart == maximumTime)
		setEntityDead();
}

int_t EntityHugeExplodeFX::getFXLayer()
{
	return 1;
}
