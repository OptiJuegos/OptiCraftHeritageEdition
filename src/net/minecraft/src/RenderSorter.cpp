#include "RenderSorter.h"
#include "WorldRenderer.h"
#include "EntityLiving.h"

RenderSorter::RenderSorter(EntityLiving* entityLiving) {
    baseEntity = entityLiving;
}

int RenderSorter::doCompare(WorldRenderer* worldrenderer, WorldRenderer* worldrenderer1) {
    bool flag = worldrenderer->isInFrustum;
    bool flag1 = worldrenderer1->isInFrustum;
    if (flag && !flag1) {
        return 1;
    }
    if (flag1 && !flag) {
        return -1;
    }
    double d = worldrenderer->distanceToEntitySquared(baseEntity);
    double d1 = worldrenderer1->distanceToEntitySquared(baseEntity);
    if (d < d1) {
        return 1;
    }
    if (d > d1) {
        return -1;
    } else {
        return worldrenderer->chunkIndex >= worldrenderer1->chunkIndex ? -1 : 1;
    }
}

bool RenderSorter::operator()(WorldRenderer* worldrenderer, WorldRenderer* worldrenderer1) {
    // guard reflexivity: Java's doCompare is non-reflexive (returns -1 for a==a
    // via the chunkIndex tie-break), which std::sort forbids. Collections.sort
    // (TimSort) tolerates it; std::sort would be UB.
    if (worldrenderer == worldrenderer1) {
        return false;
    }
    return doCompare(worldrenderer, worldrenderer1) < 0;
}
