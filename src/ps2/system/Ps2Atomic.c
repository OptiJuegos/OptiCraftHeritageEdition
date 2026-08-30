// Ps2Atomic.c — EE implementations of the __atomic_* libcalls GCC cannot inline.
//
// The ps2dev toolchain is built with -mno-llsc because the R5900 has no usable
// LL/SC pair, so every atomic read-modify-write becomes an out-of-line libcall
// instead of the usual retry loop. std::atomic<BufferedImage *>::exchange() in
// ThreadDownloadImageData uses exchange, while the network thread counters use
// fetch-add and fetch-sub.
//
// Linking the toolchain's libatomic would resolve the symbol, but its generic
// path takes a pthread_mutex from the pthreads-embedded port whose static locks
// only become valid after pte_osInit(), which nothing in this build calls -- and
// ~ThreadDownloadImageData reaches the exchange on every skin release, so that
// path would run for real. The EE is single core, so masking interrupts across
// the load/store pair is both cheaper and correct: it locks out interrupt
// handlers and the thread preemption they drive.

#ifdef PS2_PLATFORM

#include <kernel.h>
#include <stdint.h>

uint32_t __atomic_exchange_4(volatile void *ptr, uint32_t value, int memorder)
{
    volatile uint32_t *slot = (volatile uint32_t *)ptr;
    uint32_t previous;
    int reenable;

    // Single core and in-order: no memory ordering weaker than the critical
    // section itself is observable, so the model the caller asked for is moot.
    (void)memorder;

    // DIntr() reports whether interrupts were enabled on entry. Restore them
    // only if they were, so a caller that already masked them stays masked.
    reenable = DIntr();

    previous = *slot;
    *slot = value;

    if (reenable)
        EIntr();

    return previous;
}

uint32_t __atomic_fetch_add_4(volatile void *ptr, uint32_t value, int memorder)
{
    volatile uint32_t *slot = (volatile uint32_t *)ptr;
    uint32_t previous;
    int reenable;

    (void)memorder;
    reenable = DIntr();

    previous = *slot;
    *slot = previous + value;

    if (reenable)
        EIntr();

    return previous;
}

uint32_t __atomic_fetch_sub_4(volatile void *ptr, uint32_t value, int memorder)
{
    volatile uint32_t *slot = (volatile uint32_t *)ptr;
    uint32_t previous;
    int reenable;

    (void)memorder;
    reenable = DIntr();

    previous = *slot;
    *slot = previous - value;

    if (reenable)
        EIntr();

    return previous;
}

#endif // PS2_PLATFORM
