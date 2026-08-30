#include "platform/chunks/ChunkGenerationScheduler.h"

#include "platform/PlatformCompat.h"
#include "platform/PlatformTuning.h"
#include "platform/Thread.h"
#include "net/minecraft/src/Chunk.h"
#include "net/minecraft/src/IChunkProvider.h"
#include "net/minecraft/src/McRegionChunkLoader.h"

#include <atomic>
#include <deque>
#include <mutex>
#include <unordered_set>
#include <utility>

struct ChunkGenerationScheduler::Impl
{
    IChunkProvider* generator = nullptr;
    McRegionChunkLoader* regionLoader = nullptr;
    std::deque<std::pair<int_t, int_t>> pending;
    std::deque<std::pair<int_t, int_t>> worker;
    std::unordered_set<std::uint64_t> queued;
    std::deque<Result> results;
    mutable std::mutex mutex;
    std::atomic_bool stop{false};
    PlatformThread thread;
};

std::uint64_t ChunkGenerationScheduler::key(int_t x, int_t z)
{
    return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(x)) << 32)
         | static_cast<std::uint32_t>(z);
}

ChunkGenerationScheduler::ChunkGenerationScheduler(IChunkProvider* ownedGenerator,
                                                   McRegionChunkLoader* regionLoader)
    : impl_(new Impl())
{
    impl_->generator = ownedGenerator;
    impl_->regionLoader = regionLoader;
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
    return impl_->thread.start(&ChunkGenerationScheduler::threadEntry, this, 48 * 1024,
                               PLATFORM_ASYNC_GENERATION_THREAD_PRIORITY);
#else
    return false;
#endif
}

void ChunkGenerationScheduler::stop()
{
#if PLATFORM_ASYNC_CHUNK_GENERATION
    impl_->stop.store(true);
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
    if ((int_t)(impl_->pending.size() + impl_->worker.size()) >= queueLimit)
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
        dispatched = true;
    }
    return dispatched;
#else
    (void)budget; (void)predicate; (void)context;
    return false;
#endif
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
    while (!impl_->stop.load())
    {
        std::pair<int_t, int_t> coord;
        bool haveWork = false;
        {
            std::lock_guard<std::mutex> guard(impl_->mutex);
            if (!impl_->worker.empty())
            {
                coord = impl_->worker.front();
                impl_->worker.pop_front();
                haveWork = true;
            }
        }

        if (!haveWork)
        {
            PlatformCompat::delay(1);
            continue;
        }

        if (impl_->regionLoader != nullptr)
        {
            std::vector<byte_t> data;
            ChunkLoadStatus loadStatus = ChunkLoadStatus::Missing;
            if (impl_->regionLoader->readChunkData(coord.first, coord.second, data, &loadStatus))
            {
                std::lock_guard<std::mutex> guard(impl_->mutex);
                impl_->results.push_back({coord.first, coord.second, ResultKind::LoadedData,
                                          nullptr, std::move(data)});
                continue;
            }
            if (loadStatus == ChunkLoadStatus::ReadError)
            {
                std::lock_guard<std::mutex> guard(impl_->mutex);
                impl_->results.push_back({coord.first, coord.second, ResultKind::ReadError,
                                          nullptr, {}});
                continue;
            }
        }

        Chunk* generated = impl_->generator->provideChunk(coord.first, coord.second);
        if (generated == nullptr)
        {
            complete(coord.first, coord.second);
            continue;
        }

        std::lock_guard<std::mutex> guard(impl_->mutex);
        impl_->results.push_back({coord.first, coord.second, ResultKind::Generated,
                                  generated, {}});
    }
#endif
}
