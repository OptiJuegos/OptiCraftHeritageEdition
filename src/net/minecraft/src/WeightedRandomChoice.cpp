#include "WeightedRandomChoice.h"

WeightedRandomChoice::WeightedRandomChoice(int_t weight)
	: itemWeight(weight)
{
}

int_t WeightedRandomChoice::getItemWeight() const
{
	return itemWeight;
}
