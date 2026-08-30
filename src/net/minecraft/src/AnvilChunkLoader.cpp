#include "AnvilChunkLoader.h"

#include <algorithm>
#include <memory>
#include <streambuf>
#include <stdexcept>
#include <utility>

#include "platform/Log.h"
#include "Block.h"
#include "AnvilChunkLoaderPending.h"
#include "Chunk.h"
#include "CompressedStreamTools.h"
#include "Entity.h"
#include "EntityList.h"
#include "ExtendedBlockStorage.h"
#include "NBTTagCompound.h"
#include "NBTTagList.h"
#include "NextTickListEntry.h"
#include "RegionFile.h"
#include "RegionFileCache.h"
#include "TileEntity.h"
#include "ThreadedFileIOBase.h"
#include "World.h"
#include "WorldHeight.h"
#include "java/Arithmetic.h"
#include "WorldInfo.h"
#ifdef PS2_PLATFORM
#include "ps2/storage/save/Ps2SaveFileSystem.h"
#endif

namespace
{
static bool isStorageDisabledPath(const std::string &path)
{
#ifdef PS2_PLATFORM
    return Ps2SaveFileSystem::isDisabledPath(path);
#else
    (void)path;
    return false;
#endif
}

class VectorInputBuffer : public std::streambuf
{
public:
    explicit VectorInputBuffer(std::vector<byte_t> &bytes)
    {
        char *begin = reinterpret_cast<char *>(bytes.data());
        setg(begin, begin, begin + bytes.size());
    }
};

class VectorInputStream : public std::istream
{
public:
    explicit VectorInputStream(std::vector<byte_t> &bytes)
        : std::istream(nullptr), buffer(bytes)
    {
        rdbuf(&buffer);
    }
private:
    VectorInputBuffer buffer;
};

class VectorOutputBuffer : public std::streambuf
{
public:
    explicit VectorOutputBuffer(std::vector<byte_t> &bytes) : bytes(bytes)
    {
        bytes.clear();
    }

protected:
    int_type overflow(int_type ch) override
    {
        if (traits_type::eq_int_type(ch, traits_type::eof()))
            return traits_type::not_eof(ch);
        bytes.push_back((byte_t)ch);
        return ch;
    }

    std::streamsize xsputn(const char *src, std::streamsize count) override
    {
        if (count > 0)
        {
            bytes.insert(bytes.end(), reinterpret_cast<const byte_t *>(src),
                         reinterpret_cast<const byte_t *>(src) + count);
        }
        return count;
    }

private:
    std::vector<byte_t> &bytes;
};

class VectorOutputStream : public std::ostream
{
public:
    explicit VectorOutputStream(std::vector<byte_t> &bytes)
        : std::ostream(nullptr), buffer(bytes)
    {
        rdbuf(&buffer);
    }
private:
    VectorOutputBuffer buffer;
};
}

AnvilChunkLoader::AnvilChunkLoader(const std::string &worldDir)
    : worldDir(worldDir), storageDisabled(isStorageDisabledPath(worldDir))
{
}

AnvilChunkLoader::~AnvilChunkLoader()
{
    ThreadedFileIOBase::threadedIOInstance.waitForFinish();
    std::lock_guard<std::mutex> guard(pendingMutex);
    for (AnvilChunkLoaderPending *pending : pendingSaves)
        delete pending;
    pendingSaves.clear();
    pendingCoordinates.clear();
}

Chunk *AnvilChunkLoader::loadChunk(World *world, int_t x, int_t z, ChunkLoadStatus *status)
{
    if (status != nullptr)
        *status = ChunkLoadStatus::Missing;

    try
    {
        const ChunkCoordIntPair position(x, z);
        std::vector<byte_t> pendingBytes;
        if (copyPendingChunkData(position, pendingBytes))
        {
            VectorInputStream pendingStream(pendingBytes);
            std::unique_ptr<NBTTagCompound> pending(CompressedStreamTools::readCompound(pendingStream));
            return loadChunkFromCompound(world, x, z, pending.get(), status);
        }

        if (storageDisabled)
            return nullptr;

        std::shared_ptr<RegionFile> region = RegionFileCache::acquireRegionFile(
            worldDir, x, z, RegionFileCache::Format::Anvil);
        RegionFile::ReadStatus readStatus = RegionFile::ReadStatus::Missing;
        if (!region->getChunkData(x & 31, z & 31, readScratch, &readStatus))
        {
            if (status != nullptr && readStatus != RegionFile::ReadStatus::Missing)
                *status = ChunkLoadStatus::ReadError;
            return nullptr;
        }

        VectorInputStream stream(readScratch);
        std::unique_ptr<NBTTagCompound> root(CompressedStreamTools::readCompound(stream));
        return loadChunkFromCompound(world, x, z, root.get(), status);
    }
    catch (const std::exception &e)
    {
        MC_LOG_ERROR("chunk", "Invalid Anvil chunk %d,%d: %s\n", x, z, e.what());
        if (status != nullptr)
            *status = ChunkLoadStatus::ReadError;
        return nullptr;
    }
    catch (...)
    {
        MC_LOG_ERROR("chunk", "Invalid Anvil chunk %d,%d\n", x, z);
        if (status != nullptr)
            *status = ChunkLoadStatus::ReadError;
        return nullptr;
    }
}

