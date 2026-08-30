#include "MaterialLiquid.h"

MaterialLiquid::MaterialLiquid(MapColor *mapcolor) :
	Material(mapcolor)
{
	setIsGroundCover();
	setNoPushMobility();
}

bool MaterialLiquid::getIsLiquid()
{
	return true;
}

bool MaterialLiquid::getIsSolid()
{
	return false;
}

bool MaterialLiquid::isSolid()
{
	return false;
}


bool MaterialLiquid::blocksMovement()
{
	return false;
}
