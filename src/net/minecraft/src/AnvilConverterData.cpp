#include "AnvilConverterData.h"

AnvilConverterData::AnvilConverterData(int_t chunkX, int_t chunkZ)
    : lastUpdated(0), terrainPopulated(false), blockLight(std::vector<byte_t>(), 7), skyLight(std::vector<byte_t>(), 7), data(std::vector<byte_t>(), 7),
      entities(nullptr), tileEntities(nullptr), tileTicks(nullptr), x(chunkX), z(chunkZ)
{
}
