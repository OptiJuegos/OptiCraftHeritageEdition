#include "ItemMap.h"
#include "java/Arithmetic.h"
#include "MapData.h"
#include "World.h"
#include "ItemStack.h"
#include "WorldInfo.h"
#include "WorldProvider.h"
#include "Entity.h"
#include "MathHelper.h"
#include "Block.h"
#include "Chunk.h"
#include "Material.h"
#include "MapColor.h"
#include "EntityPlayer.h"
#include <cstdint>

ItemMap::ItemMap(int i)
    : ItemMapBase(i) {
    setMaxStackSize(1);
}

MapData* ItemMap::getMapData(int_t id, World* world)
{
    const std::string key = "map_" + std::to_string(id);
    MapData *mapdata = dynamic_cast<MapData*>(world->loadItemData(typeid(MapData), key));
    if (mapdata == nullptr)
    {
        const int_t newId = world->getUniqueDataId("map");
        const std::string newKey = "map_" + std::to_string(newId);
        mapdata = new MapData(newKey);
        world->setItemData(newKey, mapdata);
    }
    return mapdata;
}

MapData* ItemMap::getHorizon(short word0, World* world)
{
    return getMapData((int_t)word0, world);
}

MapData* ItemMap::getMapData(ItemStack* itemstack, World* world) {
    std::string s = "map_" + std::to_string(itemstack->getItemDamage());
    MapData* mapdata = dynamic_cast<MapData*>(world->loadItemData(typeid(MapData), s));
    if (mapdata == nullptr) {
        itemstack->setItemDamage(world->getUniqueDataId("map"));
        std::string s1 = "map_" + std::to_string(itemstack->getItemDamage());
        mapdata = new MapData(s1);
        mapdata->xCenter = world->getWorldInfo()->getSpawnX();
        mapdata->zCenter = world->getWorldInfo()->getSpawnZ();
        mapdata->scale = 3;
        mapdata->dimension = (byte_t)world->worldProvider->worldType;
        mapdata->markDirty();
        world->setItemData(s1, mapdata);
    }
    return mapdata;
}

