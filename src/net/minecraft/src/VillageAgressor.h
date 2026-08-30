#pragma once

#include "java/Type.h"

class EntityLiving;
class Village;
class World;

// net.minecraft.src.VillageAgressor
class VillageAgressor
{
public:
    VillageAgressor(Village *village, EntityLiving *aggressor, int_t aggressionTime);

    EntityLiving *resolve(World *world) const;
    bool matches(const EntityLiving *entity) const;

    int_t entityId;
    int_t agressionTime;
    Village *villageObj;
};
