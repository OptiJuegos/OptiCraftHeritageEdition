#pragma once

#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

#include "ChunkCoordIntPair.h"
#include "IChunkLoader.h"
#include "IThreadedFileIO.h"

class AnvilChunkLoaderPending;
class NBTTagCompound;

// net.minecraft.src.AnvilChunkLoader
class AnvilChunkLoader : public IChunkLoader, public IThreadedFileIO
{
public:
    explicit AnvilChunkLoader(const std::string &worldDir);
    ~AnvilChunkLoader() override;

    Chunk *loadChunk(World *world, int_t x, int_t z,
                     ChunkLoadStatus *status = nullptr) override;
    void saveChunk(World *world, Chunk *chunk) override;
    void saveExtraChunkData(World *world, Chunk *chunk) override;
    void addRandomArmor() override;
    void chunkTick() override;
    void saveExtraData() override;
    bool writeNextIO() override;

private:
    Chunk *loadChunkFromCompound(World *world, int_t expectedX, int_t expectedZ,
                                 NBTTagCompound *root, ChunkLoadStatus *status);
    Chunk *readChunkFromLevel(World *world, NBTTagCompound *level);
    void writeChunkToLevel(Chunk *chunk, World *world, NBTTagCompound *level);
    void queueChunkToSave(const ChunkCoordIntPair &position, std::vector<byte_t> serialized);
    void writePendingChunk(AnvilChunkLoaderPending *pending);
    bool copyPendingChunkData(const ChunkCoordIntPair &position, std::vector<byte_t> &out);

    std::string worldDir;
    bool storageDisabled;
    std::vector<byte_t> readScratch;
    std::vector<byte_t> writeScratch;

    std::vector<AnvilChunkLoaderPending *> pendingSaves;
    std::unordered_set<ChunkCoordIntPair, ChunkCoordIntPairValueHash, ChunkCoordIntPairValueEqual> pendingCoordinates;
    std::mutex pendingMutex;
};
