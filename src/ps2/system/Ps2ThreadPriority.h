#pragma once

// EE thread priorities: lower number = higher priority, no time slicing. A
// thread only runs while every thread with a lower number is blocked, so any
// background worker that must keep up with hardware has to sit above main.
namespace Ps2ThreadPriority
{
// The kernel starts main at 0 (highest). Lowered in Ps2Bootstrap so I/O-bound
// workers can preempt the frame loop, which busy-waits on vsync.
// PS2SDK lwIP creates its tcpip thread at priority 0x58 (88).  Main must be
// lower priority (numerically larger) than lwIP or the busy frame loop can
// starve tcpip_init forever while ps2ipInit waits on its init semaphore.
constexpr int kMain = 96;
// Music stream thread. Blocked on IOP RPC almost all the time; when it wakes it
// pushes one chunk and sleeps again, so preempting main costs microseconds.
constexpr int kMusicStream = 32;
// Application network workers should preempt main, but remain below lwIP's
// tcpip thread (88).  This ordering lets the core stack run whenever a socket
// worker needs it: lwIP 88 > app network 92 > main 96.
constexpr int kNetwork = 92;
// Background texture asset I/O. It must run above main so it can issue a USB
// request while the frame loop is active, but below music streaming.
constexpr int kAssetIo = 48;
}
