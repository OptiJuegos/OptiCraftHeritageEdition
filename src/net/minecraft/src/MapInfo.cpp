#include "MapInfo.h"

MapInfo::MapInfo(MapData *mapdata, EntityPlayer *entityplayer) :
	mapDataObj(mapdata),
	entityplayerObj(entityplayer),
	hasSkyLight(0),
	isHellWorld(0)
{
	for (int_t i = 0; i < 128; i++)
	{
		minVisibleY[i] = 0;
		maxVisibleY[i] = 127;
	}
}
