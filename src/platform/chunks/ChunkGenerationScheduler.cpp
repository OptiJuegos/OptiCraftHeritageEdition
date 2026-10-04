#include "platform/chunks/ChunkGenerationScheduler.h"

#include "platform/PlatformCompat.h"
#include "platform/PlatformTuning.h"
#include "platform/Thread.h"
#include "net/minecraft/src/Chunk.h"
#include "net/minecraft/src/IChunkProvider.h"
#include "net/minecraft/src/ChunkProviderGenerate.h"
#include "net/minecraft/src/IntCache.h"
#include "net/minecraft/src/McRegionChunkLoader.h"
#include "net/minecraft/src/NBTTagCompound.h"

#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <unordered_set>
#include <utility>

struct ChunkGenerationScheduler::Impl
{
    IChunkProvider* generator = nullptr;
    McRegionChunkLoader* regionLoader = nullptr;
    World* world = nullptr;
    std::atomic<int_t> focusX{0};
    std::atomic<int_t> focusZ{0};
    std::deque<std::pair<int_t, int_t>> pending;
    std::deque<std::pair<int_t, int_t>> worker;
    std::unordered_set<std::uint64_t> queued;
    std::deque<Result> results;
    mutable std::mutex mutex;
    std::condition_variable wake;
    std::atomic_bool stop{false};
    PlatformThread thread;
};

std::uint64_t ChunkGenerationScheduler::key(int_t x, int_t z)
{
    return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(x)) << 32)
         | static_cast<std::uint32_t>(z);
}

ChunkGenerationScheduler::ChunkGenerationScheduler(IChunkProvider* ownedGenerator,
                                                   McRegionChunkLoader* regionLoader,
                                                   World* world)
    : impl_(new Impl())
{
    impl_->generator = ownedGenerator;
    impl_->regionLoader = regionLoader;
    impl_->world = world;
}

ChunkGenerationScheduler::~ChunkGenerationScheduler()
{
    stop();
    delete impl_->generator;
    impl_->generator = nullptr;
    delete impl_;
}

bool ChunkGenerationScheduler::start()
{
#if PLATFORM_ASYNC_CHUNK_GENERATION
    if (impl_->generator == nullptr)
        return false;
    impl_->stop.store(false);
    if (!impl_->thread.start(&ChunkGenerationScheduler::threadEntry, this, 48 * 1024,
                             PLATFORM_ASYNC_GENERATION_THREAD_PRIORITY,
                             PLATFORM_ASYNC_GENERATION_AFFINITY_MASK))
        return false;
    return true;
#else
    return false;
#endif
}

void ChunkGenerationScheduler::stop()
{
#if PLATFORM_ASYNC_CHUNK_GENERATION
    // Under the mutex so the flag cannot be set between the worker evaluating
    // its wait predicate and actually sleeping, which would lose this wakeup
    // and leave join() blocked forever.
    {
        std::lock_guard<std::mutex> guard(impl_->mutex);
        impl_->stop.store(true);
    }
    impl_->wake.notify_all();
    if (impl_->thread.joinable() && !impl_->thread.isCurrent())
        impl_->thread.join();

    std::lock_guard<std::mutex> guard(impl_->mutex);
    for (Result& result : impl_->results)
        delete result.chunk;
    impl_->results.clear();
    impl_->pending.clear();
    impl_->worker.clear();
    impl_->queued.clear();
#endif
}

bool ChunkGenerationScheduler::active() const
{
#if PLATFORM_ASYNC_CHUNK_GENERATION
    return impl_->generator != nullptr && impl_->thread.joinable() && !impl_->stop.load();
#else
    return false;
#endif
}

ChunkGenerationScheduler::RequestStatus ChunkGenerationScheduler::requestDetailed(int_t x, int_t z, int_t queueLimit)
{
#if PLATFORM_ASYNC_CHUNK_GENERATION
    if (!active())
        return RequestStatus::Inactive;

    const std::uint64_t k = key(x, z);
    std::lock_guard<std::mutex> guard(impl_->mutex);
    if (impl_->queued.count(k) != 0)
        return RequestStatus::AlreadyQueued;
    if ((int_t)(impl_->pending.size() + impl_->worker.size() + impl_->results.size()) >= queueLimit)
        return RequestStatus::QueueFull;
    impl_->queued.insert(k);
    impl_->pending.emplace_back(x, z);
    return RequestStatus::Accepted;
#else
    (void)x; (void)z; (void)queueLimit;
    return RequestStatus::Inactive;
#endif
}