Chunk *AnvilChunkLoader::loadChunkFromCompound(World *world, int_t expectedX, int_t expectedZ,
                                                NBTTagCompound *root, ChunkLoadStatus *status)
{
    if (root == nullptr || !root->hasKey("Level"))
        return nullptr;

    NBTTagCompound *level = root->getCompoundTag("Level");
    if (level == nullptr || !level->hasKey("Sections"))
        return nullptr;

    Chunk *chunk = readChunkFromLevel(world, level);
    if (chunk == nullptr)
        return nullptr;

    if (!chunk->isAtLocation(expectedX, expectedZ))
    {
        MC_LOG_WARN("chunk", "Anvil chunk %d,%d belongs to %d,%d; relocating chunk coordinates\n",
                    expectedX, expectedZ, chunk->xPosition, chunk->zPosition);
        level->setInteger("xPos", expectedX);
        level->setInteger("zPos", expectedZ);
        delete chunk;
        chunk = readChunkFromLevel(world, level);
        if (chunk == nullptr)
        {
            if (status != nullptr)
                *status = ChunkLoadStatus::ReadError;
            return nullptr;
        }
    }

    chunk->removeUnknownBlocks();
    if (status != nullptr)
        *status = ChunkLoadStatus::Loaded;
    return chunk;
}

void AnvilChunkLoader::saveChunk(World *world, Chunk *chunk)
{
    if (world == nullptr || chunk == nullptr || storageDisabled)
        return;

    world->checkSessionLock();

    try
    {
        MC_LOG_DEBUG("save", "[chunk save] serialize begin %d,%d\n",
                     chunk->xPosition, chunk->zPosition);
        std::unique_ptr<NBTTagCompound> root(new NBTTagCompound());
        NBTTagCompound *level = new NBTTagCompound();
        root->setTag("Level", level);
        writeChunkToLevel(chunk, world, level);

        std::vector<byte_t> serialized;
#ifdef PS2_PLATFORM
        serialized.reserve(64 * 1024);
#endif
        VectorOutputStream stream(serialized);
        CompressedStreamTools::writeCompound(root.get(), stream);
        if (!stream.good())
            throw std::runtime_error("Failed to serialize Anvil chunk");

        // Drop the NBT object graph before the save waits in the worker queue.
        // A queued chunk now owns one contiguous byte vector instead of dozens
        // of separately allocated tag/list/array objects, which matters on the
        // PS2's fragmentation-sensitive heap while terrain is streaming.
        root.reset();
        queueChunkToSave(ChunkCoordIntPair(chunk->xPosition, chunk->zPosition), std::move(serialized));
        MC_LOG_DEBUG("save", "[chunk save] queued %d,%d\n",
                     chunk->xPosition, chunk->zPosition);
    }
    catch (const std::exception &e)
    {
        MC_LOG_ERROR("chunk", "Failed to queue Anvil chunk %d,%d: %s\n",
                     chunk->xPosition, chunk->zPosition, e.what());
    }
    catch (...)
    {
        MC_LOG_ERROR("chunk", "Failed to queue Anvil chunk %d,%d: unknown exception\n",
                     chunk->xPosition, chunk->zPosition);
    }
}

