#include "GenLayerFuzzyZoom.h"

GenLayerFuzzyZoom::GenLayerFuzzyZoom(long_t seed, Ptr parentLayer)
    : GenLayerZoom(seed, std::move(parentLayer))
{
}

int_t GenLayerFuzzyZoom::choose(int_t first, int_t second, int_t third, int_t fourth)
{
    const int_t selected = nextInt(4);
    return selected == 0 ? first : selected == 1 ? second : selected == 2 ? third : fourth;
}
