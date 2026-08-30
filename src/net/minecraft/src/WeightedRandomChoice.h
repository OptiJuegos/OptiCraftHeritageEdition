#pragma once

#include "java/Type.h"

// net.minecraft.src.WeightedRandomChoice
class WeightedRandomChoice
{
public:
	explicit WeightedRandomChoice(int_t weight);
	virtual ~WeightedRandomChoice() = default;

	int_t getItemWeight() const;

protected:
	int_t itemWeight;
};
