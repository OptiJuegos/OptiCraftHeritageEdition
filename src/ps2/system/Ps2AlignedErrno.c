/*
 * PS2SDK libkernel's errno archive member can become only 2-byte aligned after
 * libps2ip changes .data ordering. The R5900 requires aligned 32-bit accesses,
 * so networking builds provide errno from an explicitly aligned application
 * section instead. This also prevents libkernel's errno.o from being extracted.
 */
__attribute__((used, externally_visible, aligned(16),
               section(".data.ps2_aligned_errno")))
int errno = 0;
