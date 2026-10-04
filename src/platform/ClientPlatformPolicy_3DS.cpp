#include "platform/ClientPlatformPolicy.h"

#include "net/minecraft/src/GameResources.h"
#include "pc/CrashHandler.h"

#include <3ds.h>
#include <citro3d.h>

namespace ClientPlatformPolicy
{
int initialWidth()
{
    // Phase-1 constant: the 3DS top screen is 400x240. lwjgl::Display does
    // declare getWidth()/getHeight(), but nothing owns them until the
    // citro3d renderer creates the framebuffer, so reading them pre-init
    // would report whatever a stub backend left behind -- the constant also
    // compiles without pulling Display into this target's link. Once the
    // renderer lands, lwjgl::Display owns this value the way the Wii's
    // wiigl_width() owns it there.
    return 400;
}

int initialHeight()
{
    // Top screen of the same 400x240 pair; see initialWidth() for why this
    // is a constant rather than a Display query.
    return 240;
}

std::string minecraftDirectory()
{
    return GameResources::getExeDir() + "/.minecraft";
}

bool saveConverterUsesSavesSubdirectory()
{
    return true;
}

void applyGameSettingsDefaults(GameSettings*)
{
}

void preloadStartupTextures(RenderEngine*)
{
}

void releaseWorldEntryAssets(RenderEngine*)
{
}

void shutdownFlush()
{
    // The last frame is submitted asynchronously (renderSubmitFrame ->
    // C3D_FrameEnd(0)) and only drains at the next frame's
    // C3D_FrameBegin(C3D_FRAME_SYNCDRAW); on shutdown there is no next frame.
    // SYNCDRAW performs a C3D_FrameSync before anything else, so this dummy
    // begin/end pair waits for the queue to finish -- after it, no GPU work
    // references game textures, and the teardown below can free them safely.
    // Same rationale (and same fix shape) as the CrashHandler_3ds copy; see
    // also shutdownServices() in main_3ds.cpp, which never runs on this path
    // because PLATFORM_EXIT_PROCESS_ON_SHUTDOWN has shutdownMinecraftApplet()
    // call exit(0) instead of returning to main().
    if (C3D_FrameBegin(C3D_FRAME_SYNCDRAW))
    {
        C3D_FrameEnd(0);
    }
}

int panoramaSampleGrid()
{
    // The PS2's 2x2 accumulation (24 draws total): every sample is its own
    // one-quad Tessellator::draw(), and on this backend each draw is a
    // staging-arena submit -- the desktop/Wii 8x8 grid bills the menu 384
    // submits per frame for a blur the 400x240 target cannot resolve anyway.
    return 2;
}

void shutdownFinalize()
{
    // This platform exits the process from inside the game's shutdown
    // (PLATFORM_EXIT_PROCESS_ON_SHUTDOWN): the exit(0) that follows never
    // returns to main(), so shutdownServices() -- whose gfxExit() is the
    // one join of libctru's GSP event thread any normal path performs --
    // never runs. newlib's exit() then hands control to libctru's
    // __libctru_exit, which unmaps the linear heap AND the entire
    // application heap (svcControlMemory MEMOP_FREE) before
    // svcExitProcess, with every other thread still alive. The GSP event
    // thread's 4 KiB stack lives in that heap (gspInit -> threadCreate,
    // gspgpu.c), so the moment the unmap lands it is a thread scheduled
    // on unmapped memory: the next GPU interrupt it handles faults in its
    // own prologue -- syncArbitrateAddress's push {lr} writing sp-4, data
    // abort, fault status "Translation - Section" (Luma dump 2026-10-03:
    // "go to the home menu, resume, exit the game" crashes). gfxExit() is
    // the canonical stop, the same one shutdownServices() would have
    // done: it frees the framebuffers and calls gspExit(), which clears
    // gspRunEvents, signals the event and threadJoin()s the thread before
    // anything unmaps. It is idempotent (screenFree == NULL guard), so the
    // gfxExit() the crash path already does in CrashHandler_3ds.cpp stays
    // safe. No fsExit() here on purpose: SD stays mounted so anything
    // running inside exit(0) can still write the log; the process end
    // reclaims the rest.
    if (C3D_FrameBegin(C3D_FRAME_SYNCDRAW))
    {
        C3D_FrameEnd(0);
    }
    gfxExit();
}

void reportCrash(const std::string& description)
{
    CrashHandler::Crash(description);
}
}
