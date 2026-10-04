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
    explicit AnvilChunkLoader(const std::string &worldDir, bool readOnly = false);
    ~AnvilChunkLoader() override;

    Chunk *loadChunk(World *world, int_t x, int_t z,
                     ChunkLoadStatus *status = nullptr) override;
    // Whether loadChunk() would find data for this chunk: queued for writing
    // or already in its region file. Lets the streaming provider hand only
    // genuinely new terrain to the generation worker, which has no loader of
    // its own and would otherwise generate over a saved chunk.
    bool isChunkSaved(int_t x, int_t z);
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
    bool readOnly;
    std::vector<byte_t> readScratch;
    std::vector<byte_t> writeScratch;

    std::vector<AnvilChunkLoaderPending *> pendingSaves;
    std::unordered_set<ChunkCoordIntPair, ChunkCoordIntPairValueHash, ChunkCoordIntPairValueEqual> pendingCoordinates;
    // Sum of every pendingSaves entry's serializedData size. Read/written under
    // pendingMutex, exactly like the queue it measures, so the backpressure
    // flush in queueChunkToSave() sees the same accounting the pops do.
    std::size_t pendingBytes = 0;
    std::mutex pendingMutex;
};
