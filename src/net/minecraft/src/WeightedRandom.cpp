#include "WeightedRandom.h"

#include <stdexcept>

#include "WeightedRandomChoice.h"
#include "java/Random.h"

int_t WeightedRandom::getTotalWeight(const std::vector<WeightedRandomChoice *> &items)
{
	int_t totalWeight = 0;
	for (WeightedRandomChoice *item : items)
	{
		if (item != nullptr)
			totalWeight += item->getItemWeight();
	}
	return totalWeight;
}

WeightedRandomChoice *WeightedRandom::getRandomItem(Random &random,
                                                    const std::vector<WeightedRandomChoice *> &items,
                                                    int_t totalWeight)
{
	if (totalWeight <= 0)
		throw std::invalid_argument("WeightedRandom total weight must be positive");

	int_t selection = random.nextInt(totalWeight);
	for (WeightedRandomChoice *item : items)
	{
		if (item == nullptr)
			continue;
		selection -= item->getItemWeight();
		if (selection < 0)
			return item;
	}
	return nullptr;
}

WeightedRandomChoice *WeightedRandom::getRandomItem(Random &random,
                                                    const std::vector<WeightedRandomChoice *> &items)
{
	return getRandomItem(random, items, getTotalWeight(items));
}
