#include "GuiPlayerInfo.h"

GuiPlayerInfo::GuiPlayerInfo(const jstring &playerName) :
	name(playerName),
	nameInLowerCase(String::toLowerCaseJava(playerName))
{
}

bool GuiPlayerInfo::nameStartsWith(const jstring &prefix) const
{
	const jstring lowerPrefix = String::toLowerCaseJava(prefix);
	return nameInLowerCase.size() >= lowerPrefix.size() &&
	       nameInLowerCase.compare(0, lowerPrefix.size(), lowerPrefix) == 0;
}
