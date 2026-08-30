#include "ps2/system/Ps2Heap.h"

#ifdef PS2_PLATFORM

#include <cstdio>
#include <cstdlib>
#include <new>

#include "platform/Log.h"
#include "platform/WorldLoadTrace.h"

extern "C" long ps2_dbg_ram_free_kb();
extern "C" long ps2_dbg_ram_used_kb();
extern "C" long ps2_dbg_malloc_used_kb();
extern "C" long ps2_dbg_malloc_free_kb();
extern "C" long ps2_dbg_gs_queue_ram_bytes();
extern "C" int  ps2_dbg_vram_current_kb();

namespace
{
// Same contract as the Wii side: five lines, short enough for GuiErrorScreen's
// fixed-width column. Static storage because capture happens after the heap has
// already refused a request.
char s_badAllocLines[5][96] = {};

std::size_t s_lastBadAllocRequest = 0;
void* s_lastBadAllocCaller = nullptr;

// The failure has to reach debug.log from HERE, not from the catch site.
// Minecraft::run() does catch bad_alloc and calls platformCaptureBadAlloc(),
// but that only runs if the throw completes: __cxa_allocate_exception() itself
// calls malloc, and the unwinder then walks .eh_frame across the whole world
// creation stack. On a heap that has just refused a request, either can stop
// without ever reaching a handler -- and then the failure is invisible, which
// reads as a hang rather than an out-of-memory.
//
// McLog::write() formats into static buffers and reaches the file through
// fopen/fputs, which use C malloc rather than operator new, so a failure there
// returns null and stops logging instead of re-entering this function. The
// guard covers anything else in the sink that might allocate.
bool s_reportingFailure = false;

void reportAllocFailure(std::size_t request, void* caller)
{
    if (s_reportingFailure)
        return;
    s_reportingFailure = true;

    MC_LOG_ERROR("memory", "[PS2] operator new FAILED: %u bytes, caller %p, "
                           "used=%ldKB free=%ldKB mallocUsed=%ldKB mallocFree=%ldKB\n",
                 (unsigned)request, caller,
                 ps2_dbg_ram_used_kb(), ps2_dbg_ram_free_kb(),
                 ps2_dbg_malloc_used_kb(), ps2_dbg_malloc_free_kb());

    const char* stage = WorldLoadTrace::currentStage();
    if (stage[0] != '\0')
        MC_LOG_ERROR("memory", "[PS2] operator new FAILED during world load: %s\n", stage);

    s_reportingFailure = false;
}
} // namespace

// The standard allocator reports bad_alloc and nothing else. On a 32MB console
// that makes a fragmented 64KB texture upload indistinguishable from a corrupt
// save asking for 8MB, and those have opposite fixes. Record the request and
// the return address before throwing.
//
// This mirrors the Wii override in src/wii/WiiHeap.cpp rather than inventing a
// second strategy: same retry-then-new_handler loop the standard requires, same
// two captured facts.
void* operator new(std::size_t size)
{
    const std::size_t request = size == 0 ? 1 : size;
    for (;;)
    {
        if (void* result = std::malloc(request))
            return result;

        std::new_handler handler = std::get_new_handler();
        if (handler == nullptr)
        {
            s_lastBadAllocRequest = request;
            s_lastBadAllocCaller =
                __builtin_extract_return_addr(__builtin_return_address(0));
            reportAllocFailure(s_lastBadAllocRequest, s_lastBadAllocCaller);
            throw std::bad_alloc();
        }
        handler();
    }
}

void* operator new[](std::size_t size)
{
    return ::operator new(size);
}

void ps2HeapCaptureBadAlloc()
{
    const long freeKb = ps2_dbg_ram_free_kb();
    const long usedKb = ps2_dbg_ram_used_kb();
    const long mallocUsedKb = ps2_dbg_malloc_used_kb();
    const long mallocFreeKb = ps2_dbg_malloc_free_kb();

    std::snprintf(s_badAllocLines[0], sizeof(s_badAllocLines[0]),
        "OUT OF MEMORY");
    std::snprintf(s_badAllocLines[1], sizeof(s_badAllocLines[1]),
        "heap used %ldKB, free %ldKB", usedKb, freeKb);
    // mallocFree high while free is near zero is the fragmentation signature:
    // the bytes exist, no single run of them is large enough. That distinction
    // decides whether the fix is "use less" or "stop churning the arena".
    std::snprintf(s_badAllocLines[2], sizeof(s_badAllocLines[2]),
        "arena used %ldKB, unusable free %ldKB", mallocUsedKb, mallocFreeKb);
    std::snprintf(s_badAllocLines[3], sizeof(s_badAllocLines[3]),
        "GS queue %ldKB, VRAM %dKB",
        ps2_dbg_gs_queue_ram_bytes() / 1024, ps2_dbg_vram_current_kb());

    if (s_lastBadAllocRequest > 0)
    {
        std::snprintf(s_badAllocLines[4], sizeof(s_badAllocLines[4]),
            "request %uKB (%u bytes), caller %p",
            (unsigned)(s_lastBadAllocRequest / 1024u),
            (unsigned)s_lastBadAllocRequest,
            s_lastBadAllocCaller);
    }
    else
    {
        std::snprintf(s_badAllocLines[4], sizeof(s_badAllocLines[4]),
            "failed C++ request: size unavailable");
    }

    // Also to the log. The error screen needs a working renderer and a font;
    // debug.log survives cases where neither is still usable, and errors commit
    // to the card immediately.
    for (unsigned i = 0; i < 5; ++i)
        MC_LOG_ERROR("memory", "[PS2] oom: %s\n", s_badAllocLines[i]);

    // World creation is where the largest single allocations happen, and it is
    // the one path that can fail with the log's last line pointing at the wrong
    // place (see platform/WorldLoadTrace.h). The stage is held in RAM, so it is
    // still accurate here even when nothing since has reached the card.
    if (WorldLoadTrace::currentStage()[0] != '\0')
        MC_LOG_ERROR("memory", "[PS2] oom during world load: %s\n",
                     WorldLoadTrace::currentStage());
}

const char* ps2HeapLastBadAllocLine(unsigned index)
{
    return index < 5 ? s_badAllocLines[index] : "";
}

#endif // PS2_PLATFORM
