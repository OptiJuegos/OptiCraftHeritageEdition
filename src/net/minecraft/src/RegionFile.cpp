#include "RegionFile.h"
#include "platform/Log.h"
#include "java/Arithmetic.h"
#include "platform/PlatformConfig.h"

#include <cstring>
#include <ctime>
#include <limits>
#include <stdexcept>
#include <cstdio>
#include <zlib.h>
#if PLATFORM_REGION_WHOLE_FILE_BUFFER
#include "platform/Storage.h"
#endif

const byte_t RegionFile::emptySector[4096] = {};

// std::iostream so the same helpers work with a native random-access file or
// the whole-file in-memory backing selected by the storage capability.
static void writeInt32BE(std::iostream &f, int_t v)
{
    const std::uint32_t bits = static_cast<std::uint32_t>(v);
    byte_t b[4];
    b[0] = JavaArithmetic::byteFromBits(static_cast<ubyte_t>(bits >> 24));
    b[1] = JavaArithmetic::byteFromBits(static_cast<ubyte_t>(bits >> 16));
    b[2] = JavaArithmetic::byteFromBits(static_cast<ubyte_t>(bits >> 8));
    b[3] = JavaArithmetic::byteFromBits(static_cast<ubyte_t>(bits));
    f.write(reinterpret_cast<char *>(b), 4);
}

static int_t readInt32BE(std::iostream &f)
{
    byte_t b[4] = {};
    f.read(reinterpret_cast<char *>(b), 4);
    if (f.gcount() != 4)
        throw std::runtime_error("Unexpected end of region file");
    const std::uint32_t bits = (static_cast<std::uint32_t>(static_cast<unsigned char>(b[0])) << 24)
                             | (static_cast<std::uint32_t>(static_cast<unsigned char>(b[1])) << 16)
                             | (static_cast<std::uint32_t>(static_cast<unsigned char>(b[2])) << 8)
                             | static_cast<std::uint32_t>(static_cast<unsigned char>(b[3]));
    return JavaArithmetic::intFromBits(bits);
}

static void writeByte(std::iostream &f, byte_t b)
{
    f.write((char*)&b, 1);
}

static byte_t readByte(std::iostream &f)
{
    byte_t b = 0;
    f.read((char*)&b, 1);
    if (f.gcount() != 1)
        throw std::runtime_error("Unexpected end of region file");
    return b;
}

static long_t fileLength(std::iostream &f)
{
    f.clear();
    auto pos = f.tellg();
    f.seekg(0, std::ios::end);
    const auto end = f.tellg();
    if (end < 0)
        throw std::runtime_error("Unable to determine region file length");
    long_t len = static_cast<long_t>(end);
    f.clear();
    f.seekg(pos);
    return len;
}

// Streaming inflate into a bounded, gradually grown buffer. The old code
// committed a flat 1MB vector per chunk read; on the PS2's 32MB heap that
// single spike could exhaust the heap mid-frame. A beta chunk's raw NBT is
// ~85KB, so the caps below stay generous. windowBits selects the container:
// 16+MAX_WBITS = gzip (version 1 chunks), MAX_WBITS = zlib (version 2).
static bool inflateChunkData(const byte_t *src, size_t srcLen, int windowBits,
                             std::vector<byte_t> &out)
{
#if PLATFORM_SMALL_REGION_SCRATCH
    const size_t maxOut   = 256 * 1024;
    const size_t startOut = 128 * 1024;
#else
    const size_t maxOut   = 1024 * 1024;
    const size_t startOut = 256 * 1024;
#endif
    out.clear();
    out.resize(startOut);

    z_stream zs{};
    if (inflateInit2(&zs, windowBits) != Z_OK)
        return false;

    zs.next_in  = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(src));
    zs.avail_in = (uInt)srcLen;

    for (;;)
    {
        zs.next_out  = reinterpret_cast<Bytef*>(out.data() + zs.total_out);
        zs.avail_out = (uInt)(out.size() - (size_t)zs.total_out);
        int status = inflate(&zs, Z_NO_FLUSH);
        if (status == Z_STREAM_END)
            break;
        // Only a full output buffer justifies growing; anything else (corrupt
        // sector, truncated input, over-cap chunk) fails instead of returning
        // partially inflated data.
        const bool outFull = (status == Z_OK || status == Z_BUF_ERROR) && zs.avail_out == 0;
        if (!outFull || out.size() >= maxOut)
        {
            inflateEnd(&zs);
            out.clear();
            return false;
        }
        size_t next = out.size() * 2;
        if (next > maxOut)
            next = maxOut;
        out.resize(next);
    }

    out.resize((size_t)zs.total_out);
    inflateEnd(&zs);
    return true;
}

