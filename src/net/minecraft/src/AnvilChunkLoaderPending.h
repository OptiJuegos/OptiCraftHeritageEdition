#pragma once

#include <utility>
#include <vector>

#include "ChunkCoordIntPair.h"
#include "java/Type.h"

// net.minecraft.src.AnvilChunkLoaderPending
class AnvilChunkLoaderPending
{
public:
    AnvilChunkLoaderPending(const ChunkCoordIntPair &position, std::vector<byte_t> data)
        : chunkPosition(position), serializedData(std::move(data))
    {
    }

    ~AnvilChunkLoaderPending();

    AnvilChunkLoaderPending(const AnvilChunkLoaderPending &) = delete;
    AnvilChunkLoaderPending &operator=(const AnvilChunkLoaderPending &) = delete;

    ChunkCoordIntPair chunkPosition;
    std::vector<byte_t> serializedData;
};
