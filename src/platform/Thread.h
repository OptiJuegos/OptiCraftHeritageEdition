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

    bool start(Entry entry, void* argument, std::size_t stackSize = 32 * 1024, int priority = 64);
    void join();
    bool joinable() const;
    bool isCurrent() const;

private:
#ifdef WII_PLATFORM
    std::uintptr_t handle_;
#else
    struct Impl;
    Impl* impl_;
#endif
};
