#pragma once

#include "java/Type.h"

#include "IChunkLoader.h"
#include <string>
#include <vector>

class World;
class Chunk;

// net.minecraft.src.McRegionChunkLoader
class McRegionChunkLoader : public IChunkLoader
{
public:
    explicit McRegionChunkLoader(const std::string &worldDir);

    Chunk* loadChunk(World *world, int_t x, int_t z,
                     ChunkLoadStatus *status = nullptr) override;
    bool readChunkData(int_t x, int_t z, std::vector<byte_t> &data,
                       ChunkLoadStatus *status = nullptr);
    Chunk* loadChunkFromData(World *world, int_t x, int_t z,
                             std::vector<byte_t> &data,
                             ChunkLoadStatus *status = nullptr);
    void saveChunk(World *world, Chunk *chunk) override;
    void saveExtraChunkData(World *world, Chunk *chunk) override;
    void addRandomArmor() override;
    void saveExtraData() override;

private:
    std::string worldDir;
    // Synchronous I/O scratch, retained for the loader lifetime.  A beta
    // chunk's NBT typically fits in 85KB; reusing these removes the repeated
    // grow/free cycles that fragment newlib's heap during world streaming.
    std::vector<byte_t> readScratch;
    std::vector<byte_t> writeScratch;
};
