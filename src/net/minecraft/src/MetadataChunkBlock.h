#pragma once

#include "java/Type.h"

class EnumSkyBlock;
class World;

// net.minecraft.src.MetadataChunkBlock
class MetadataChunkBlock
{
public:
    MetadataChunkBlock(EnumSkyBlock *skyBlock, int_t minX, int_t minY, int_t minZ,
                       int_t maxX, int_t maxY, int_t maxZ);

    void updateLight(World *world);
    bool tryMerge(int_t minX, int_t minY, int_t minZ, int_t maxX, int_t maxY, int_t maxZ);

    const EnumSkyBlock *skyBlock;
    int_t minX;
    int_t minY;
    int_t minZ;
    int_t maxX;
    int_t maxY;
    int_t maxZ;
};
