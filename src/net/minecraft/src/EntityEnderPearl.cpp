#include "EntityEnderPearl.h"

#include "DamageSource.h"
#include "EntityLiving.h"
#include "MovingObjectPosition.h"
#include "World.h"

EntityEnderPearl::EntityEnderPearl(World *world)
    : EntityThrowable(world)
{
}

EntityEnderPearl::EntityEnderPearl(World *world, EntityLiving *thrower)
    : EntityThrowable(world, thrower)
{
}

EntityEnderPearl::EntityEnderPearl(World *world, double x, double y, double z)
    : EntityThrowable(world, x, y, z)
{
}

void EntityEnderPearl::onImpact(MovingObjectPosition *hit)
{
    EntityLiving *thrower = getThrower();
    if (hit != nullptr && hit->entityHit != nullptr)
    {
        DamageSource source = DamageSource::causeThrownDamage(this, thrower);
        hit->entityHit->attackEntityFrom(source, 0);
    }

    for (int_t i = 0; i < 32; ++i)
    {
        const double particleY = posY + rand.nextDouble() * 2.0;
        const double particleMotionX = rand.nextGaussian();
        const double particleMotionZ = rand.nextGaussian();
        worldObj->spawnParticle("portal", posX, particleY, posZ,
                                particleMotionX, 0.0, particleMotionZ);
    }

    if (!worldObj->multiplayerWorld)
    {
        if (thrower != nullptr)
        {
            thrower->setPositionAndUpdate(posX, posY, posZ);
            thrower->fallDistance = 0.0f;
            thrower->attackEntityFrom(DamageSource::fall, 5);
        }
        setEntityDead();
    }
}
