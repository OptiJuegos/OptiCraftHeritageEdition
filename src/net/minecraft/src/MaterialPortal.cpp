#include "MaterialPortal.h"

MaterialPortal::MaterialPortal(MapColor *mapcolor) :
	Material(mapcolor)
{
}

bool MaterialPortal::isSolid()
{
	return false;
}

bool MaterialPortal::getCanBlockGrass()
{
	return false;
}

bool MaterialPortal::getIsSolid()
{
	return false;
}


bool MaterialPortal::blocksMovement()
{
	return false;
}
