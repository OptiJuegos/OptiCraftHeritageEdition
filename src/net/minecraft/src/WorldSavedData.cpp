#include "WorldSavedData.h"

WorldSavedData::WorldSavedData(const jstring &name) : mapName(name), dirty(false)
{
}

void WorldSavedData::markDirty()
{
	setDirty(true);
}

void WorldSavedData::setDirty(bool value)
{
	dirty = value;
}

bool WorldSavedData::isDirty() const
{
	return dirty;
}
