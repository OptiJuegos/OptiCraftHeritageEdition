#pragma once

#include <vector>
#include "java/Type.h"

class RegionFile;

// net.minecraft.src.RegionFileChunkBuffer (extends ByteArrayOutputStream)
class RegionFileChunkBuffer
{
public:
    RegionFileChunkBuffer(RegionFile *regionfile, int_t x, int_t z);

    void write(byte_t b);
    void write(const byte_t *buf, int_t len);
    void close();

private:
    RegionFile *regionFile;
    int_t chunkX;
    int_t chunkZ;
    std::vector<byte_t> buf;
};
