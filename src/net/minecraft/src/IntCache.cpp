#include "IntCache.h"

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>

#if PLATFORM_WII || PLATFORM_3DS
#include <atomic>

#include "platform/Thread.h"
#endif

#if PLATFORM_WII || PLATFORM_3DS
namespace
{

// Two storage slots rather than one per thread.
//
// The Wii and the 3DS run PLATFORM_ASYNC_CHUNK_GENERATION, so the generation
// worker walks the whole GenLayer chain -- every layer of which allocates
// through getIntCache() -- while the main thread does the same for the
// sky/fog colour in EntityRenderer, for mob spawning and for every
// getBiomeGenAt() call in World and Chunk. On one shared free list that is a
// data race with teeth: resetIntCache() moves arrays from the in-use lists
// back to the free lists, so a reset on one thread hands the other thread's
// live array to the next caller, and both then write the same biome buffer.
// The std::vector bookkeeping underneath races too, which is heap corruption
// rather than a wrong biome.
//
// PLATFORM_PC_LEGACY solves this with thread_local. The Wii cannot: devkitPPC
// miscompiles it, because libogc's linker script declares no .tdata/.tbss output
// section and every thread_local ends up aliasing the same word (the full
// diagnosis is in external/stb_image.cpp, and cmake/wii_check_no_tls.cmake fails
// the link if any .tbss reappears).
//
// The 3DS joins with the same two-slot scheme rather than trying
// thread_local: the 3dsx load path and libctru's thread model carry their own
// TLS history, and the scheduler only ever runs one worker, so the Wii's
// slot-by-identity approach is the shape already proven here.
//
// So the slot is selected by thread identity instead. Only two threads ever
// reach this cache and only the worker registers itself, which is why a single
// registered id is enough -- a third generating thread would need a real map.
// The cost is one duplicated set of scratch arrays.
std::atomic<bool> generationThreadBound{false};
std::atomic<std::uintptr_t> generationThreadId{0};

} // namespace
#endif

IntCache::Storage &IntCache::storage()
{
#if PLATFORM_WII || PLATFORM_3DS
    static Storage mainStorage;
    static Storage generationStorage;

    // The bound flag is separate from the id because no platform promises that
    // a valid thread id is non-zero, so the id alone has no free sentinel.
    if (generationThreadBound.load(std::memory_order_acquire) &&
        PlatformThread::currentId() == generationThreadId.load(std::memory_order_relaxed))
        return generationStorage;
    return mainStorage;
#elif PLATFORM_PC_LEGACY
    static thread_local Storage perThread;
    return perThread;
#else
    static Storage shared;
    return shared;
#endif
}

#if PLATFORM_WII || PLATFORM_3DS
void IntCache::bindGenerationThread()
{
    generationThreadId.store(PlatformThread::currentId(), std::memory_order_relaxed);
    generationThreadBound.store(true, std::memory_order_release);
}

void IntCache::unbindGenerationThread()
{
    generationThreadBound.store(false, std::memory_order_release);
}
#endif

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

    Storage &store = storage();

    if (requestedSize <= 256)
        return acquire(store.freeSmallArrays, store.inUseSmallArrays, 256);

    if (requestedSize > store.intCacheSize)
    {
        store.intCacheSize = requestedSize;
        store.freeLargeArrays.clear();

        for (OwnedArray &array : store.inUseLargeArrays)
            store.retiredLargeArrays.push_back(std::move(array));
        store.inUseLargeArrays.clear();
    }

    return acquire(store.freeLargeArrays, store.inUseLargeArrays, store.intCacheSize);
}

void IntCache::resetIntCache()
{
    Storage &store = storage();

    store.retiredLargeArrays.clear();

    if (!store.freeLargeArrays.empty())
        store.freeLargeArrays.pop_back();
    if (!store.freeSmallArrays.empty())
        store.freeSmallArrays.pop_back();

    for (OwnedArray &array : store.inUseLargeArrays)
        store.freeLargeArrays.push_back(std::move(array));
    for (OwnedArray &array : store.inUseSmallArrays)
        store.freeSmallArrays.push_back(std::move(array));

    store.inUseLargeArrays.clear();
    store.inUseSmallArrays.clear();
}
