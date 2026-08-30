#pragma once

// Heap tracing for the Wii port.
//
// This exists because "Out of memory" on its own cannot separate the three cases
// that matter, and this port has already spent sessions on the wrong one:
//
//   in use high, free low   -> genuinely exhausted; cut what is allocated
//   in use low,  free high  -> fragmentation; the request cannot be PLACED
//   total small             -> the arena is not the one we think it is
//
// Native GX display-list memory is reported alongside the heap so world-load
// milestones and steady-state memory can be compared directly.
//
// What is printed
// ---------------
// With MALLOC_MEM2 enabled, libogc's sbrk consumes MEM1 first and then MEM2.
// Heap headroom therefore consists of free bytes already inside malloc plus the
// unclaimed portions of both arenas.
//
//   free    total free bytes inside the committed malloc arena (fordblks)
//   blocks  how many pieces that free space is in (ordblks)
//   top     the top chunk (keepcost)
//   arena1  bytes sbrk can still claim from MEM1
//   arena2  bytes sbrk can still claim from MEM2
//
// free/blocks is the ratio that separates "exhausted" from "fragmented": a large
// free total split across hundreds of small blocks with a small top chunk is
// fragmentation, and cutting allocations further will not help.

// Print one tagged heap line. Tag is padded so a log reads as a column.
void wiiHeapReport(const char *tag);

// Snapshot the heap at the point a C++ allocation failed.  The OOM handler
// subsequently releases the world, so this has to happen before cleanup if the
// in-game error screen is to show useful numbers rather than an empty heap.
void wiiHeapCaptureBadAlloc();

// One of the short diagnostic lines captured by wiiHeapCaptureBadAlloc().
// Returns an empty string when no allocation failure has been captured yet.
const char *wiiHeapLastBadAllocLine(unsigned index);
