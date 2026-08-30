#include "VillageAgressor.h"

#include "Entity.h"
#include "EntityLiving.h"
#include "World.h"

VillageAgressor::VillageAgressor(Village *village, EntityLiving *aggressor, int_t time)
    : entityId(aggressor != nullptr ? aggressor->entityId : -1), agressionTime(time), villageObj(village)
{
}

EntityLiving *VillageAgressor::resolve(World *world) const
{
    if (world == nullptr || entityId < 0)
        return nullptr;
    return dynamic_cast<EntityLiving *>(world->getEntityByID(entityId));
}

bool VillageAgressor::matches(const EntityLiving *entity) const
{
    return entity != nullptr && entityId == entity->entityId;
}
