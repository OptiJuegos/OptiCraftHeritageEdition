#pragma once

#include <string>
#include "IStatStringFormat.h"

class Minecraft;

// net.minecraft.src.StatStringFormatKeyInv
class StatStringFormatKeyInv : public IStatStringFormat
{
public:
	StatStringFormatKeyInv(Minecraft *minecraft);

	std::string formatString(const std::string &s) override;

private:
	Minecraft *mc;
};
