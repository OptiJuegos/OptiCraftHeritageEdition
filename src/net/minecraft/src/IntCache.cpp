#include "IntCache.h"

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>

int_t IntCache::intCacheSize = 256;
std::vector<IntCache::OwnedArray> IntCache::freeSmallArrays;
std::vector<IntCache::OwnedArray> IntCache::inUseSmallArrays;
std::vector<IntCache::OwnedArray> IntCache::freeLargeArrays;
std::vector<IntCache::OwnedArray> IntCache::inUseLargeArrays;
std::vector<IntCache::OwnedArray> IntCache::retiredLargeArrays;

IntCache::Array &IntCache::acquire(std::vector<OwnedArray> &freeArrays,
                                   std::vector<OwnedArray> &inUseArrays,
                                   int_t size)
{
    OwnedArray array;
    if (freeArrays.empty())
        array = std::make_unique<Array>(static_cast<std::size_t>(size), 0);
    else
    {
        array = std::move(freeArrays.back());
        freeArrays.pop_back();
        if (array->size() < static_cast<std::size_t>(size))
            array->resize(static_cast<std::size_t>(size));
    }

    inUseArrays.push_back(std::move(array));
    return *inUseArrays.back();
}

int_t IntCache::checkedAreaSize(int_t width, int_t height)
{
    if (width < 0 || height < 0)
        throw std::length_error("IntCache: negative area dimension");

    const std::uint64_t area = static_cast<std::uint64_t>(width) *
                               static_cast<std::uint64_t>(height);
    if (area > static_cast<std::uint64_t>(std::numeric_limits<int_t>::max()))
        throw std::length_error("IntCache: area exceeds Java array range");
    return static_cast<int_t>(area);
}

IntCache::Array &IntCache::getIntCache(int_t requestedSize)
{
    if (requestedSize < 0)
        throw std::length_error("IntCache: negative array size");

    if (requestedSize <= 256)
        return acquire(freeSmallArrays, inUseSmallArrays, 256);

    if (requestedSize > intCacheSize)
    {
        intCacheSize = requestedSize;
        freeLargeArrays.clear();

        for (OwnedArray &array : inUseLargeArrays)
            retiredLargeArrays.push_back(std::move(array));
        inUseLargeArrays.clear();
    }

    return acquire(freeLargeArrays, inUseLargeArrays, intCacheSize);
}

void IntCache::resetIntCache()
{
    retiredLargeArrays.clear();

    if (!freeLargeArrays.empty())
        freeLargeArrays.pop_back();
    if (!freeSmallArrays.empty())
        freeSmallArrays.pop_back();

    for (OwnedArray &array : inUseLargeArrays)
        freeLargeArrays.push_back(std::move(array));
    for (OwnedArray &array : inUseSmallArrays)
        freeSmallArrays.push_back(std::move(array));

    inUseLargeArrays.clear();
    inUseSmallArrays.clear();
}
