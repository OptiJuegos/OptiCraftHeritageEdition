#pragma once

#include <string>
#include "IStatType.h"

// net.minecraft.src.StatTypeSimple
class StatTypeSimple : public IStatType
{
public:
	StatTypeSimple();

	std::string format(int_t i) override;
};