RegionFile::RegionFile(const std::string &path) :
    filePath(path),
    sizeDelta(0)
{
    memset(offsets, 0, sizeof(offsets));
    memset(timestamps, 0, sizeof(timestamps));
    memset(entryValid, 0, sizeof(entryValid));

    try
    {
#if PLATFORM_REGION_WHOLE_FILE_BUFFER
        // Load the whole region through PlatformStorage into the
        // in-memory buffer; close() writes it back. A missing file just leaves the
        // buffer empty and the header-init below creates the sector tables.
        std::vector<unsigned char> existing;
        PlatformStorage::readFile(path, existing);
        if (!existing.empty())
            dataFile.write(reinterpret_cast<const char*>(existing.data()),
                           (std::streamsize)existing.size());
        dataFile.clear();
        dataFile.seekg(0);
        dataFile.seekp(0);
#else
        dataFile.open(path, std::ios::in | std::ios::out | std::ios::binary);
        if (!dataFile.is_open())
        {
            dataFile.open(path, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
        }
        if (!dataFile.is_open())
            throw std::runtime_error("Unable to open region file");
#endif

        long_t headerLength = fileLength(dataFile);
        if (headerLength < 8192)
        {
            dataFile.clear();
            if (headerLength < 4096)
            {
                dataFile.seekp(0);
                dataFile.write(reinterpret_cast<const char *>(emptySector), 4096);
                dataFile.write(reinterpret_cast<const char *>(emptySector), 4096);
                sizeDelta = JavaArithmetic::intAdd(sizeDelta, static_cast<int_t>(8192 - headerLength));
                headerLength = 8192;
            }
            else
            {
                dataFile.seekp(0, std::ios::end);
            }
            while (headerLength < 8192)
            {
                dataFile.write(reinterpret_cast<const char *>(emptySector), 4096);
                headerLength += 4096;
                sizeDelta = JavaArithmetic::intAdd(sizeDelta, 4096);
            }
            if (!dataFile)
                throw std::runtime_error("Unable to initialize region file header");
            dataFile.flush();
        }

        // Pad to 4096-byte boundary
        long_t len = fileLength(dataFile);
        if ((len & 4095) != 0)
        {
            dataFile.seekp(0, std::ios::end);
            int pad = (int)(4096 - (len & 4095));
            for (int i = 0; i < pad; i++)
                writeByte(dataFile, 0);
            dataFile.flush();
        }

        int sectorCount = (int)(fileLength(dataFile) / 4096);
        if (sectorCount < 2)
            throw std::runtime_error("Region file header is incomplete");
        sectorFree.assign(sectorCount, true);
        sectorOwner.assign(sectorCount, -1);
        sectorFree[0] = false; // offset table sector
        sectorFree[1] = false; // timestamp table sector
        sectorOwner[0] = -2;  // header / never allocatable
        sectorOwner[1] = -2;

        // Read and validate the offset table.
        //
        // A region entry owns a contiguous run of sectors.  The old port only
        // checked that each run ended inside the file, then marked it used.
        // If an already-corrupt .mcr contained two entries for the same sector,
        // both entries looked valid.  Saving either chunk later freed the run
        // before allocating its replacement, making the still-live entry point
        // at storage which could be given to another chunk.  The result on the
        // next load is the familiar "wrong location; relocating" message --
        // and, worse, it spreads the corruption on every save.
        //
        // Keep the first valid owner in physical/header order. Invalid entries
        // remain in the header so a read is reported as an error rather than as
        // a missing chunk. This prevents the provider from regenerating and
        // later overwriting data that may still be recoverable externally.
        dataFile.seekg(0);
        for (int i = 0; i < 1024; i++)
        {
            int_t v = readInt32BE(dataFile);
            offsets[i] = v;
            if (v == 0)
                continue;

            int start  = v >> 8;
            int length = v & 0xff;
            bool valid = start >= 2 && length > 0 &&
                         start + length <= (int)sectorFree.size();
            if (valid)
            {
                for (int s = 0; s < length; s++)
                {
                    if (sectorOwner[start + s] != -1)
                    {
                        valid = false;
                        break;
                    }
                }
            }

            if (!valid)
            {
                entryValid[i] = false;
                MC_LOG_WARN("chunk", "[region] invalid/overlapping sector entry %d in %s; preserving chunk data\n",
                            i, filePath.c_str());

                // Do not let later saves reuse the in-range portion of an
                // invalid entry.  Even though we cannot trust it enough to
                // load, those bytes may still be recoverable.  A successful
                // rewrite of this header entry will point elsewhere; after the
                // region is reopened these stale sectors become free normally.
                if (length > 0)
                {
                    int reserveStart = start < 2 ? 2 : start;
                    int reserveEnd = start + length;
                    if (reserveEnd > (int)sectorFree.size())
                        reserveEnd = (int)sectorFree.size();
                    if (reserveStart < 0)
                        reserveStart = 0;
                    for (int s = reserveStart; s < reserveEnd; s++)
                    {
                        sectorFree[s] = false;
                        if (sectorOwner[s] == -1)
                            sectorOwner[s] = -2;
                    }
                }
                continue;
            }

            entryValid[i] = true;
            for (int s = 0; s < length; s++)
            {
                sectorOwner[start + s] = i;
                sectorFree[start + s] = false;
            }
        }

        // Read timestamp table
        for (int i = 0; i < 1024; i++)
            timestamps[i] = readInt32BE(dataFile);

    }
    catch (...)
    {
        // Silently continue (matching Java behavior)
    }
}

RegionFile::~RegionFile()
{
    close();
}

int_t RegionFile::getSizeDelta()
{
    std::lock_guard<std::mutex> lock(mtx);
    int_t d = sizeDelta;
    sizeDelta = 0;
    return d;
}

bool RegionFile::outOfBounds(int_t x, int_t z)
{
    return x < 0 || x >= 32 || z < 0 || z >= 32;
}

int_t RegionFile::getOffset(int_t x, int_t z)
{
    return offsets[x + z * 32];
}

void RegionFile::setOffset(int_t x, int_t z, int_t value)
{
    offsets[x + z * 32] = value;
    dataFile.clear();
    dataFile.seekp((x + z * 32) * 4);
    writeInt32BE(dataFile, value);
}

void RegionFile::setTimestamp(int_t x, int_t z, int_t value)
{
    timestamps[x + z * 32] = value;
    dataFile.clear();
    dataFile.seekp(4096 + (x + z * 32) * 4);
    writeInt32BE(dataFile, value);
}

bool RegionFile::hasChunk(int_t x, int_t z)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (outOfBounds(x, z)) return false;
    return getOffset(x, z) != 0;
}