bool ChunkGenerationScheduler::request(int_t x, int_t z, int_t queueLimit)
{
    return requestDetailed(x, z, queueLimit) == RequestStatus::Accepted;
}

bool ChunkGenerationScheduler::dispatch(int_t budget, CoordinatePredicate predicate, void* context)
{
#if PLATFORM_ASYNC_CHUNK_GENERATION
    if (budget <= 0 || !active())
        return false;

    bool dispatched = false;
    for (int_t n = 0; n < budget; ++n)
    {
        std::pair<int_t, int_t> coord;
        {
            std::lock_guard<std::mutex> guard(impl_->mutex);
            if (impl_->pending.empty())
                break;
            coord = impl_->pending.front();
            impl_->pending.pop_front();
        }

        if (predicate != nullptr && !predicate(context, coord.first, coord.second))
        {
            complete(coord.first, coord.second);
            continue;
        }

        {
            std::lock_guard<std::mutex> guard(impl_->mutex);
            impl_->worker.push_back(coord);
        }
        impl_->wake.notify_one();
        dispatched = true;
    }
    return dispatched;
#else
    (void)budget; (void)predicate; (void)context;
    return false;
#endif
}

void ChunkGenerationScheduler::setFocus(int_t chunkX, int_t chunkZ)
{
    impl_->focusX.store(chunkX, std::memory_order_relaxed);
    impl_->focusZ.store(chunkZ, std::memory_order_relaxed);
}

bool ChunkGenerationScheduler::popResult(Result& out)
{
#if PLATFORM_ASYNC_CHUNK_GENERATION
    std::lock_guard<std::mutex> guard(impl_->mutex);
    if (impl_->results.empty())
        return false;
    out = std::move(impl_->results.front());
    impl_->results.pop_front();
    return true;
#else
    (void)out;
    return false;
#endif
}

void ChunkGenerationScheduler::complete(int_t x, int_t z)
{
#if PLATFORM_ASYNC_CHUNK_GENERATION
    std::lock_guard<std::mutex> guard(impl_->mutex);
    impl_->queued.erase(key(x, z));
#else
    (void)x; (void)z;
#endif
}

void ChunkGenerationScheduler::queueSizes(int_t& pending, int_t& completed) const
{
#if PLATFORM_ASYNC_CHUNK_GENERATION
    std::lock_guard<std::mutex> guard(impl_->mutex);
    pending = static_cast<int_t>(impl_->pending.size() + impl_->worker.size());
    completed = static_cast<int_t>(impl_->results.size());
#else
    pending = 0;
    completed = 0;
#endif
}

void* ChunkGenerationScheduler::threadEntry(void* argument)
{
    static_cast<ChunkGenerationScheduler*>(argument)->runWorker();
    return nullptr;
}

