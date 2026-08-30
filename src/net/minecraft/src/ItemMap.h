#pragma once

#include "ItemMapBase.h"

class MapData;
class World;
class Entity;
class ItemStack;

class ItemMap : public ItemMapBase {
public:
    ItemMap(int id);

    // Java 1.2.5: getMPMapData(short, World). Kept under the historic C++ name for packet callers.
    static MapData* getMapData(int_t id, World* world);
    static MapData* getHorizon(short word0, World* world);
    MapData* getMapData(ItemStack* itemstack, World* world);
    // Legacy readable-name alias retained for existing renderer callers.
    MapData* getAverageGroundLevel(ItemStack* itemstack, World* world) { return getMapData(itemstack, world); }
    void initDimension(World* world, Entity* entity, MapData* mapdata);
    void onUpdate(ItemStack* itemstack, World* world, Entity* entity, int i, bool flag) override;
    void onCreated(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) override;
};