// Returns raw (inflated) chunk bytes, or empty on failure
bool RegionFile::getChunkData(int_t x, int_t z, std::vector<byte_t> &out,
                              ReadStatus *status)
{
    std::lock_guard<std::mutex> lock(mtx);
    out.clear();

    const auto fail = [status](ReadStatus failure)
    {
        if (status != nullptr)
            *status = failure;
        return false;
    };

    if (outOfBounds(x, z)) return fail(ReadStatus::InvalidCoordinates);

    const int_t entryIndex = x + z * 32;
    int_t offset = getOffset(x, z);
    if (offset == 0) return fail(ReadStatus::Missing);

    // A non-zero offset is not necessarily readable.  The constructor marks
    // out-of-range/overlapping header entries invalid; never follow one into
    // another chunk's sectors.
    if (!entryValid[entryIndex])
        return fail(ReadStatus::InvalidSector);

    int_t sectorStart  = offset >> 8;
    int_t sectorLength = offset & 0xff;

    if (sectorStart < 2 || sectorLength <= 0)
        return fail(ReadStatus::InvalidSector);
    if (sectorStart + sectorLength > (int_t)sectorFree.size())
        return fail(ReadStatus::InvalidSector);

    // Re-check ownership as a cheap invariant guard.  This catches an internal
    // allocator bookkeeping bug before it can turn into a wrong-chunk read.
    for (int_t s = 0; s < sectorLength; s++)
    {
        if (sectorOwner[sectorStart + s] != entryIndex)
            return fail(ReadStatus::InvalidSector);
    }

    try
    {
        dataFile.clear();
        dataFile.seekg(static_cast<long_t>(JavaArithmetic::intMul(sectorStart, 4096)));
        int_t dataLength = readInt32BE(dataFile);
        if (dataLength <= 0 || dataLength > 4096 * sectorLength)
            return fail(ReadStatus::InvalidLength);

        byte_t version = readByte(dataFile);

        ioScratch.resize((size_t)dataLength - 1u);
        dataFile.read((char*)ioScratch.data(), dataLength - 1);
        if (!dataFile || dataFile.gcount() != dataLength - 1)
            return fail(ReadStatus::TruncatedData);

        if (version == 1)
        {
            // GZIP
            if (!inflateChunkData(ioScratch.data(), ioScratch.size(), 16 + MAX_WBITS, out))
                return fail(ReadStatus::DecompressionFailed);
        }
        else if (version == 2)
        {
            // Deflate (zlib)
            if (!inflateChunkData(ioScratch.data(), ioScratch.size(), MAX_WBITS, out))
                return fail(ReadStatus::DecompressionFailed);
        }
        else
        {
            return fail(ReadStatus::UnsupportedCompression);
        }

        if (status != nullptr)
            *status = ReadStatus::Success;
        return true;
    }
    catch (...)
    {
        out.clear();
        return fail(ReadStatus::IoError);
    }
    out.clear();
    return fail(ReadStatus::IoError);
}

