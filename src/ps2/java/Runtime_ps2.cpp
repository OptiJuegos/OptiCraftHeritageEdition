#ifdef PS2_PLATFORM

#include "platform/Log.h"
#include "java/Runtime.h"

#include <unistd.h>
#include <stdint.h>
#include <cstdio>
#include <malloc.h>

Runtime Runtime::instance;

namespace
{
    static uintptr_t s_heapBase = 0;
    static bool s_heapBaseLocked = false;

    uintptr_t ps2HeapEnd()
    {
        return reinterpret_cast<uintptr_t>(sbrk(0));
    }

    uintptr_t ps2StackNow()
    {
        volatile char stackProbe = 0;
        return reinterpret_cast<uintptr_t>(&stackProbe);
    }

    uintptr_t ps2HeapBase()
    {
        uintptr_t heap = ps2HeapEnd();
        if (!s_heapBaseLocked && (s_heapBase == 0 || heap < s_heapBase))
            s_heapBase = heap;
        return s_heapBase;
    }

    long_t ps2ApproxFreeHeapBytes()
    {
        uintptr_t heap = ps2HeapEnd();
        uintptr_t stack = ps2StackNow();

        // PCSX2/ps2sdk configurations can report EE heap pointers above the
        // classic 0x02000000 limit. Use the live stack when it is sane, else
        // fall back to a conservative 64 MB virtual top so the log does not
        // print 0/0 forever.
        const uintptr_t eeTop32 = 0x02000000u;
        const uintptr_t eeTop64 = 0x04000000u;
        uintptr_t top = 0;
        if (stack > heap && stack < 0x80000000u)
            top = stack;
        else if (heap < eeTop32)
            top = eeTop32;
        else
            top = eeTop64;

        intptr_t freeBytes = static_cast<intptr_t>(top - heap);
        const intptr_t guard = 512 * 1024;
        if (freeBytes > guard)
            freeBytes -= guard;
        else
            freeBytes = 0;

        return static_cast<long_t>(freeBytes);
    }

    long_t ps2ApproxUsedHeapBytes()
    {
        uintptr_t base = ps2HeapBase();
        uintptr_t heap = ps2HeapEnd();
        if (heap <= base)
            return 0;
        return static_cast<long_t>(heap - base);
    }
}

extern "C" void ps2_dbg_init_memory()
{
    s_heapBase = ps2HeapEnd();
    s_heapBaseLocked = true;
    MC_LOG_INFO("game", "[PS2] memory baseline heap=0x%08x stack=0x%08x\n",
           (unsigned)s_heapBase, (unsigned)ps2StackNow());
}

extern "C" long ps2_dbg_ram_free_kb()
{
    return static_cast<long>(ps2ApproxFreeHeapBytes() / 1024LL);
}

extern "C" long ps2_dbg_ram_used_kb()
{
    return static_cast<long>(ps2ApproxUsedHeapBytes() / 1024LL);
}

// Real allocator statistics (newlib mallinfo): distinguishes a true leak from
// fragmentation. uordblks = bytes handed out and not freed; fordblks = bytes
// sitting free INSIDE the malloc arena (invisible to the sbrk-gap "free=" —
// if free= falls while mallocUsed stays flat, the arena is fragmenting, not
// leaking).
extern "C" long ps2_dbg_malloc_used_kb()
{
    struct mallinfo mi = mallinfo();
    return (long)((unsigned)mi.uordblks / 1024u);
}

extern "C" long ps2_dbg_malloc_free_kb()
{
    struct mallinfo mi = mallinfo();
    return (long)((unsigned)mi.fordblks / 1024u);
}

// Number of free chunks in the arena. fordblks says how much free memory the
// arena holds; this says how many pieces it is in. A large fordblks split over
// many ordblks is fragmentation, and a big contiguous request (World's 128KiB
// lightUpdateBlockList, a chunk's 32KiB block array) can fail or fall through
// to sbrk while "free=" still looks healthy.
extern "C" long ps2_dbg_malloc_free_blocks()
{
    struct mallinfo mi = mallinfo();
    return (long)mi.ordblks;
}

extern "C" unsigned ps2_dbg_heap_end()
{
    return static_cast<unsigned>(ps2HeapEnd());
}

extern "C" unsigned ps2_dbg_stack_now()
{
    return static_cast<unsigned>(ps2StackNow());
}

Runtime& Runtime::getRuntime()
{
    ps2HeapBase();
    return instance;
}

long_t Runtime::maxMemory()
{
    return 32LL * 1024LL * 1024LL;
}

long_t Runtime::totalMemory()
{
    long_t used = ps2ApproxUsedHeapBytes();
    long_t free = ps2ApproxFreeHeapBytes();
    return used + free;
}

long_t Runtime::freeMemory()
{
    return ps2ApproxFreeHeapBytes();
}

#endif // PS2_PLATFORM
