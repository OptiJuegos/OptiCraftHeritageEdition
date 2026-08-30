#include "MapCoord.h"

MapCoord::MapCoord(MapData *mapdata, byte_t byte0, byte_t byte1, byte_t byte2, byte_t byte3) :
	mapDataObj(mapdata),
	iconSize(byte0),
	centerX(byte1),
	centerZ(byte2),
	iconRotation(byte3)
{
}
