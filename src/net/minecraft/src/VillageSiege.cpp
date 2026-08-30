#include "VillageSiege.h"
#include "java/Arithmetic.h"

#include <cmath>

#include "EntityPlayer.h"
#include "EntityZombie.h"
#include "EnumCreatureType.h"
#include "MathHelper.h"
#include "SpawnerAnimals.h"
#include "Vec3D.h"
#include "Village.h"
#include "VillageCollection.h"
#include "World.h"

namespace
{
constexpr float PI_F = 3.14159265358979323846f;
}

VillageSiege::VillageSiege(World *world)
    : worldObj(world), siegeSetup(false), siegeState(-1), remainingZombies(0), nextSpawnTime(0),
      selectedVillageCenter(0, 0, 0), selectedVillageRadius(0), spawnX(0), spawnY(0), spawnZ(0)
{
}

void VillageSiege::tick()
{
    if (worldObj == nullptr || worldObj->villageCollectionObj == nullptr)
        return;
    if (worldObj->isDaytime())
    {
        siegeState = 0;
        return;
    }
    if (siegeState == 2)
        return;
    if (siegeState == 0)
    {
        float angle = worldObj->getCelestialAngle(0.0f);
        if ((double)angle < 0.5 || (double)angle > 0.501)
            return;
        siegeState = worldObj->rand.nextInt(10) == 0 ? 1 : 2;
        siegeSetup = false;
        if (siegeState == 2)
            return;
    }
    if (!siegeSetup)
    {
        if (!trySetupSiege())
            return;
        siegeSetup = true;
    }
    if (nextSpawnTime > 0)
    {
        --nextSpawnTime;
        return;
    }
    nextSpawnTime = 2;
    if (remainingZombies > 0)
    {
        spawnZombie();
        --remainingZombies;
    }
    else
    {
        siegeState = 2;
    }
}

Village *VillageSiege::resolveVillage() const
{
    if (worldObj == nullptr || worldObj->villageCollectionObj == nullptr)
        return nullptr;
    const auto &villages = worldObj->villageCollectionObj->getVillageList();
    for (const auto &entry : villages)
    {
        Village *village = entry.get();
        if (village == nullptr)
            continue;
        ChunkCoordinates center = village->getCenter();
        if (center.x == selectedVillageCenter.x && center.y == selectedVillageCenter.y && center.z == selectedVillageCenter.z)
            return village;
    }
    return nullptr;
}

bool VillageSiege::trySetupSiege()
{
    for (EntityPlayer *player : worldObj->playerEntities)
    {
        if (player == nullptr || player->isDead)
            continue;
        Village *village = worldObj->villageCollectionObj->findNearestVillage(JavaArithmetic::doubleToInt(player->posX), JavaArithmetic::doubleToInt(player->posY), JavaArithmetic::doubleToInt(player->posZ), 1);
        if (village == nullptr || village->getNumVillageDoors() < 10 || village->getTicksSinceLastDoorAdding() < 20 || village->getNumVillagers() < 20)
            continue;

        ChunkCoordinates center = village->getCenter();
        float radius = (float)village->getVillageRadius();
        bool overlapsOtherVillage = false;
        for (int_t attempt = 0; attempt < 10; ++attempt)
        {
            const int_t offsetX = JavaArithmetic::doubleToInt(
                (double)(MathHelper::cos(worldObj->rand.nextFloat() * PI_F * 2.0f) * radius) * 0.9);
            spawnX = JavaArithmetic::intAdd(center.x, offsetX);
            spawnY = center.y;
            const int_t offsetZ = JavaArithmetic::doubleToInt(
                (double)(MathHelper::sin(worldObj->rand.nextFloat() * PI_F * 2.0f) * radius) * 0.9);
            spawnZ = JavaArithmetic::intAdd(center.z, offsetZ);
            overlapsOtherVillage = false;
            for (const auto &entry : worldObj->villageCollectionObj->getVillageList())
            {
                Village *other = entry.get();
                if (other != nullptr && other != village && other->isInRange(spawnX, spawnY, spawnZ))
                {
                    overlapsOtherVillage = true;
                    break;
                }
            }
            if (!overlapsOtherVillage)
                break;
        }
        if (overlapsOtherVillage)
            return false;
        if (findRandomSpawnPosition(spawnX, spawnY, spawnZ, village) == nullptr)
            continue;

        selectedVillageCenter = center;
        selectedVillageRadius = village->getVillageRadius();
        nextSpawnTime = 0;
        remainingZombies = 20;
        return true;
    }
    return false;
}

bool VillageSiege::spawnZombie()
{
    Village *village = resolveVillage();
    if (village == nullptr)
        return false;
    Vec3D *position = findRandomSpawnPosition(spawnX, spawnY, spawnZ, village);
    if (position == nullptr)
        return false;

    EntityZombie *zombie = new EntityZombie(worldObj);
    zombie->setLocationAndAngles(position->xCoord, position->yCoord, position->zCoord, worldObj->rand.nextFloat() * 360.0f, 0.0f);
    if (!worldObj->entityJoinedWorld(zombie))
    {
        delete zombie;
        return false;
    }
    ChunkCoordinates center = village->getCenter();
    zombie->setHomeArea(center.x, center.y, center.z, village->getVillageRadius());
    return true;
}

Vec3D *VillageSiege::findRandomSpawnPosition(int_t x, int_t y, int_t z, Village *village)
{
    if (village == nullptr)
        return nullptr;
    for (int_t attempt = 0; attempt < 10; ++attempt)
    {
        int_t candidateX = JavaArithmetic::intSub(JavaArithmetic::intAdd(x, worldObj->rand.nextInt(16)), 8);
        int_t candidateY = JavaArithmetic::intSub(JavaArithmetic::intAdd(y, worldObj->rand.nextInt(6)), 3);
        int_t candidateZ = JavaArithmetic::intSub(JavaArithmetic::intAdd(z, worldObj->rand.nextInt(16)), 8);
        if (village->isInRange(candidateX, candidateY, candidateZ) &&
            SpawnerAnimals::canCreatureTypeSpawnAtLocation(EnumCreatureType::monster, worldObj, candidateX, candidateY, candidateZ))
            return Vec3D::createVector((double)candidateX, (double)candidateY, (double)candidateZ);
    }
    return nullptr;
}
