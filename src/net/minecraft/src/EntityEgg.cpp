#include "EntityEgg.h"

#include "DamageSource.h"
#include "EntityChicken.h"
#include "EntityLiving.h"
#include "MovingObjectPosition.h"
#include "World.h"

EntityEgg::EntityEgg(World *world)
	: EntityThrowable(world)
{
}

EntityEgg::EntityEgg(World *world, EntityLiving *thrower)
	: EntityThrowable(world, thrower)
{
}

EntityEgg::EntityEgg(World *world, double x, double y, double z)
	: EntityThrowable(world, x, y, z)
{
}

void EntityEgg::onImpact(MovingObjectPosition *hit)
{
	if (hit != nullptr && hit->entityHit != nullptr)
	{
		DamageSource source = DamageSource::causeThrownDamage(this, getThrower());
		hit->entityHit->attackEntityFrom(source, 0);
	}

	if (!worldObj->multiplayerWorld && rand.nextInt(8) == 0)
	{
		int_t count = 1;
		if (rand.nextInt(32) == 0)
			count = 4;
		for (int_t i = 0; i < count; ++i)
		{
			EntityChicken *chicken = new EntityChicken(worldObj);
			chicken->setGrowingAge(-24000);
			chicken->setLocationAndAngles(posX, posY, posZ, rotationYaw, 0.0f);
			if (!worldObj->entityJoinedWorld(chicken))
				delete chicken;
		}
	}

	for (int_t i = 0; i < 8; ++i)
		worldObj->spawnParticle("snowballpoof", posX, posY, posZ, 0.0, 0.0, 0.0);

	if (!worldObj->multiplayerWorld)
		setEntityDead();
}
