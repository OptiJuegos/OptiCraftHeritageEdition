#pragma once

#include <vector>

#include "NibbleArrayReader.h"
#include "java/Type.h"

class NBTTagList;

// net.minecraft.src.AnvilConverterData
class AnvilConverterData
{
public:
    AnvilConverterData(int_t chunkX, int_t chunkZ);

    long_t lastUpdated;
    bool terrainPopulated;
    std::vector<byte_t> heightmap;
    NibbleArrayReader blockLight;
    NibbleArrayReader skyLight;
    NibbleArrayReader data;
    std::vector<byte_t> blocks;
    NBTTagList *entities;
    NBTTagList *tileEntities;
    NBTTagList *tileTicks;
    const int_t x;
    const int_t z;
};
