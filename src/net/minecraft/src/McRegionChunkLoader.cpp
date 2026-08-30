#include "platform/Log.h"
#include "McRegionChunkLoader.h"

#include "RegionFileCache.h"
#include "RegionFile.h"
#include "CompressedStreamTools.h"
#include "NBTTagCompound.h"
#include "ChunkLoader.h"
#include "Chunk.h"
#include "World.h"
#include "WorldInfo.h"
#include <sstream>
#include <cstdio>
#include <memory>

namespace
{
    const char *regionReadErrorName(RegionFile::ReadStatus status)
    {
        switch (status)
        {
        case RegionFile::ReadStatus::InvalidCoordinates: return "invalid coordinates";
        case RegionFile::ReadStatus::InvalidSector: return "invalid sector";
        case RegionFile::ReadStatus::InvalidLength: return "invalid payload length";
        case RegionFile::ReadStatus::TruncatedData: return "truncated payload";
        case RegionFile::ReadStatus::UnsupportedCompression: return "unsupported compression";
        case RegionFile::ReadStatus::DecompressionFailed: return "decompression failed";
        case RegionFile::ReadStatus::IoError: return "I/O error";
        default: return "unknown error";
        }
    }

    // Non-owning streams over the loader's persistent vectors.  NBT parsing and
    // writing are synchronous, so their backing storage remains valid for the
    // whole operation and does not need an allocation-bearing stringstream.
    class VectorInputBuf : public std::streambuf
    {
    public:
        explicit VectorInputBuf(std::vector<byte_t> &bytes)
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
        VectorInputBuf buffer;
    };

    class VectorOutputBuf : public std::streambuf
    {
    public:
        explicit VectorOutputBuf(std::vector<byte_t> &bytes) : bytes(bytes)
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
                bytes.insert(bytes.end(), reinterpret_cast<const byte_t *>(src),
                             reinterpret_cast<const byte_t *>(src) + count);
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
        VectorOutputBuf buffer;
    };
}

McRegionChunkLoader::McRegionChunkLoader(const std::string &dir) :
    worldDir(dir)
{
}

Chunk* McRegionChunkLoader::loadChunk(World *world, int_t x, int_t z,
                                     ChunkLoadStatus *status)
{
    if (!readChunkData(x, z, readScratch, status))
        return nullptr;
    return loadChunkFromData(world, x, z, readScratch, status);
}

bool McRegionChunkLoader::readChunkData(int_t x, int_t z, std::vector<byte_t> &data,
                                        ChunkLoadStatus *status)
{
    if (status != nullptr)
        *status = ChunkLoadStatus::Missing;

    std::shared_ptr<RegionFile> rf = RegionFileCache::acquireRegionFile(worldDir, x, z);
    RegionFile::ReadStatus readStatus = RegionFile::ReadStatus::Missing;
    if (!rf->getChunkData(x & 0x1f, z & 0x1f, data, &readStatus))
    {
        if (status != nullptr && readStatus != RegionFile::ReadStatus::Missing)
            *status = ChunkLoadStatus::ReadError;
        if (readStatus != RegionFile::ReadStatus::Missing)
            MC_LOG_INFO("chunk", "Chunk %d,%d exists but its region data could not be read (%s); preserving it\n",
                   x, z, regionReadErrorName(readStatus));
        return false;
    }

    if (status != nullptr)
        *status = ChunkLoadStatus::Loaded;
    return true;
}

Chunk* McRegionChunkLoader::loadChunkFromData(World *world, int_t x, int_t z,
                                               std::vector<byte_t> &data,
                                               ChunkLoadStatus *status)
{
    if (status != nullptr)
        *status = ChunkLoadStatus::ReadError;

    VectorInputStream is(data);

    std::unique_ptr<NBTTagCompound> nbttagcompound;
    try
    {
        nbttagcompound.reset(CompressedStreamTools::readCompound(is));
    }
    catch (...)
    {
        MC_LOG_ERROR("chunk", "Invalid NBT in chunk %d,%d; preserving region data\n", x, z);
        return nullptr;
    }
    if (!nbttagcompound || !nbttagcompound->hasKey("Level"))
    {
        MC_LOG_WARN("chunk", "Chunk file at %d,%d is missing level data, skipping\n", x, z);
        return nullptr;
    }

    NBTTagCompound *level = nbttagcompound->getCompoundTag("Level");
    if (level == nullptr || !level->hasKey("Blocks"))
    {
        MC_LOG_ERROR("chunk", "Chunk file at %d,%d is missing block data, skipping\n", x, z);
        return nullptr;
    }

    Chunk *chunk = ChunkLoader::loadChunkIntoWorldFromCompound(world, level);

    if (chunk != nullptr && !chunk->isAtLocation(x, z))
    {
        // Do not "repair" a wrong-location region chunk by changing only xPos/zPos.
        // Its block array belongs to the other coordinates; relocating it creates
        // a valid-looking duplicate chunk and the next save makes the corruption
        // permanent. Treat this exactly like an unreadable region entry so the
        // provider preserves the file instead of regenerating/overwriting it.
        MC_LOG_ERROR("chunk", "Chunk file at %d,%d is in the wrong location; rejecting it. "
               "(Expected %d, %d, got %d, %d) Region data preserved.\n",
               x, z, x, z, chunk->xPosition, chunk->zPosition);
        delete chunk;
        return nullptr;
    }

    if (chunk != nullptr)
    {
        // Java McRegionChunkLoader calls Chunk.func_25124_i() here.  In this
        // port that method is remapBlocks(); onChunkLoadData() is func_4143_d()
        // and belongs to ChunkProviderLoadOrGenerate after the chunk is installed.
        chunk->remapBlocks();
        if (status != nullptr)
            *status = ChunkLoadStatus::Loaded;
    }
    return chunk;
}

void McRegionChunkLoader::saveChunk(World *world, Chunk *chunk)
{
    world->checkSessionLock();

    try
    {
        std::unique_ptr<NBTTagCompound> nbttagcompound(new NBTTagCompound());
        NBTTagCompound *nbttagcompound1 = new NBTTagCompound();
        nbttagcompound->setTag("Level", nbttagcompound1);
        ChunkLoader::storeChunkInCompound(chunk, world, nbttagcompound1);

        // Serialize straight into reusable loader-owned scratch.  This is the
        // Wii equivalent of the PS2 port's fixed/bounded I/O staging: no
        // ostringstream growth, no string copy, and no per-save heap churn.
        VectorOutputStream os(writeScratch);
        CompressedStreamTools::writeCompound(nbttagcompound.get(), os);

        // Write to region file
        std::shared_ptr<RegionFile> rf = RegionFileCache::acquireRegionFile(
            worldDir, chunk->xPosition, chunk->zPosition);
        rf->write(chunk->xPosition & 0x1f, chunk->zPosition & 0x1f,
                  writeScratch.data(), (int_t)writeScratch.size());

        WorldInfo *worldinfo = world->getWorldInfo();
        worldinfo->setSizeOnDisk(worldinfo->getSizeOnDisk() +
            (long_t)rf->getSizeDelta());
    }
    catch (...)
    {
        // silently catch, matching Java behavior
    }
}

void McRegionChunkLoader::saveExtraChunkData(World *world, Chunk *chunk)
{
}

void McRegionChunkLoader::addRandomArmor()
{
}

void McRegionChunkLoader::saveExtraData()
{
    // Flush and close cached region files on full saves / world shutdown.
    // RegionFile::write() intentionally avoids flushing every individual chunk
    // to prevent gameplay stutter; this keeps the durable sync point here.
    RegionFileCache::clearCache();
}
