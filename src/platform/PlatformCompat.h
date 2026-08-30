#pragma once

#include <cstdint>
#include <chrono>

#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
#include "pc/lwjgl/Mouse.h"
#include "pc/lwjgl/Display.h"
#else
#include <SDL.h>
#endif

#ifdef WII_PLATFORM
#include <ogc/lwp_watchdog.h>
#include <unistd.h>
#endif

#ifdef PS2_PLATFORM
#include "ps2/system/Ps2Clock.h"
#endif

// Small platform layer for code that is shared by PC and the console ports.
// Keep direct SDL calls inside this file or inside src/pc only.
namespace PlatformCompat
{
inline uint32_t getTicks()
{
#if defined(WII_PLATFORM)
    // libogc's timebase rather than std::chrono. On this toolchain the chrono
    // clocks go through newlib's gettimeofday, which is only as good as whatever
    // backend is wired up; gettime() reads the Broadway's timebase register
    // directly and is what every other libogc timing path uses.
    return static_cast<uint32_t>(ticks_to_millisecs(gettime()));
#elif defined(PS2_PLATFORM)
    using namespace std::chrono;
    return static_cast<uint32_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
#else
    return SDL_GetTicks();
#endif
}

// Monotonic microseconds, for measuring how long something took.
//
// Separate from getTicks() because a millisecond is too coarse to spend a
// per-frame budget against: PLATFORM_CHUNK_BUILD_BUDGET_MS is single digits, so
// a millisecond clock quantises every measurement to 0 or 1 and the budget
// either never fires or fires after one section.
//
// Separate from System::nanoTime() because that goes through
// std::chrono::high_resolution_clock, which on these toolchains is only as good
// as whatever newlib has wired to gettimeofday -- and on the PS2 it can sit at a
// constant forever (see the boot-time USABLE/DEAD probe). A budget that reads a
// dead clock silently becomes no budget at all, so the console paths read the
// hardware timer they know is alive.
//
// The PS2 branch was missing until now: the comment above claimed both consoles
// read hardware, but only the Wii did, and this fell through to std::chrono ->
// newlib -- a five-call-deep chain (see Ps2Clock.h) charged twice per renderer
// update by RenderGlobal's MeshBudget, which is measuring milliseconds.
inline uint64_t getMonotonicMicros()
{
#if defined(WII_PLATFORM)
    return static_cast<uint64_t>(ticks_to_microsecs(gettime()));
#elif defined(PS2_PLATFORM)
    return static_cast<uint64_t>(ps2_ee_micros());
#else
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count());
#endif
}

inline void delay(uint32_t ms)
{
#if defined(WII_PLATFORM)
    // Real sleep: unlike the PS2 path this one yields to libogc's scheduler, so
    // the audio and USB threads keep running while the main loop waits.
    if (ms) usleep(ms * 1000u);
#elif defined(PS2_PLATFORM)
    // The PS2 main loop is already synced by the GS flip. Do not busy-wait here.
    (void)ms;
#else
    SDL_Delay(ms);
#endif
}

inline void setSmoothInputThreadPriority(bool enabled)
{
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
    // C6's Smooth Input is a JVM main-thread priority tweak. The console ports
    // have different scheduler/audio/input constraints, so changing their main
    // thread priority here would be a new platform policy rather than a faithful
    // translation of the desktop optimization.
    (void)enabled;
#else
    // Java C6 uses priority 10 normally and priority 5 with Smooth Input. SDL's
    // closest portable mapping is HIGH -> NORMAL. Cache the setting so the OS
    // priority API is touched only when the option changes, not every frame.
    static int appliedState = -1;
    const int requestedState = enabled ? 1 : 0;
    if (appliedState == requestedState)
        return;

    SDL_SetThreadPriority(enabled ? SDL_THREAD_PRIORITY_NORMAL : SDL_THREAD_PRIORITY_HIGH);
    appliedState = requestedState;
#endif
}

inline void getMouseState(int *x, int *y)
{
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
    // LWJGL Mouse::getY() is bottom-left origin. SDL_GetMouseState() is
    // top-left origin, and shared GUI code expects that here.
    if (x) *x = lwjgl::Mouse::getX();
    if (y) *y = lwjgl::Display::getHeight() - lwjgl::Mouse::getY() - 1;
#else
    SDL_GetMouseState(x, y);
#endif
}
}
