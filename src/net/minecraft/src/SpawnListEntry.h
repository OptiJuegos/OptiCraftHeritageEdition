#pragma once

#include <typeindex>
#include "java/Type.h"
#include "WeightedRandomChoice.h"

// net.minecraft.src.SpawnListEntry
// Java Class replaced with std::type_index
class SpawnListEntry : public WeightedRandomChoice
{
public:
    SpawnListEntry(std::type_index cls, int_t weight);
    SpawnListEntry(std::type_index cls, int_t weight, int_t minGroupCount, int_t maxGroupCount);

    std::type_index entityClass;
    int_t spawnRarityRate;
    int_t minGroupCount;
    int_t maxGroupCount;
};
