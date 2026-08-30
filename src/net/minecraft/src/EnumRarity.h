#pragma once

#include "java/String.h"

// net.minecraft.src.EnumRarity
class EnumRarity
{
public:
	EnumRarity(int color, const char *label) : nameColor(color), field_40532_f(label) {}

	int nameColor;
	jstring field_40532_f;

	static const EnumRarity common;
	static const EnumRarity uncommon;
	static const EnumRarity rare;
	static const EnumRarity epic;
};
