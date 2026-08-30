#include "EntitySnowball.h"

#include "DamageSource.h"
#include "EntityBlaze.h"
#include "EntityLiving.h"
#include "MovingObjectPosition.h"
#include "World.h"

EntitySnowball::EntitySnowball(World *world)
	: EntityThrowable(world)
{
}

EntitySnowball::EntitySnowball(World *world, EntityLiving *thrower)
	: EntityThrowable(world, thrower)
{
}

EntitySnowball::EntitySnowball(World *world, double x, double y, double z)
	: EntityThrowable(world, x, y, z)
{
}

void EntitySnowball::onImpact(MovingObjectPosition *hit)
{
	if (hit != nullptr && hit->entityHit != nullptr)
	{
		const int_t damage = dynamic_cast<EntityBlaze *>(hit->entityHit) != nullptr ? 3 : 0;
		DamageSource source = DamageSource::causeThrownDamage(this, getThrower());
		hit->entityHit->attackEntityFrom(source, damage);
	}

	for (int_t i = 0; i < 8; ++i)
		worldObj->spawnParticle("snowballpoof", posX, posY, posZ, 0.0, 0.0, 0.0);

	if (!worldObj->multiplayerWorld)
		setEntityDead();
}