void ItemMap::initDimension(World* world, Entity* entity, MapData* mapdata) {
    if (world->worldProvider->worldType != mapdata->dimension) {
        return;
    }
    int_t c = 128;
    int_t c1 = 128;
    int i = JavaArithmetic::intShl(1, mapdata->scale);
    int j = mapdata->xCenter;
    int k = mapdata->zCenter;
    int l = MathHelper::floor_double(entity->posX - (double)j) / i + c / 2;
    int i1 = MathHelper::floor_double(entity->posZ - (double)k) / i + c1 / 2;
    int j1 = 128 / i;
    if (world->worldProvider->hasNoSky) {
        j1 /= 2;
    }
    mapdata->achievementParent++;
    for (int k1 = (l - j1) + 1; k1 < l + j1; k1++) {
        if ((k1 & 0xf) != (mapdata->achievementParent & 0xf)) {
            continue;
        }
        int l1 = 255;
        int i2 = 0;
        double d = 0.0;
        for (int j2 = i1 - j1 - 1; j2 < i1 + j1; j2++) {
            if (k1 < 0 || j2 < -1 || k1 >= c || j2 >= c1) {
                continue;
            }
            int k2 = k1 - l;
            int l2 = j2 - i1;
            bool flag = k2 * k2 + l2 * l2 > (j1 - 2) * (j1 - 2);
            int i3 = ((j / i + k1) - c / 2) * i;
            int j3 = ((k / i + j2) - c1 / 2) * i;
            int k3 = 0;
            int l3 = 0;
            int i4 = 0;
            int ai[256] = {0};
            Chunk* chunk = world->getChunkFromBlockCoords(i3, j3);
            int j4 = i3 & 0xf;
            int k4 = j3 & 0xf;
            int l4 = 0;
            double d1 = 0.0;
            if (world->worldProvider->hasNoSky) {
                // Java int arithmetic wraps at 32 bits. Use unsigned operations
                // explicitly so optimized C++ builds reproduce Beta exactly.
                uint32_t i5u = uint32_t(i3) + uint32_t(j3) * 0x389bfu;
                i5u = i5u * i5u * 0x1dd6751u + i5u * 11u;
                int_t i5 = JavaArithmetic::intFromBits(i5u);
                if ((JavaArithmetic::intShr(i5, 20) & 1) == 0) {
                    ai[Block::dirt->blockID] += 10;
                } else {
                    ai[Block::stone->blockID] += 10;
                }
                d1 = 100.0;
            } else {
                for (int j5 = 0; j5 < i; j5++) {
                    for (int l5 = 0; l5 < i; l5++) {
                        int j6 = chunk->getHeightValue(j5 + j4, l5 + k4) + 1;
                        int l6 = 0;
                        if (j6 > 1) {
                            bool flag1 = false;
                            do {
                                flag1 = true;
                                l6 = chunk->getBlockID(j5 + j4, j6 - 1, l5 + k4);
                                if (l6 == 0) {
                                    flag1 = false;
                                } else if (j6 > 0 && l6 > 0 && Block::blocksList[l6]->blockMaterial->materialMapColor == MapColor::airColor) {
                                    flag1 = false;
                                }
                                if (!flag1) {
                                    j6--;
                                    l6 = chunk->getBlockID(j5 + j4, j6 - 1, l5 + k4);
                                }
                            } while (j6 > 0 && !flag1);
                            if (j6 > 0 && l6 != 0 && Block::blocksList[l6]->blockMaterial->getIsLiquid()) {
                                int i7 = j6 - 1;
                                int k7 = 0;
                                do {
                                    k7 = chunk->getBlockID(j5 + j4, i7--, l5 + k4);
                                    l4++;
                                } while (i7 > 0 && k7 != 0 && Block::blocksList[k7]->blockMaterial->getIsLiquid());
                            }
                        }
                        d1 += (double)j6 / (double)(i * i);
                        ai[l6]++;
                    }
                }
            }
            l4 /= i * i;
            k3 /= i * i;
            l3 /= i * i;
            i4 /= i * i;
            int k5 = 0;
            int i6 = 0;
            for (int k6 = 0; k6 < 256; k6++) {
                if (ai[k6] > k5) {
                    i6 = k6;
                    k5 = ai[k6];
                }
            }

            double d2 = ((d1 - d) * 4.0) / (double)(i + 4) + ((double)(k1 + j2 & 1) - 0.5) * 0.40000000000000002;
            char byte0 = 1;
            if (d2 > 0.59999999999999998) {
                byte0 = 2;
            }
            if (d2 < -0.59999999999999998) {
                byte0 = 0;
            }
            int j7 = 0;
            if (i6 > 0) {
                MapColor* mapcolor = Block::blocksList[i6]->blockMaterial->materialMapColor;
                if (mapcolor == MapColor::waterColor) {
                    double d3 = (double)l4 * 0.10000000000000001 + (double)(k1 + j2 & 1) * 0.20000000000000001;
                    byte0 = 1;
                    if (d3 < 0.5) {
                        byte0 = 2;
                    }
                    if (d3 > 0.90000000000000002) {
                        byte0 = 0;
                    }
                }
                j7 = mapcolor->colorIndex;
            }
            d = d1;
            if (j2 < 0 || k2 * k2 + l2 * l2 >= j1 * j1 || flag && (k1 + j2 & 1) == 0) {
                continue;
            }
            byte_t byte1 = mapdata->colors[k1 + j2 * c];
            byte_t byte2 = static_cast<byte_t>(j7 * 4 + byte0);
            if (byte1 == byte2) {
                continue;
            }
            if (l1 > j2) {
                l1 = j2;
            }
            if (i2 < j2) {
                i2 = j2;
            }
            mapdata->colors[k1 + j2 * c] = byte2;
        }

        if (l1 <= i2) {
            mapdata->setColumnDirty(k1, l1, i2);
        }
    }
}

void ItemMap::onUpdate(ItemStack* itemstack, World* world, Entity* entity, int i, bool flag) {
    if (world->multiplayerWorld) {
        return;
    }
    MapData* mapdata = getMapData(itemstack, world);
    EntityPlayer* entityplayer = entity->isPlayer() ? static_cast<EntityPlayer*>(entity) : nullptr;
    if (entityplayer) {
        mapdata->updateMapData(entityplayer, itemstack);
    }
    if (flag) {
        initDimension(world, entity, mapdata);
    }
}

void ItemMap::onCreated(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) {
    itemstack->setItemDamage(world->getUniqueDataId("map"));
    std::string s = "map_" + std::to_string(itemstack->getItemDamage());
    MapData* mapdata = new MapData(s);
    world->setItemData(s, mapdata);
    mapdata->xCenter = MathHelper::floor_double(entityplayer->posX);
    mapdata->zCenter = MathHelper::floor_double(entityplayer->posZ);
    mapdata->scale = 3;
    mapdata->dimension = (byte_t)world->worldProvider->worldType;
    mapdata->markDirty();
}
