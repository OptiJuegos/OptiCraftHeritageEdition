#pragma once

#include "java/Type.h"

// net.minecraft.src.MapColor
class MapColor
{
public:
	MapColor(int_t index, int_t color);
	static void cleanup();

	static MapColor* mapColorArray[16];
	static MapColor* airColor;
	static MapColor* grassColor;
	static MapColor* sandColor;
	static MapColor* clothColor;
	static MapColor* tntColor;
	static MapColor* iceColor;
	static MapColor* ironColor;
	static MapColor* foliageColor;
	static MapColor* snowColor;
	static MapColor* clayColor;
	static MapColor* dirtColor;
	static MapColor* stoneColor;
	static MapColor* waterColor;
	static MapColor* woodColor;

	int_t colorValue;
	int_t colorIndex;
};
