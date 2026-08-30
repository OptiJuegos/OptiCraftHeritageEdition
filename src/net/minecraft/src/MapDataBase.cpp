#include "MapDataBase.h"

MapDataBase::MapDataBase(const std::string &s) :
	mapName(s),
	dirty(false)
{
}

void MapDataBase::markDirty()
{
	setDirty(true);
}

void MapDataBase::setDirty(bool flag)
{
	dirty = flag;
}

bool MapDataBase::isDirty()
{
	return dirty;
}
