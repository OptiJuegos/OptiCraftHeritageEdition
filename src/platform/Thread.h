#pragma once

#include <cstddef>
#include <cstdint>

class PlatformThread
{
public:
    using Entry = void* (*)(void*);

    PlatformThread();
    ~PlatformThread();
    PlatformThread(const PlatformThread&) = delete;
    PlatformThread& operator=(const PlatformThread&) = delete;

    bool start(Entry entry, void* argument, std::size_t stackSize = 32 * 1024,
               int priority = 64, std::uintptr_t affinityMask = 0);
    void join();
    bool joinable() const;
    bool isCurrent() const;

    // Identity of the calling thread, for code that has to tell two threads
    // apart without owning either PlatformThread. Comparable and stable for the
    // lifetime of the thread; the value itself carries no meaning.
    static std::uintptr_t currentId();

    // Platforms whose OS only grants the application a second CPU after an
    // explicit request (the 3DS: core 1 needs APT_SetAppCpuTimeLimit first)
    // gate it here. Call before starting worker threads; until then a thread
    // whose affinity mask names that core falls back to the default core,
    // which keeps it correct on every loader that refuses the request.
    // No-op where every core is always available.
    static void setSecondaryCoreAvailable(bool available);

private:
#ifdef WII_PLATFORM
    std::uintptr_t handle_;
#else
    struct Impl;
    Impl* impl_;
#endif
};