void AnvilChunkLoader::writeChunkToLevel(Chunk *chunk, World *world, NBTTagCompound *level)
{
    level->setInteger("xPos", chunk->xPosition);
    level->setInteger("zPos", chunk->zPosition);
    level->setLong("LastUpdate", world->getWorldTime());
    level->setIntArray("HeightMap", std::vector<int_t>(chunk->heightMap, chunk->heightMap + 256));
    level->setBoolean("TerrainPopulated", chunk->isTerrainPopulated);

    NBTTagList *sections = new NBTTagList();
    ExtendedBlockStorage **storage = chunk->getBlockStorageArray();
    for (int_t sectionIndex = 0; sectionIndex < Chunk::SECTION_COUNT; ++sectionIndex)
    {
        ExtendedBlockStorage *section = storage[sectionIndex];
        if (section == nullptr || section->func_48700_f() == 0)
            continue;

        NBTTagCompound *tag = new NBTTagCompound();
        tag->setByte("Y", (byte_t)(section->getYLocation() >> 4));
        tag->setByteArray("Blocks", section->func_48692_g());
        if (const NibbleArray *add = section->getBlockMSBArray())
            tag->setByteArray("Add", add->data);
        tag->setByteArray("Data", section->func_48697_j().data);
        tag->setByteArray("SkyLight", section->getSkylightArray().data);
        tag->setByteArray("BlockLight", section->getBlocklightArray().data);
        sections->appendTag(tag);
    }
    level->setTag("Sections", sections);
    level->setByteArray("Biomes", chunk->getBiomeArray());

    chunk->hasEntities = false;
    NBTTagList *entitiesTag = new NBTTagList();
    for (int_t sectionIndex = 0; sectionIndex < Chunk::SECTION_COUNT; ++sectionIndex)
    {
        for (Entity *entity : chunk->entities[sectionIndex])
        {
            if (entity == nullptr)
                continue;

            // Chunk entity buckets are non-owning indexes. Java's GC made a stale
            // reference survivable; in C++ it is a use-after-free as soon as the
            // save path calls a virtual method on it. Validate the raw address
            // against World's authoritative live list before dereferencing it.
            if (!world->isLoadedEntityPointer(entity))
            {
                MC_LOG_ERROR("chunk",
                    "Skipping stale entity pointer while saving chunk %d,%d section=%d ptr=%p\n",
                    chunk->xPosition, chunk->zPosition, sectionIndex, (void *)entity);
                continue;
            }

            // A live entity indexed in the wrong chunk/section is also evidence
            // of a stale duplicate. Serialize only the authoritative bucket.
            if (!entity->addedToChunk || entity->chunkCoordX != chunk->xPosition ||
                entity->chunkCoordZ != chunk->zPosition || entity->chunkCoordY != sectionIndex)
            {
                MC_LOG_WARN("chunk",
                    "Skipping misindexed entity id=%d saveChunk=%d,%d/%d entityChunk=%d,%d/%d added=%d\n",
                    entity->entityId, chunk->xPosition, chunk->zPosition, sectionIndex,
                    entity->chunkCoordX, entity->chunkCoordZ, entity->chunkCoordY,
                    entity->addedToChunk ? 1 : 0);
                continue;
            }

            chunk->hasEntities = true;
            NBTTagCompound *entityTag = new NBTTagCompound();
            if (entity->addEntityID(entityTag))
                entitiesTag->appendTag(entityTag);
            else
                delete entityTag;
        }
    }
    level->setTag("Entities", entitiesTag);

    NBTTagList *tileEntitiesTag = new NBTTagList();
    for (const ChunkPosition &position : chunk->chunkTileEntityOrder.valuesInIterationOrder())
    {
        auto it = chunk->chunkTileEntityMap.find(position);
        if (it == chunk->chunkTileEntityMap.end() || it->second == nullptr)
            continue;
        TileEntity *tileEntity = it->second;
        if (!world->isLoadedTileEntityPointer(tileEntity))
        {
            MC_LOG_ERROR("chunk",
                "Skipping stale tile entity pointer while saving chunk %d,%d ptr=%p\n",
                chunk->xPosition, chunk->zPosition, (void *)tileEntity);
            continue;
        }
        if (JavaArithmetic::intShr(tileEntity->xCoord, 4) != chunk->xPosition ||
            JavaArithmetic::intShr(tileEntity->zCoord, 4) != chunk->zPosition ||
            tileEntity->yCoord < 0 || tileEntity->yCoord >= WorldHeight::HEIGHT)
        {
            MC_LOG_WARN("chunk",
                "Skipping misindexed tile entity saveChunk=%d,%d tile=%d,%d,%d ptr=%p\n",
                chunk->xPosition, chunk->zPosition, tileEntity->xCoord, tileEntity->yCoord,
                tileEntity->zCoord, (void *)tileEntity);
            continue;
        }
        NBTTagCompound *tileTag = new NBTTagCompound();
        tileEntity->writeToNBT(tileTag);
        tileEntitiesTag->appendTag(tileTag);
    }
    level->setTag("TileEntities", tileEntitiesTag);

    std::vector<NextTickListEntry *> pending = world->getPendingBlockUpdates(chunk, false);
    if (!pending.empty())
    {
        const long_t now = world->getWorldTime();
        NBTTagList *ticks = new NBTTagList();
        for (NextTickListEntry *entry : pending)
        {
            if (entry == nullptr)
                continue;
            NBTTagCompound *tick = new NBTTagCompound();
            tick->setInteger("i", entry->blockID);
            tick->setInteger("x", entry->xCoord);
            tick->setInteger("y", entry->yCoord);
            tick->setInteger("z", entry->zCoord);
            tick->setInteger("t", JavaArithmetic::longToInt(JavaArithmetic::longSub(entry->scheduledTime, now)));
            ticks->appendTag(tick);
        }
        level->setTag("TileTicks", ticks);
    }
}