void ChunkGenerationScheduler::runWorker()
{
#if PLATFORM_ASYNC_CHUNK_GENERATION
#if PLATFORM_WII || PLATFORM_3DS
    // Every GenLayer step below allocates from IntCache, which the main thread
    // also uses for sky colour, mob spawning and getBiomeGenAt(). Claim the
    // worker's own slot before the first chunk.
    IntCache::bindGenerationThread();
#endif

    while (true)
    {
        std::pair<int_t, int_t> coord;
        {
            // Sleep rather than poll. The worker only ever gets the CPU the main
            // thread leaves behind, so a 1 ms poll spent a large share of that
            // gap deciding it had nothing to do; dispatch() now wakes it.
            std::unique_lock<std::mutex> lock(impl_->mutex);
            impl_->wake.wait(lock, [this]()
            {
                return impl_->stop.load(std::memory_order_relaxed) || !impl_->worker.empty();
            });

            if (impl_->stop.load(std::memory_order_relaxed))
                break;

#if PLATFORM_ASYNC_NEAREST_FIRST
            // Nearest queued column to the focus first (Chebyshev, the chunk
            // ring metric). Requests arrive in renderer/prefetch order, which
            // after a turn is not the order the player will see them in.
            const int_t focusX = impl_->focusX.load(std::memory_order_relaxed);
            const int_t focusZ = impl_->focusZ.load(std::memory_order_relaxed);
            auto best = impl_->worker.begin();
            long_t bestDistance = 0;
            for (auto it = impl_->worker.begin(); it != impl_->worker.end(); ++it)
            {
                long_t dx = static_cast<long_t>(it->first) - static_cast<long_t>(focusX);
                long_t dz = static_cast<long_t>(it->second) - static_cast<long_t>(focusZ);
                if (dx < 0) dx = -dx;
                if (dz < 0) dz = -dz;
                const long_t distance = dx > dz ? dx : dz;
                if (it == impl_->worker.begin() || distance < bestDistance)
                {
                    best = it;
                    bestDistance = distance;
                }
            }
            coord = *best;
            impl_->worker.erase(best);
#else
            coord = impl_->worker.front();
            impl_->worker.pop_front();
#endif
        }

        if (impl_->regionLoader != nullptr)
        {
            std::vector<byte_t> data;
            ChunkLoadStatus loadStatus = ChunkLoadStatus::Missing;
            if (impl_->regionLoader->readChunkData(coord.first, coord.second, data, &loadStatus))
            {
#if PLATFORM_ASYNC_CHUNK_DECODE
                if (impl_->world != nullptr)
                {
                    // Blocks, light and heightmap only; the parsed root travels
                    // with the chunk so publish can add its entities. A decode
                    // failure falls through to the game-thread path, which
                    // reports it and preserves the region data as before.
                    Result result;
                    result.x = coord.first;
                    result.z = coord.second;
                    ChunkLoadStatus decodeStatus = ChunkLoadStatus::ReadError;
                    result.chunk = impl_->regionLoader->decodeChunkBlocksFromData(
                        impl_->world, coord.first, coord.second, data, result.nbt, &decodeStatus);
                    if (result.chunk != nullptr)
                    {
                        result.kind = ResultKind::LoadedChunk;
                        std::lock_guard<std::mutex> guard(impl_->mutex);
                        impl_->results.push_back(std::move(result));
                        continue;
                    }
                }
#endif
                Result result;
                result.x = coord.first;
                result.z = coord.second;
                result.kind = ResultKind::LoadedData;
                result.data = std::move(data);
                std::lock_guard<std::mutex> guard(impl_->mutex);
                impl_->results.push_back(std::move(result));
                continue;
            }
            if (loadStatus == ChunkLoadStatus::ReadError)
            {
                Result result;
                result.x = coord.first;
                result.z = coord.second;
                result.kind = ResultKind::ReadError;
                std::lock_guard<std::mutex> guard(impl_->mutex);
                impl_->results.push_back(std::move(result));
                continue;
            }
        }

#if PLATFORM_PC_LEGACY || PLATFORM_WII || PLATFORM_3DS
        if (ChunkProviderGenerate* generator = dynamic_cast<ChunkProviderGenerate*>(impl_->generator))
        {
            std::vector<byte_t> generatedData;
            if (!generator->generateAsyncChunkData(coord.first, coord.second, generatedData))
            {
                complete(coord.first, coord.second);
                continue;
            }

            Result result;
            result.x = coord.first;
            result.z = coord.second;
            result.kind = ResultKind::GeneratedData;
            result.data = std::move(generatedData);
            std::lock_guard<std::mutex> guard(impl_->mutex);
            impl_->results.push_back(std::move(result));
            continue;
        }
#endif

        Chunk* generated = impl_->generator->provideChunk(coord.first, coord.second);
        if (generated == nullptr)
        {
            complete(coord.first, coord.second);
            continue;
        }

        Result result;
        result.x = coord.first;
        result.z = coord.second;
        result.kind = ResultKind::Generated;
        result.chunk = generated;
        std::lock_guard<std::mutex> guard(impl_->mutex);
        impl_->results.push_back(std::move(result));
    }

#if PLATFORM_WII || PLATFORM_3DS
    IntCache::unbindGenerationThread();
#endif
#endif
}
