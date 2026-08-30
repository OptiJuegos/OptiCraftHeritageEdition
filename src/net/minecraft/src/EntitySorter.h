#pragma once

#include "java/Type.h"

class Entity;
class WorldRenderer;

// net.minecraft.src.EntitySorter
// Comparator that sorts WorldRenderers by distance from an entity.
class EntitySorter
{
public:
	EntitySorter(Entity *entity);

	int_t sortByDistanceToEntity(WorldRenderer *worldrenderer, WorldRenderer *worldrenderer1);

	// Comparator interface
	int_t compare(WorldRenderer *obj, WorldRenderer *obj1);
	bool operator()(WorldRenderer *a, WorldRenderer *b);

private:
	double field_30008_a;
	double field_30007_b;
	double field_30009_c;
};
