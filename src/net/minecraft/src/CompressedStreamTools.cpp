#include "CompressedStreamTools.h"

#include "NBTBase.h"
#include "NBTTagCompound.h"
#include <sstream>
#include <vector>
#include <stdexcept>
#include <zlib.h>

static std::vector<char> gzipDecompress(const std::vector<char> &compressed)
{
    // Grow the output in steps instead of committing a flat 1MB up-front —
    // these are level.dat / map_N.dat sized files, and on the PS2's 32MB heap
    // that spike was a real OOM contributor.
#ifdef PS2_PLATFORM
    const size_t maxOut = 256 * 1024;
#else
    const size_t maxOut = 1024 * 1024;
#endif
    std::vector<char> out;
    out.resize(64 * 1024);

    z_stream zs{};
    if (inflateInit2(&zs, 16 + MAX_WBITS) != Z_OK) // gzip mode
        throw std::runtime_error("gzip decompress failed");
    zs.next_in  = (Bytef*)compressed.data();
    zs.avail_in = (uInt)compressed.size();

    for (;;)
    {
        zs.next_out  = (Bytef*)(out.data() + zs.total_out);
        zs.avail_out = (uInt)(out.size() - (size_t)zs.total_out);
        int ret = inflate(&zs, Z_NO_FLUSH);
        if (ret == Z_STREAM_END)
            break;
        const bool outFull = (ret == Z_OK || ret == Z_BUF_ERROR) && zs.avail_out == 0;
        if (!outFull || out.size() >= maxOut)
        {
            inflateEnd(&zs);
            throw std::runtime_error("gzip decompress failed");
        }
        size_t next = out.size() * 2;
        if (next > maxOut)
            next = maxOut;
        out.resize(next);
    }

    uLong actual = zs.total_out;
    inflateEnd(&zs);
    out.resize(actual);
    return out;
}

static std::vector<char> gzipCompress(const std::vector<char> &raw)
{
    uLongf compLen = compressBound((uLong)raw.size()) + 64;
    std::vector<char> out(compLen);

    z_stream zs{};
    deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
    zs.next_in  = (Bytef*)raw.data();
    zs.avail_in = (uInt)raw.size();
    zs.next_out  = (Bytef*)out.data();
    zs.avail_out = (uInt)out.size();

    deflate(&zs, Z_FINISH);
    uLong actual = zs.total_out;
    deflateEnd(&zs);

    out.resize(actual);
    return out;
}

NBTTagCompound* CompressedStreamTools::readGzippedCompound(std::istream &is)
{
    // Read all bytes from stream
    std::vector<char> compressed((std::istreambuf_iterator<char>(is)), {});
    std::vector<char> raw = gzipDecompress(compressed);

    std::string s(raw.begin(), raw.end());
    std::istringstream rawStream(s, std::ios::binary);
    return readCompound(rawStream);
}

NBTTagCompound* CompressedStreamTools::readCompressed(std::istream &is)
{
    return readGzippedCompound(is);
}

void CompressedStreamTools::writeGzippedCompoundToOutputStream(NBTTagCompound *compound, std::ostream &os)
{
    const std::vector<char> compressed = compress(compound);
    os.write(compressed.data(), (std::streamsize)compressed.size());
}

std::vector<char> CompressedStreamTools::compress(NBTTagCompound *compound)
{
    if (compound == nullptr)
        return {};

    std::ostringstream rawStream(std::ios::binary);
    writeCompound(compound, rawStream);
    const std::string raw = rawStream.str();
    return gzipCompress(std::vector<char>(raw.begin(), raw.end()));
}

NBTTagCompound* CompressedStreamTools::decompress(const std::vector<char> &compressed)
{
    const std::vector<char> raw = gzipDecompress(compressed);
    std::string data(raw.begin(), raw.end());
    std::istringstream rawStream(data, std::ios::binary);
    return readCompound(rawStream);
}

NBTTagCompound* CompressedStreamTools::readCompound(std::istream &is)
{
    NBTBase *tag = NBTBase::readTag(is);
    NBTTagCompound *compound = dynamic_cast<NBTTagCompound*>(tag);
    if (compound == nullptr)
    {
        delete tag;
        throw std::runtime_error("Root tag must be a named compound tag");
    }
    return compound;
}

void CompressedStreamTools::writeCompound(NBTTagCompound *compound, std::ostream &os)
{
    NBTBase::writeTag(compound, os);
}
