#pragma once

#include <string>
#include "java/Type.h"

// net.minecraft.src.IStatType
class IStatType
{
public:
	virtual ~IStatType() = default;
	virtual std::string format(int_t i) = 0;
};
