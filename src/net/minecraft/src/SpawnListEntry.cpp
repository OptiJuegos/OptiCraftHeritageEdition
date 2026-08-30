#include "SpawnListEntry.h"

SpawnListEntry::SpawnListEntry(std::type_index cls, int_t weight)
    : SpawnListEntry(cls, weight, 1, 1)
{
}

SpawnListEntry::SpawnListEntry(std::type_index cls, int_t weight, int_t minimumGroupCount, int_t maximumGroupCount)
    : WeightedRandomChoice(weight),
      entityClass(cls),
      spawnRarityRate(weight),
      minGroupCount(minimumGroupCount),
      maxGroupCount(maximumGroupCount)
{
}
