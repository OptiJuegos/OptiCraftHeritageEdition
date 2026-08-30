#include "TileEntityMobSpawner.h"

#include <cmath>

#include "NBTTagCompound.h"
#include "World.h"
#include "EntityList.h"
#include "EntityLiving.h"
#include "AxisAlignedBB.h"
#include "java/Random.h"

TileEntityMobSpawner::TileEntityMobSpawner()
    : delay(-1)
    , yaw(0.0)
    , yaw2(0.0)
    , mobID("Pig")
{
    delay = 20;
}

std::string TileEntityMobSpawner::getMobID()
{
    return mobID;
}

void TileEntityMobSpawner::setMobID(const std::string &s)
{
    mobID = s;
}

bool TileEntityMobSpawner::anyPlayerInRange()
{
    return worldObj->getClosestPlayer((double)xCoord + 0.5, (double)yCoord + 0.5, (double)zCoord + 0.5, 16.0) != nullptr;
}

void TileEntityMobSpawner::readFromNBT(NBTTagCompound* nbttagcompound)
{
    TileEntity::readFromNBT(nbttagcompound);
    mobID = nbttagcompound->getString("EntityId");
    delay = nbttagcompound->getShort("Delay");
}

void TileEntityMobSpawner::writeToNBT(NBTTagCompound* nbttagcompound)
{
    TileEntity::writeToNBT(nbttagcompound);
    nbttagcompound->setString("EntityId", mobID);
    nbttagcompound->setShort("Delay", (short)delay);
}

void TileEntityMobSpawner::updateEntity()
{
    yaw2 = yaw;
    if (!anyPlayerInRange())
        return;
    double d  = (float)xCoord + worldObj->rand.nextFloat();
    double d2 = (float)yCoord + worldObj->rand.nextFloat();
    double d4 = (float)zCoord + worldObj->rand.nextFloat();
    worldObj->spawnParticle("smoke", d, d2, d4, 0.0, 0.0, 0.0);
    worldObj->spawnParticle("flame", d, d2, d4, 0.0, 0.0, 0.0);
    for (yaw += 1000.0f / ((float)delay + 200.0f); yaw > 360.0;)
    {
        yaw  -= 360.0;
        yaw2 -= 360.0;
    }

    if (!worldObj->multiplayerWorld)
    {
        if (delay == -1)
            updateDelay();
        if (delay > 0)
        {
            delay--;
            return;
        }
        const int_t byte0 = 4;
        for (int_t i = 0; i < byte0; i++)
        {
            Entity *spawned = EntityList::createEntityInWorld(mobID, worldObj);
            EntityLiving *entityliving = dynamic_cast<EntityLiving *>(spawned);
            if (entityliving == nullptr)
            {
                delete spawned;
                return;
            }
            int_t j = (int_t)worldObj->getEntitiesWithinAABB(typeid(*entityliving),
                AxisAlignedBB::getBoundingBoxFromPool(xCoord, yCoord, zCoord, xCoord + 1, yCoord + 1, zCoord + 1)->expand(8.0, 4.0, 8.0)).size();
            if (j >= 6)
            {
                delete entityliving;
                updateDelay();
                return;
            }
            double d6 = (double)xCoord + worldObj->rand.nextDoubleDifference() * 4.0;
            double d7 = (yCoord + worldObj->rand.nextInt(3)) - 1;
            double d8 = (double)zCoord + worldObj->rand.nextDoubleDifference() * 4.0;
            entityliving->setLocationAndAngles(d6, d7, d8, worldObj->rand.nextFloat() * 360.0f, 0.0f);
            if (!entityliving->getCanSpawnHere())
            {
                delete entityliving;
                continue;
            }
            const bool joinedWorld = worldObj->entityJoinedWorld(entityliving);
            worldObj->playAuxSFX(2004, xCoord, yCoord, zCoord, 0);
            entityliving->spawnExplosionParticle();
            updateDelay();
            if (!joinedWorld)
                delete entityliving;
        }
    }
    TileEntity::updateEntity();
}

void TileEntityMobSpawner::updateDelay()
{
    delay = 200 + worldObj->rand.nextInt(600);
}
