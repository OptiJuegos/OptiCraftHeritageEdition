#pragma once

#include "java/Type.h"

class MapData;

// net.minecraft.src.MapCoord
class MapCoord
{
public:
	MapCoord(MapData *mapdata, byte_t byte0, byte_t byte1, byte_t byte2, byte_t byte3);

	byte_t iconSize;        // iconSize
	byte_t centerX;         // centerX
	byte_t centerZ;         // centerZ
	byte_t iconRotation;    // iconRotation
	MapData *mapDataObj;    // mapDataObj
};
