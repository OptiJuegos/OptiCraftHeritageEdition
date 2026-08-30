#include "RegionFileChunkBuffer.h"

#include "RegionFile.h"

#include <limits>
#include <stdexcept>

RegionFileChunkBuffer::RegionFileChunkBuffer(RegionFile *regionfile, int_t x, int_t z) :
    regionFile(regionfile),
    chunkX(x),
    chunkZ(z)
{
    buf.reserve(8096);
}

void RegionFileChunkBuffer::write(byte_t b)
{
    buf.push_back(b);
}

void RegionFileChunkBuffer::write(const byte_t *data, int_t len)
{
    if (len < 0)
        throw std::invalid_argument("RegionFileChunkBuffer: negative write length");
    if (len == 0)
        return;
    if (data == nullptr)
        throw std::invalid_argument("RegionFileChunkBuffer: null write buffer");

    const std::size_t length = static_cast<std::size_t>(len);
    if (length > std::numeric_limits<std::size_t>::max() - buf.size())
        throw std::overflow_error("RegionFileChunkBuffer: write size overflow");
    buf.insert(buf.end(), data, data + length);
}

void RegionFileChunkBuffer::close()
{
    if (buf.size() > static_cast<std::size_t>(std::numeric_limits<int_t>::max()))
        throw std::overflow_error("RegionFileChunkBuffer: chunk payload exceeds Java int range");
    regionFile->write(chunkX, chunkZ, buf.data(), static_cast<int_t>(buf.size()));
}
