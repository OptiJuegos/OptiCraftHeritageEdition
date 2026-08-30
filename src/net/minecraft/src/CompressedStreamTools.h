#pragma once

#include <iostream>
#include <vector>

class NBTTagCompound;

// net.minecraft.src.CompressedStreamTools
class CompressedStreamTools
{
public:
    static NBTTagCompound* readGzippedCompound(std::istream &is);
    static NBTTagCompound* readCompressed(std::istream &is); // alias for readGzippedCompound
    static void writeGzippedCompoundToOutputStream(NBTTagCompound *compound, std::ostream &os);
    static std::vector<char> compress(NBTTagCompound *compound);
    static NBTTagCompound* decompress(const std::vector<char> &compressed);

    // Read/write NBT without gzip (used by McRegionChunkLoader via RegionFile)
    static NBTTagCompound* readCompound(std::istream &is);
    static void writeCompound(NBTTagCompound *compound, std::ostream &os);
};
