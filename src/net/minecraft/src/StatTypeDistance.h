#pragma once

#include <string>
#include "IStatType.h"

// net.minecraft.src.StatTypeDistance
class StatTypeDistance : public IStatType
{
public:
	StatTypeDistance();

	std::string format(int_t i) override;
};
