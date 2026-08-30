#ifdef PS2_PLATFORM

#include "ps2/system/Ps2LibcLocks.h"

#include <kernel.h>
#include <stdio.h>

namespace
{

static int s_semaphore = -1;
static int s_ownerThread = -1;
static int s_recursionDepth = 0;

static int s_earlyDepth = 0;
static int s_earlyInterruptState = 0;

static void acquireEarly()
{
    const int previousInterruptState = DIntr();
    if (s_earlyDepth == 0)
        s_earlyInterruptState = previousInterruptState;
    ++s_earlyDepth;
}

static void releaseEarly()
{
    if (s_earlyDepth <= 0)
        return;

    --s_earlyDepth;
    if (s_earlyDepth == 0 && s_earlyInterruptState)
        EIntr();
}

static void acquireGlobal()
{
    if (s_semaphore < 0)
    {
        acquireEarly();
        return;
    }

    const int threadId = GetThreadId();
    if (s_ownerThread == threadId)
    {
        ++s_recursionDepth;
        return;
    }

    WaitSema(s_semaphore);
    s_ownerThread = threadId;
    s_recursionDepth = 1;
}

static bool tryAcquireGlobal()
{
    if (s_semaphore < 0)
    {
        acquireEarly();
        return true;
    }

    const int threadId = GetThreadId();
    if (s_ownerThread == threadId)
    {
        ++s_recursionDepth;
        return true;
    }

    if (PollSema(s_semaphore) < 0)
        return false;

    s_ownerThread = threadId;
    s_recursionDepth = 1;
    return true;
}

static void releaseGlobal()
{
    if (s_semaphore < 0)
    {
        releaseEarly();
        return;
    }

    const int threadId = GetThreadId();
    if (s_ownerThread != threadId || s_recursionDepth <= 0)
        return;

    --s_recursionDepth;
    if (s_recursionDepth == 0)
    {
        s_ownerThread = -1;
        SignalSema(s_semaphore);
    }
}

} // namespace

namespace Ps2LibcLocks
{

bool initialize()
{
    if (s_semaphore >= 0)
        return true;

    ee_sema_t semaphore = {};
    semaphore.init_count = 1;
    semaphore.max_count = 1;
    semaphore.option = 0;

    const int semaphoreId = CreateSema(&semaphore);
    if (semaphoreId < 0)
        return false;

    s_semaphore = semaphoreId;
    return true;
}

bool initialized()
{
    return s_semaphore >= 0;
}

} // namespace Ps2LibcLocks

extern "C"
{

struct __lock
{
    int token;
};

static __lock s_libcLock = {0};

void __retarget_lock_init(_LOCK_T* lock)
{
    if (lock)
        *lock = &s_libcLock;
}

void __retarget_lock_init_recursive(_LOCK_T* lock)
{
    if (lock)
        *lock = &s_libcLock;
}

void __retarget_lock_close(_LOCK_T lock)
{
    (void)lock;
}

void __retarget_lock_close_recursive(_LOCK_T lock)
{
    (void)lock;
}

void __retarget_lock_acquire(_LOCK_T lock)
{
    (void)lock;
    acquireGlobal();
}

void __retarget_lock_acquire_recursive(_LOCK_T lock)
{
    (void)lock;
    acquireGlobal();
}

int __retarget_lock_try_acquire(_LOCK_T lock)
{
    (void)lock;
    return tryAcquireGlobal() ? 1 : 0;
}

int __retarget_lock_try_acquire_recursive(_LOCK_T lock)
{
    (void)lock;
    return tryAcquireGlobal() ? 1 : 0;
}

void __retarget_lock_release(_LOCK_T lock)
{
    (void)lock;
    releaseGlobal();
}

void __retarget_lock_release_recursive(_LOCK_T lock)
{
    (void)lock;
    releaseGlobal();
}

void __malloc_lock(struct _reent* reent)
{
    (void)reent;
    acquireGlobal();
}

void __malloc_unlock(struct _reent* reent)
{
    (void)reent;
    releaseGlobal();
}

} // extern "C"

#endif // PS2_PLATFORM
