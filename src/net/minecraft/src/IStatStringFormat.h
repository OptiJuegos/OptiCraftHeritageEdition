#pragma once

#include <string>

// net.minecraft.src.IStatStringFormat
class IStatStringFormat
{
public:
	virtual ~IStatStringFormat() = default;
	virtual std::string formatString(const std::string &s) = 0;
};
