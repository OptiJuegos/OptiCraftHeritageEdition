#include "EntitySorter.h"

#include "java/Arithmetic.h"

#include "Entity.h"
#include "WorldRenderer.h"

EntitySorter::EntitySorter(Entity *entity)
{
	field_30008_a = -entity->posX;
	field_30007_b = -entity->posY;
	field_30009_c = -entity->posZ;
}

int_t EntitySorter::sortByDistanceToEntity(WorldRenderer *worldrenderer, WorldRenderer *worldrenderer1)
{
	double d  = (double)worldrenderer->posXPlus  + field_30008_a;
	double d1 = (double)worldrenderer->posYPlus  + field_30007_b;
	double d2 = (double)worldrenderer->posZPlus  + field_30009_c;
	double d3 = (double)worldrenderer1->posXPlus + field_30008_a;
	double d4 = (double)worldrenderer1->posYPlus + field_30007_b;
	double d5 = (double)worldrenderer1->posZPlus + field_30009_c;
	return JavaArithmetic::doubleToInt(((d * d + d1 * d1 + d2 * d2) - (d3 * d3 + d4 * d4 + d5 * d5)) * 1024.0);
}

int_t EntitySorter::compare(WorldRenderer *obj, WorldRenderer *obj1)
{
	return sortByDistanceToEntity(obj, obj1);
}

bool EntitySorter::operator()(WorldRenderer *a, WorldRenderer *b)
{
	// Compare raw squared distances (not the int-cast compare() result) so this
	// is a valid strict weak ordering for std::sort. The *1024 int truncation in
	// compare()/Java's Comparator can round distinct distances to equal and break
	// transitivity, which is undefined behaviour for std::sort.
	double d  = (double)a->posXPlus + field_30008_a;
	double d1 = (double)a->posYPlus + field_30007_b;
	double d2 = (double)a->posZPlus + field_30009_c;
	double d3 = (double)b->posXPlus + field_30008_a;
	double d4 = (double)b->posYPlus + field_30007_b;
	double d5 = (double)b->posZPlus + field_30009_c;
	return (d * d + d1 * d1 + d2 * d2) < (d3 * d3 + d4 * d4 + d5 * d5);
}
