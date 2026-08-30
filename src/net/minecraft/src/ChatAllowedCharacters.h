#pragma once

#include <string>
#include "java/Type.h"

// net.minecraft.src.ChatAllowedCharacters
class ChatAllowedCharacters
{
public:
	static const std::string allowedCharacters;
	static const char allowedCharactersArray[15];
	static bool isAllowedCharacter(char_t c);
	static std::string filterAllowedCharacters(const std::string &text);
	static std::string func_52019_a(const std::string &text) { return filterAllowedCharacters(text); }

private:
	static std::string getAllowedCharacters();
};
