#pragma once
#ifdef WII_PLATFORM

// GXRModeObj is a typedef of `struct _gx_rmodeobj`, not a struct tag, so it
// cannot be forward-declared as `struct GXRModeObj`. Pull in the real header.
#include <ogc/gx_struct.h>

// Low-level Wii log sink used only by the unified logger. Game code must use
// MC_LOG_* from platform/Log.h instead of calling this directly.
extern "C" void wiiPlatformLogWrite(const char* line);

// Called once, by wiigl_init, at the moment GX takes ownership of the
// framebuffer. After this the Wii log sink stops writing to the text console, because that
// console renders into the very buffer the game is now flipping -- see the
// channel 1 comment in WiiEarlyInit.cpp. Everything else keeps working.
void wiiPlatformLogEndBootPhase();

// Mounts the SD/USB filesystem, at most once, and reports whether it worked.
//
// Safe and cheap to call from anywhere, including from a static initializer:
// the first call does the work, every later call returns the cached result.
// See WiiEarlyInit.cpp for why this cannot simply live in main().
bool wiiEnsureStorage();

// The install directory: where assets are read from and worlds are written.
// "sd:/apps/BetaPlusPlus", "usb:/apps/BetaPlusPlus", or wherever the loader
// actually started the DOL from -- resolved once by wiiEnsureStorage(), which
// this calls for you.
//
// SD and USB are both supported and neither is a build-time choice; see the
// device-selection comment in WiiEarlyInit.cpp for how one is picked.
//
// Always returns a usable absolute path with a device prefix and no trailing
// slash, even when nothing mounted -- callers get a path that fails to open
// rather than a null pointer to check.
//
// Returns a `const char *` and not a std::string on purpose: it is read from
// static initialisers, and a namespace-scope std::string holding the answer
// would be constructed *after* the prioritised constructor that fills it in,
// blanking it. GameResources::getExeDir() copies it into a std::string at the
// point of use, which is safe.
const char *wiiGetAppDir();

// True when the install directory really holds a staged data/ tree. False means
// the game will fail on its first asset load, and the caller is expected to say
// so in a way the user can act on rather than dying inside a texture load.
bool wiiHasGameData();

// Brings up video and libogc's text console, at most once. Called automatically
// before main() so that early failures are readable on the TV instead of being
// a black screen; safe to call again.
void wiiEnsureEarlyVideo();

// The video mode and the first framebuffer chosen by the early init. gx_wii.cpp
// adopts both instead of allocating its own, so the diagnostic console costs no
// extra memory once the renderer takes over.
GXRModeObj *wiiGetRenderMode();
void       *wiiGetEarlyFramebuffer();

// Upper bound sampled during early init for the malloc heap. libogc grows sbrk
// through MEM1 first and, when MALLOC_MEM2 is enabled, continues into MEM2. The
// value includes bytes already committed to malloc plus the still-unclaimed
// portions of both arenas. java/Runtime uses it as the F3 max-memory denominator.
u32 wiiGetHeapCeiling();

// Actual bytes currently committed to the malloc heap. Do not use
// mallinfo().arena for this on Wii: after sbrk transitions from MEM1 to MEM2
// newlib sees a non-contiguous address jump and arena becomes artificially huge.
u32 wiiGetHeapCommitted();

#endif // WII_PLATFORM