Chunk *AnvilChunkLoader::readChunkFromLevel(World *world, NBTTagCompound *level)
{
    const int_t x = level->getInteger("xPos");
    const int_t z = level->getInteger("zPos");
    std::unique_ptr<Chunk> chunk(new Chunk(world, x, z));
    chunk->isTerrainPopulated = level->getBoolean("TerrainPopulated");

    std::vector<int_t> heightMap = level->getIntArray("HeightMap");
    if (heightMap.size() == 256)
        std::copy(heightMap.begin(), heightMap.end(), chunk->heightMap);

    NBTTagList *sections = level->getTagList("Sections");
    ExtendedBlockStorage **storage = chunk->getBlockStorageArray();
    for (int_t index = 0; index < sections->tagCount(); ++index)
    {
        NBTTagCompound *tag = dynamic_cast<NBTTagCompound *>(sections->tagAt(index));
        if (tag == nullptr)
            continue;

        const int_t sectionY = ((int_t)tag->getByte("Y")) & 0xff;
        if (sectionY < 0 || sectionY >= Chunk::SECTION_COUNT)
            continue;

        std::unique_ptr<ExtendedBlockStorage> section(new ExtendedBlockStorage(sectionY << 4));
        section->setBlockLSBArray(tag->getByteArray("Blocks"));
        if (tag->hasKey("Add"))
            section->setBlockMSBArray(std::unique_ptr<NibbleArray>(new NibbleArray(tag->getByteArray("Add"), 4)));
        section->setBlockMetadataArray(NibbleArray(tag->getByteArray("Data"), 4));
        section->setSkylightArray(NibbleArray(tag->getByteArray("SkyLight"), 4));
        section->setBlocklightArray(NibbleArray(tag->getByteArray("BlockLight"), 4));
        section->func_48708_d();
        delete storage[sectionY];
        storage[sectionY] = section.release();
    }

    if (level->hasKey("Biomes"))
        chunk->setBiomeArray(level->getByteArray("Biomes"));

    if (heightMap.size() != 256)
        chunk->generateSkylightMap();

    if (level->hasKey("Entities"))
    {
        NBTTagList *entities = level->getTagList("Entities");
        for (int_t index = 0; index < entities->tagCount(); ++index)
        {
            NBTTagCompound *tag = dynamic_cast<NBTTagCompound *>(entities->tagAt(index));
            if (tag == nullptr)
                continue;
            Entity *entity = EntityList::createEntityFromNBT(tag, world);
            chunk->hasEntities = true;
            if (entity != nullptr)
                chunk->addEntity(entity);
        }
    }

    if (level->hasKey("TileEntities"))
    {
        NBTTagList *tileEntities = level->getTagList("TileEntities");
        for (int_t index = 0; index < tileEntities->tagCount(); ++index)
        {
            NBTTagCompound *tag = dynamic_cast<NBTTagCompound *>(tileEntities->tagAt(index));
            if (tag == nullptr)
                continue;
            TileEntity *tile = TileEntity::createAndLoadEntity(tag);
            if (tile != nullptr)
                chunk->addTileEntity(tile);
        }
    }

    if (level->hasKey("TileTicks"))
    {
        NBTTagList *ticks = level->getTagList("TileTicks");
        for (int_t index = 0; index < ticks->tagCount(); ++index)
        {
            NBTTagCompound *tag = dynamic_cast<NBTTagCompound *>(ticks->tagAt(index));
            if (tag == nullptr)
                continue;
            world->scheduleBlockUpdateFromLoad(
                tag->getInteger("x"), tag->getInteger("y"), tag->getInteger("z"),
                tag->getInteger("i"), tag->getInteger("t"));
        }
    }

    return chunk.release();
}

