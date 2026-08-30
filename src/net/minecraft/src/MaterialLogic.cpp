#include "MaterialLogic.h"

MaterialLogic::MaterialLogic(MapColor *mapcolor) :
	Material(mapcolor)
{
}

bool MaterialLogic::isSolid()
{
	return false;
}

bool MaterialLogic::getCanBlockGrass()
{
	return false;
}

bool MaterialLogic::getIsSolid()
{
	return false;
}


bool MaterialLogic::blocksMovement()
{
	return false;
}
