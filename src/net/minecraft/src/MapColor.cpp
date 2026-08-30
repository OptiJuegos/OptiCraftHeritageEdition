#include "MapColor.h"

MapColor* MapColor::mapColorArray[16] = {nullptr};
MapColor* MapColor::airColor = nullptr;
MapColor* MapColor::grassColor = nullptr;
MapColor* MapColor::sandColor = nullptr;
MapColor* MapColor::clothColor = nullptr;
MapColor* MapColor::tntColor = nullptr;
MapColor* MapColor::iceColor = nullptr;
MapColor* MapColor::ironColor = nullptr;
MapColor* MapColor::foliageColor = nullptr;
MapColor* MapColor::snowColor = nullptr;
MapColor* MapColor::clayColor = nullptr;
MapColor* MapColor::dirtColor = nullptr;
MapColor* MapColor::stoneColor = nullptr;
MapColor* MapColor::waterColor = nullptr;
MapColor* MapColor::woodColor = nullptr;

MapColor::MapColor(int_t index, int_t color) :
	colorIndex(index),
	colorValue(color)
{
	mapColorArray[index] = this;
}

// Static initialization
struct MapColorInit
{
	MapColorInit()
	{
		MapColor::airColor = new MapColor(0, 0);
		MapColor::grassColor = new MapColor(1, 0x7fb238);
		MapColor::sandColor = new MapColor(2, 0xf7e9a3);
		MapColor::clothColor = new MapColor(3, 0xa7a7a7);
		MapColor::tntColor = new MapColor(4, 0xff0000);
		MapColor::iceColor = new MapColor(5, 0xa0a0ff);
		MapColor::ironColor = new MapColor(6, 0xa7a7a7);
		MapColor::foliageColor = new MapColor(7, 31744);
		MapColor::snowColor = new MapColor(8, 0xffffff);
		MapColor::clayColor = new MapColor(9, 0xa4a8b8);
		MapColor::dirtColor = new MapColor(10, 0xb76a2f);
		MapColor::stoneColor = new MapColor(11, 0x707070);
		MapColor::waterColor = new MapColor(12, 0x4040ff);
		MapColor::woodColor = new MapColor(13, 0x685332);
	}
} mapColorInit;


void MapColor::cleanup()
{
    for (int i = 0; i < 16; i++)
    {
        delete mapColorArray[i];
        mapColorArray[i] = nullptr;
    }
    airColor = nullptr;
    grassColor = nullptr;
    sandColor = nullptr;
    clothColor = nullptr;
    tntColor = nullptr;
    iceColor = nullptr;
    ironColor = nullptr;
    foliageColor = nullptr;
    snowColor = nullptr;
    clayColor = nullptr;
    dirtColor = nullptr;
    stoneColor = nullptr;
    waterColor = nullptr;
    woodColor = nullptr;
}
