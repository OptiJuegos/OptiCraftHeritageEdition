#include "platform/Thread.h"

#ifdef WII_PLATFORM
#include <ogc/lwp.h>
#else
#include <thread>
#include <new>
#endif

#ifdef WII_PLATFORM
PlatformThread::PlatformThread() : handle_(static_cast<std::uintptr_t>(LWP_THREAD_NULL)) {}
PlatformThread::~PlatformThread() { if (joinable() && !isCurrent()) join(); }
bool PlatformThread::start(Entry entry, void* argument, std::size_t stackSize, int priority)
{
    if (joinable() || entry == nullptr) return false;
    lwp_t thread = LWP_THREAD_NULL;
    const s32 result = LWP_CreateThread(&thread, entry, argument, nullptr, static_cast<u32>(stackSize), priority);
    if (result < 0) return false;
    handle_ = static_cast<std::uintptr_t>(thread);
    return true;
}
void PlatformThread::join()
{
    if (!joinable() || isCurrent()) return;
    LWP_JoinThread(static_cast<lwp_t>(handle_), nullptr);
    handle_ = static_cast<std::uintptr_t>(LWP_THREAD_NULL);
}
bool PlatformThread::joinable() const { return static_cast<lwp_t>(handle_) != LWP_THREAD_NULL; }
bool PlatformThread::isCurrent() const { return joinable() && static_cast<lwp_t>(handle_) == LWP_GetSelf(); }
#else
struct PlatformThread::Impl { std::thread thread; };
PlatformThread::PlatformThread() : impl_(new (std::nothrow) Impl()) {}
PlatformThread::~PlatformThread() { if (impl_) { if (joinable() && !isCurrent()) join(); delete impl_; } }
bool PlatformThread::start(Entry entry, void* argument, std::size_t, int)
{
    if (!impl_ || !entry || joinable()) return false;
    try { impl_->thread = std::thread([entry, argument]() { entry(argument); }); return true; }
    catch (...) { return false; }
}
void PlatformThread::join() { if (joinable() && !isCurrent()) impl_->thread.join(); }
bool PlatformThread::joinable() const { return impl_ && impl_->thread.joinable(); }
bool PlatformThread::isCurrent() const { return joinable() && impl_->thread.get_id() == std::this_thread::get_id(); }
#endif
