#pragma once

#include "java/Type.h"
#include "java/String.h"

// net.minecraft.src.EnumArt
class EnumArt
{
public:
	static const EnumArt Kebab;
	static const EnumArt Aztec;
	static const EnumArt Alban;
	static const EnumArt Aztec2;
	static const EnumArt Bomb;
	static const EnumArt Plant;
	static const EnumArt Wasteland;
	static const EnumArt Pool;
	static const EnumArt Courbet;
	static const EnumArt Sea;
	static const EnumArt Sunset;
	static const EnumArt Creebet;
	static const EnumArt Wanderer;
	static const EnumArt Graham;
	static const EnumArt Match;
	static const EnumArt Bust;
	static const EnumArt Stage;
	static const EnumArt Void;
	static const EnumArt SkullAndRoses;
	static const EnumArt Fighters;
	static const EnumArt Pointer;
	static const EnumArt Pigscene;
	static const EnumArt BurningSkull;
	static const EnumArt Skeleton;
	static const EnumArt DonkeyKong;

	static const EnumArt *values[25];
	static constexpr int_t valuesCount = 25;
	static const int_t maxArtTitleLength;

	const jstring title;
	const int_t sizeX;
	const int_t sizeY;
	const int_t offsetX;
	const int_t offsetY;

private:
	EnumArt(const jstring &s1, int_t j, int_t k, int_t l, int_t i1)
		: title(s1), sizeX(j), sizeY(k), offsetX(l), offsetY(i1) {}
};