void RegionFile::write(int_t x, int_t z, const byte_t *rawData, int_t rawLength)
{
    std::lock_guard<std::mutex> lock(mtx);

    try
    {
        if (outOfBounds(x, z) || rawData == nullptr || rawLength <= 0)
            return;

        // Compress with zlib deflate.
        uLongf compLen = compressBound((uLong)rawLength);
        if (compLen > static_cast<uLongf>(std::numeric_limits<std::size_t>::max()))
            return;
        ioScratch.resize(static_cast<std::size_t>(compLen));
        // Console chunk writes run synchronously inside a world tick, so the
        // deflate level is frame time, not disk space. Level 1 is several times
        // faster than level 6 for roughly 10-15% larger sectors, which the
        // region format absorbs in sector rounding anyway. Both levels produce
        // ordinary zlib streams, so saves stay readable by every other build.
#if PLATFORM_FAST_REGION_COMPRESSION
        const int compressionLevel = Z_BEST_SPEED;
#else
        const int compressionLevel = Z_DEFAULT_COMPRESSION;
#endif
        int status = compress2(reinterpret_cast<Bytef *>(ioScratch.data()), &compLen,
                               reinterpret_cast<const Bytef *>(rawData),
                               (uLong)rawLength, compressionLevel);
        if (status != Z_OK)
            return;

        if (compLen > static_cast<uLongf>(std::numeric_limits<std::size_t>::max()))
            return;
        const std::size_t compressedBytes = static_cast<std::size_t>(compLen);
        if (compressedBytes > std::numeric_limits<std::size_t>::max() - 5u)
            return;
        ioScratch.resize(compressedBytes);

        const std::size_t sectorNeededSize = (compressedBytes + 5u) / 4096u + 1u;
        if (sectorNeededSize >= 256u)
            return;
        const int_t sectorNeeded = static_cast<int_t>(sectorNeededSize);

        const int_t entryIndex = x + z * 32;
        const int_t existingOffset = getOffset(x, z);
        const int_t existingSector = existingOffset >> 8;
        const int_t existingLength = existingOffset & 0xff;

        // Trust an old allocation only if the constructor validated the header
        // AND every sector is still owned by this exact chunk entry.
        bool existingOwned = existingOffset != 0 && entryValid[entryIndex] &&
                             existingSector >= 2 && existingLength > 0 &&
                             existingSector + existingLength <= (int_t)sectorOwner.size();
        if (existingOwned)
        {
            for (int_t i = 0; i < existingLength; i++)
            {
                if (sectorOwner[existingSector + i] != entryIndex)
                {
                    existingOwned = false;
                    break;
                }
            }
        }

        if (existingOffset != 0 && !existingOwned)
        {
            // Never free or overwrite sectors from an invalid/overlapping entry.
            // They may belong to another valid chunk.  Allocate fresh storage
            // below and repair only this header slot.
            MC_LOG_INFO("chunk", "[region] rewriting unsafe entry %d (%d,%d) in %s using fresh sectors\n",
                        entryIndex, x, z, filePath.c_str());
            entryValid[entryIndex] = false;
        }

        if (existingOwned && existingLength == sectorNeeded)
        {
            // Same-size rewrite is safe in place because ownership is proven.
            writeSector(existingSector, ioScratch.data(), (int_t)ioScratch.size());
        }
        else
        {
            // Copy-on-write for resized chunks.  Crucially, do NOT free the old
            // run before the replacement has been written and published in the
            // header.  If an I/O failure happens midway, the old header/data pair
            // is still intact instead of pointing at sectors already reused by
            // another chunk.
            int_t runStart = -1;
            int_t runLen = 0;
            for (int_t i = 2; i < (int_t)sectorFree.size(); i++)
            {
                if (sectorFree[i])
                {
                    if (runLen == 0)
                        runStart = i;
                    ++runLen;
                    if (runLen >= sectorNeeded)
                        break;
                }
                else
                {
                    runLen = 0;
                    runStart = -1;
                }
            }

            if (runLen < sectorNeeded)
            {
                // Grow the file.  Keep sectorOwner exactly in sync with
                // sectorFree so later ownership checks remain reliable.
                dataFile.clear();
                dataFile.seekp(0, std::ios::end);
                runStart = (int_t)sectorFree.size();
                for (int_t i = 0; i < sectorNeeded; i++)
                {
                    dataFile.write((char*)emptySector, 4096);
                    sectorFree.push_back(false);
                    sectorOwner.push_back(entryIndex);
                }
                sizeDelta = JavaArithmetic::intAdd(sizeDelta, JavaArithmetic::intMul(4096, sectorNeeded));
            }

            // Write the replacement before changing the on-disk header.
            writeSector(runStart, ioScratch.data(), (int_t)ioScratch.size());
            if (!dataFile)
            {
                // The old allocation is still untouched/published.  For a grown
                // run the appended sectors simply remain unreachable garbage and
                // will be reclaimed on next open if the stream recovers.
                return;
            }

            // If the run came from existing free space, claim it now.
            if (runLen >= sectorNeeded)
            {
                for (int_t i = 0; i < sectorNeeded; i++)
                {
                    sectorFree[runStart + i] = false;
                    sectorOwner[runStart + i] = entryIndex;
                }
            }

            setOffset(x, z, JavaArithmetic::intShl(runStart, 8) | sectorNeeded);
            if (!dataFile)
                return;
            entryValid[entryIndex] = true;

            // Only after the new header is published may the previous allocation
            // become reusable, and only sectors proven to belong to this entry.
            if (existingOwned)
            {
                for (int_t i = 0; i < existingLength; i++)
                {
                    const int_t s = existingSector + i;
                    if (s >= 2 && s < (int_t)sectorOwner.size() &&
                        sectorOwner[s] == entryIndex)
                    {
                        sectorOwner[s] = -1;
                        sectorFree[s] = true;
                    }
                }
            }
        }

        setTimestamp(x, z, JavaArithmetic::longToInt(static_cast<long_t>(time(nullptr))));
        if (!dataFile)
            return;

        dirty = true;
#if PLATFORM_REGION_WHOLE_FILE_BUFFER
        flushed = false;
#endif
        // Do not flush on every chunk write during gameplay. The close()/
        // RegionFileCache::clearCache() path still flushes on save-all/exit, while
        // avoiding a disk-sync spike for every incremental chunk eviction.
    }
    catch (...)
    {
        // Keep the previous header/allocation whenever possible.  Most writes
        // above are copy-on-write specifically so an exception does not free the
        // only known-good sector run.
    }
}

