#pragma once

#include "java/Type.h"
#include "platform/PlatformConfig.h"

#include <cstdint>
#include <vector>

class Chunk;
class IChunkProvider;
class McRegionChunkLoader;

class ChunkGenerationScheduler
{
public:
    enum class ResultKind
    {
        LoadedData,
        Generated,
        ReadError
    };

    enum class RequestStatus
    {
        Accepted,
        AlreadyQueued,
        QueueFull,
        Inactive
    };

    struct Result
    {
        int_t x = 0;
        int_t z = 0;
        ResultKind kind = ResultKind::ReadError;
        Chunk* chunk = nullptr;
        std::vector<byte_t> data;
    };

    using CoordinatePredicate = bool (*)(void* context, int_t x, int_t z);

    ChunkGenerationScheduler(IChunkProvider* ownedGenerator, McRegionChunkLoader* regionLoader);
    ~ChunkGenerationScheduler();

    bool start();
    void stop();
    bool active() const;

    RequestStatus requestDetailed(int_t x, int_t z, int_t queueLimit);
    bool request(int_t x, int_t z, int_t queueLimit);
    bool dispatch(int_t budget, CoordinatePredicate predicate, void* context);
    bool popResult(Result& out);
    void complete(int_t x, int_t z);
    void queueSizes(int_t& pending, int_t& completed) const;

private:
    static std::uint64_t key(int_t x, int_t z);
    static void* threadEntry(void* argument);
    void runWorker();

    struct Impl;
    Impl* impl_;
};
