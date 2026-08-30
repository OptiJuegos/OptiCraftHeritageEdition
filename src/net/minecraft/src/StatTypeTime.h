#pragma once

#include <string>
#include "IStatType.h"

// net.minecraft.src.StatTypeTime
class StatTypeTime : public IStatType
{
public:
	StatTypeTime();

	std::string format(int_t i) override;
};
