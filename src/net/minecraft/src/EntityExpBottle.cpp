#include "EntityExpBottle.h"
#include "java/Arithmetic.h"

#include <cmath>

#include "EntityXPOrb.h"
#include "MovingObjectPosition.h"
#include "World.h"

EntityExpBottle::EntityExpBottle(World *world)
    : EntityThrowable(world)
{
}

EntityExpBottle::EntityExpBottle(World *world, EntityLiving *thrower)
    : EntityThrowable(world, thrower)
{
}

EntityExpBottle::EntityExpBottle(World *world, double x, double y, double z)
    : EntityThrowable(world, x, y, z)
{
}

float EntityExpBottle::getGravityVelocity() const
{
    return 0.07f;
}

float EntityExpBottle::getVelocity() const
{
    return 0.7f;
}

float EntityExpBottle::getInaccuracyPitchOffset() const
{
    return -20.0f;
}

void EntityExpBottle::onImpact(MovingObjectPosition *)
{
    if (worldObj->multiplayerWorld)
        return;

    const int_t x = JavaArithmetic::longToInt(JavaArithmetic::roundDouble(posX));
    const int_t y = JavaArithmetic::longToInt(JavaArithmetic::roundDouble(posY));
    const int_t z = JavaArithmetic::longToInt(JavaArithmetic::roundDouble(posZ));
    worldObj->playAuxSFX(2002, x, y, z, 0);

    int_t experience = 3 + worldObj->rand.nextInt(5) + worldObj->rand.nextInt(5);
    while (experience > 0)
    {
        const int_t split = EntityXPOrb::getXPSplit(experience);
        experience -= split;
        EntityXPOrb *orb = new EntityXPOrb(worldObj, posX, posY, posZ, split);
        if (!worldObj->spawnEntityInWorld(orb))
            delete orb;
    }
    setEntityDead();
}
