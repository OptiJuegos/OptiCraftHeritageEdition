#pragma once

#include "java/String.h"

// net.minecraft.src.GuiPlayerInfo
class GuiPlayerInfo
{
public:
	explicit GuiPlayerInfo(const jstring &playerName);

	bool nameStartsWith(const jstring &prefix) const;

	const jstring name;
	int_t responseTime = 0;

private:
	const jstring nameInLowerCase;
};
