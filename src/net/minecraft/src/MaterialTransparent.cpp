#include "MaterialTransparent.h"

MaterialTransparent::MaterialTransparent(MapColor *mapcolor) :
	Material(mapcolor)
{
	setIsGroundCover();
}

bool MaterialTransparent::isSolid()
{
	return false;
}

bool MaterialTransparent::getCanBlockGrass()
{
	return false;
}

bool MaterialTransparent::getIsSolid()
{
	return false;
}


bool MaterialTransparent::blocksMovement()
{
	return false;
}
