#include "Packet51MapChunk.h"

#include <stdexcept>
#include <utility>
#include <zlib.h>

#include "NetHandler.h"

namespace
{
constexpr int_t kMaxCompressedChunkBytes = 256 * 1024;
constexpr std::size_t kBytesPerPrimarySectionWorstCase = 12288;

int_t countSectionBits(int_t mask)
{
    int_t count = 0;
    for (int_t section = 0; section < 16; ++section)
        count += (mask >> section) & 1;
    return count;
}
}

Packet51MapChunk::Packet51MapChunk()
{
    isChunkDataPacket = true;
}

std::size_t Packet51MapChunk::expectedInflatedSize() const
{
    const int_t sectionCount = countSectionBits(yChMin & 0xffff);
    return kBytesPerPrimarySectionWorstCase * (std::size_t)sectionCount
        + (includeInitialize ? 256u : 0u);
}

void Packet51MapChunk::readPacketData(std::istream &is)
{
    xCh = IOUtil::readInt(is);
    zCh = IOUtil::readInt(is);
    includeInitialize = IOUtil::readUnsignedByte(is) != 0;
    yChMin = (int_t)(ushort_t)IOUtil::readShort(is);
    yChMax = (int_t)(ushort_t)IOUtil::readShort(is);
    tempLength = IOUtil::readInt(is);
    field_48178_h = IOUtil::readInt(is);

    if (tempLength <= 0 || tempLength > kMaxCompressedChunkBytes)
        throw std::runtime_error("Invalid compressed map chunk size: " + std::to_string(tempLength));

    compressedChunk.resize((std::size_t)tempLength);
    is.read(reinterpret_cast<char *>(compressedChunk.data()), tempLength);
    if (!is)
        throw std::runtime_error("Truncated compressed map chunk");

#ifndef WII_PLATFORM
    if (!ensureDecompressed())
        throw std::runtime_error("Invalid compressed map chunk data");
#endif
}

bool Packet51MapChunk::ensureDecompressed()
{
    if (!chunkData.empty())
        return true;
    if (compressedChunk.empty())
        return false;

    const std::size_t expected = expectedInflatedSize();
    if (expected == 0)
        return false;

    chunkData.assign(expected, 0);
    uLongf actual = (uLongf)chunkData.size();
    const int result = uncompress(reinterpret_cast<Bytef *>(chunkData.data()), &actual,
                                  reinterpret_cast<const Bytef *>(compressedChunk.data()),
                                  (uLong)compressedChunk.size());
    if (result != Z_OK)
    {
        chunkData.clear();
        return false;
    }

    // Java allocates the worst-case 12288 bytes for every primary section and
    // leaves any unused Add-array tail zero-filled. Keep that full allocation;
    // Chunk::func_48494_a consumes only the bytes selected by yChMax.
#ifndef WII_PLATFORM
    std::vector<byte_t>().swap(compressedChunk);
#endif
    return true;
}

std::vector<byte_t> Packet51MapChunk::takeCompressedData()
{
    return std::move(compressedChunk);
}

std::vector<byte_t> Packet51MapChunk::copyCompressedData() const
{
    return compressedChunk;
}

void Packet51MapChunk::writePacketData(std::ostream &os)
{
    IOUtil::writeInt(os, xCh);
    IOUtil::writeInt(os, zCh);
    IOUtil::writeByte(os, (byte_t)(includeInitialize ? 1 : 0));
    IOUtil::writeShort(os, (short_t)(yChMin & 0xffff));
    IOUtil::writeShort(os, (short_t)(yChMax & 0xffff));
    IOUtil::writeInt(os, tempLength);
    IOUtil::writeInt(os, field_48178_h);
    if (!compressedChunk.empty())
        os.write(reinterpret_cast<const char *>(compressedChunk.data()), tempLength);
}

void Packet51MapChunk::processPacket(NetHandler &nethandler)
{
    nethandler.func_48487_a(*this);
}

int_t Packet51MapChunk::getPacketSize()
{
    return 17 + tempLength;
}
