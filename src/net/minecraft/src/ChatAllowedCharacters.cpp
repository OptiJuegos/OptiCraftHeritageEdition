#include "ChatAllowedCharacters.h"
#include <vector>

#include "java/Resource.h"
#include "java/String.h"

#include <memory>

const std::string ChatAllowedCharacters::allowedCharacters = ChatAllowedCharacters::getAllowedCharacters();

const char ChatAllowedCharacters::allowedCharactersArray[15] = {
	'/', '\n', '\r', '\t', '\0', '\f', '`', '?', '*', '\\',
	'<', '>', '|', '"', ':'
};

std::string ChatAllowedCharacters::getAllowedCharacters()
{
	std::unique_ptr<std::istream> bufferedreader(Resource::getResource("/font.txt"));
	std::string result;

	std::string line;
	while (std::getline(*bufferedreader, line))
	{
		if (line.empty() || line[0] == '#')
			continue;
		if (line.back() == '\r')
			line.pop_back();
		result += line;
	}
	return result;
}


bool ChatAllowedCharacters::isAllowedCharacter(char_t c)
{
	return c != 167 && (String::indexOfUtf16Unit(allowedCharacters, c) >= 0 || c > 32);
}

std::string ChatAllowedCharacters::filterAllowedCharacters(const std::string &text)
{
	std::vector<char_t> filtered;
	for (char_t c : String::toUtf16(jstring(text)))
	{
		if (isAllowedCharacter(c))
			filtered.push_back(c);
	}
	return String::fromUtf16(filtered);
}
