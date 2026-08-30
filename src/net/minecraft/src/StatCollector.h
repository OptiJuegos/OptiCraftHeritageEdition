#pragma once

#include <string>

class StringTranslate;

// net.minecraft.src.StatCollector
class StatCollector
{
public:
	static std::string translateToLocal(const std::string &s);
	static std::string translateToLocalFormatted(const std::string &s, const std::string &arg);

private:
	static StringTranslate *localizedName;
};
