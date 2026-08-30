#pragma once

#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <iostream>
#include "java/Type.h"

class RegionFile;

// net.minecraft.src.RegionFileCache
class RegionFileCache
{
public:
    enum class Format
    {
        McRegion,
        Anvil
    };

private:
    RegionFileCache() = delete;

public:
    static RegionFile* getRegionFile(const std::string &worldDir, int_t chunkX, int_t chunkZ);
    static std::shared_ptr<RegionFile> acquireRegionFile(const std::string &worldDir, int_t chunkX, int_t chunkZ,
                                                        Format format = Format::McRegion);
    static void clearCache();
    static int_t getSizeDelta(const std::string &worldDir, int_t chunkX, int_t chunkZ);
    static std::istream* getChunkInputStream(const std::string &worldDir, int_t chunkX, int_t chunkZ);
    static std::ostream* getChunkOutputStream(const std::string &worldDir, int_t chunkX, int_t chunkZ);

private:
    static std::map<std::string, std::shared_ptr<RegionFile>> cache;
    static std::mutex cacheMutex;
};