void AnvilChunkLoader::saveExtraChunkData(World *world, Chunk *chunk)
{
    (void)world;
    (void)chunk;
}

void AnvilChunkLoader::addRandomArmor()
{
    chunkTick();
}

void AnvilChunkLoader::chunkTick()
{
}

bool AnvilChunkLoader::copyPendingChunkData(const ChunkCoordIntPair &position, std::vector<byte_t> &out)
{
    std::lock_guard<std::mutex> guard(pendingMutex);
    if (pendingCoordinates.find(position) == pendingCoordinates.end())
        return false;

    for (AnvilChunkLoaderPending *pending : pendingSaves)
    {
        if (pending != nullptr && pending->chunkPosition == position && !pending->serializedData.empty())
        {
            out = pending->serializedData;
            return true;
        }
    }
    return false;
}

void AnvilChunkLoader::queueChunkToSave(const ChunkCoordIntPair &position, std::vector<byte_t> serialized)
{
    if (serialized.empty() || storageDisabled)
        return;

    bool queueWorker = false;
    {
        std::lock_guard<std::mutex> guard(pendingMutex);
        if (pendingCoordinates.find(position) != pendingCoordinates.end())
        {
            for (AnvilChunkLoaderPending *pending : pendingSaves)
            {
                if (pending != nullptr && pending->chunkPosition == position)
                {
                    if (pending->serializedData.capacity() >= serialized.size())
                    {
                        pending->serializedData.assign(serialized.begin(), serialized.end());
                    }
                    else
                    {
                        pending->serializedData.swap(serialized);
                    }
                    return;
                }
            }
        }

        pendingSaves.push_back(new AnvilChunkLoaderPending(position, std::move(serialized)));
        pendingCoordinates.insert(position);
        queueWorker = true;
    }

    if (queueWorker)
        ThreadedFileIOBase::threadedIOInstance.queueIO(this);
}

void AnvilChunkLoader::writePendingChunk(AnvilChunkLoaderPending *pending)
{
    if (pending == nullptr || pending->serializedData.empty() || storageDisabled)
        return;

    MC_LOG_DEBUG("save", "[chunk save] write begin %d,%d\n",
                 pending->chunkPosition.chunkXPos, pending->chunkPosition.chunkZPos);

    std::shared_ptr<RegionFile> region = RegionFileCache::acquireRegionFile(
        worldDir, pending->chunkPosition.chunkXPos, pending->chunkPosition.chunkZPos,
        RegionFileCache::Format::Anvil);
    region->write(pending->chunkPosition.chunkXPos & 31, pending->chunkPosition.chunkZPos & 31,
                  pending->serializedData.data(), (int_t)pending->serializedData.size());
    MC_LOG_DEBUG("save", "[chunk save] write end %d,%d bytes=%u\n",
                 pending->chunkPosition.chunkXPos, pending->chunkPosition.chunkZPos,
                 (unsigned)pending->serializedData.size());
}

bool AnvilChunkLoader::writeNextIO()
{
    AnvilChunkLoaderPending *pending = nullptr;
    {
        std::lock_guard<std::mutex> guard(pendingMutex);
        if (pendingSaves.empty())
            return false;
        pending = pendingSaves.front();
        pendingSaves.erase(pendingSaves.begin());
        if (pending != nullptr)
            pendingCoordinates.erase(pending->chunkPosition);
    }

    std::unique_ptr<AnvilChunkLoaderPending> ownedPending(pending);
    try
    {
        writePendingChunk(pending);
    }
    catch (const std::exception &e)
    {
        if (pending != nullptr)
        {
            MC_LOG_ERROR("chunk", "Failed to save queued Anvil chunk %d,%d: %s\n",
                         pending->chunkPosition.chunkXPos, pending->chunkPosition.chunkZPos, e.what());
        }
        else
        {
            MC_LOG_ERROR("chunk", "Failed to save queued Anvil chunk: %s\n", e.what());
        }
    }
    return true;
}

void AnvilChunkLoader::saveExtraData()
{
    MC_LOG_DEBUG("save", "[chunk save] flush wait begin\n");
    ThreadedFileIOBase::threadedIOInstance.waitForFinish();
    MC_LOG_DEBUG("save", "[chunk save] flush wait end; region cache flush begin\n");
    RegionFileCache::clearCache();
    MC_LOG_DEBUG("save", "[chunk save] region cache flush end\n");
}
