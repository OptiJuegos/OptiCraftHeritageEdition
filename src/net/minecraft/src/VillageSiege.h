#pragma once

#include "ChunkCoordinates.h"
#include "java/Type.h"

class Vec3D;
class Village;
class World;

// net.minecraft.src.VillageSiege
class VillageSiege
{
public:
    explicit VillageSiege(World *world);

    void tick();

private:
    bool trySetupSiege();
    bool spawnZombie();
    Vec3D *findRandomSpawnPosition(int_t x, int_t y, int_t z, Village *village);
    Village *resolveVillage() const;

    World *worldObj;
    bool siegeSetup;
    int_t siegeState;
    int_t remainingZombies;
    int_t nextSpawnTime;
    ChunkCoordinates selectedVillageCenter;
    int_t selectedVillageRadius;
    int_t spawnX;
    int_t spawnY;
    int_t spawnZ;
};
