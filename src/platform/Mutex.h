#pragma once

#ifdef PS2_PLATFORM
#include <kernel.h>

class PlatformMutex
{
public:
    PlatformMutex()
    {
        ee_sema_t semaphore{};
        semaphore.init_count = 1;
        semaphore.max_count = 1;
        semaphore.option = 0;
        semaphoreId_ = CreateSema(&semaphore);
    }

    ~PlatformMutex()
    {
        if (semaphoreId_ >= 0)
            DeleteSema(semaphoreId_);
    }

    PlatformMutex(const PlatformMutex&) = delete;
    PlatformMutex& operator=(const PlatformMutex&) = delete;

    void lock()
    {
        if (semaphoreId_ >= 0)
            WaitSema(semaphoreId_);
    }

    bool try_lock()
    {
        return semaphoreId_ >= 0 && PollSema(semaphoreId_) >= 0;
    }

    void unlock()
    {
        if (semaphoreId_ >= 0)
            SignalSema(semaphoreId_);
    }

private:
    int semaphoreId_ = -1;
};

#else

#include <mutex>
using PlatformMutex = std::mutex;

#endif
