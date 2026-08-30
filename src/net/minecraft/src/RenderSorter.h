#ifndef RENDERSORTER_H
#define RENDERSORTER_H

#include <vector>
#include <algorithm>

class WorldRenderer;
class EntityLiving;

class RenderSorter {
public:
    RenderSorter(EntityLiving* entityLiving);

    int doCompare(WorldRenderer* worldrenderer, WorldRenderer* worldrenderer1);

    // strict-weak-ordering "less" predicate for std::sort, consistent with
    // Java's Collections.sort(list, comparator) (ascending by doCompare)
    bool operator()(WorldRenderer* worldrenderer, WorldRenderer* worldrenderer1);

private:
    EntityLiving* baseEntity;
};

#endif