void RegionFile::writeSector(int_t sectorNum, const byte_t *data, int_t length)
{
    dataFile.clear();
    dataFile.seekp(static_cast<long_t>(JavaArithmetic::intMul(sectorNum, 4096)));
    writeInt32BE(dataFile, length + 1);
    writeByte(dataFile, 2); // version 2 = zlib deflate
    dataFile.write((char*)data, length);
}

void RegionFile::close()
{
#if PLATFORM_REGION_WHOLE_FILE_BUFFER
    // Write the whole in-memory region back through PlatformStorage.
    std::lock_guard<std::mutex> lock(mtx);
    if (flushed)
        return;
    if (!dirty)
    {
        flushed = true;
        MC_LOG_DEBUG("save", "region %s clean, skip write-back\n", filePath.c_str());
        return;
    }

    dataFile.flush();
    const std::string bytes = dataFile.str();
    if (bytes.empty())
    {
        MC_LOG_ERROR("save", "region %s flush failed: empty backing buffer\n", filePath.c_str());
        return;
    }

    const bool ok = PlatformStorage::writeFile(filePath, bytes.data(), bytes.size());
    MC_LOG_DEBUG("save", "region %s flush %s, %u bytes\n",
           filePath.c_str(), ok ? "OK" : "FAIL", (unsigned)bytes.size());
    if (!ok)
        return;

    dirty = false;
    flushed = true;
#else
    if (dataFile.is_open())
    {
        dataFile.flush();
        dataFile.close();
    }
#endif
}
