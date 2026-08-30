#pragma once

#include <vector>

#include "java/Type.h"

class Random;
class WeightedRandomChoice;

// net.minecraft.src.WeightedRandom
class WeightedRandom
{
public:
	static int_t getTotalWeight(const std::vector<WeightedRandomChoice *> &items);
	static WeightedRandomChoice *getRandomItem(Random &random,
	                                           const std::vector<WeightedRandomChoice *> &items,
	                                           int_t totalWeight);
	static WeightedRandomChoice *getRandomItem(Random &random,
	                                           const std::vector<WeightedRandomChoice *> &items);
};
