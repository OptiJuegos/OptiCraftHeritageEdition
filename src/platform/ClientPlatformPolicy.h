#pragma once

#include <string>

class GameSettings;
class RenderEngine;

namespace ClientPlatformPolicy
{
    int initialWidth();
    int initialHeight();
    std::string minecraftDirectory();
    bool saveConverterUsesSavesSubdirectory();
    void applyGameSettingsDefaults(GameSettings* settings);
    void preloadStartupTextures(RenderEngine* renderEngine);
    void releaseWorldEntryAssets(RenderEngine* renderEngine);
    // Called once from Minecraft::shutdownMinecraftApplet() before any
    // teardown touches GPU-owned memory. Only the 3DS needs it: the game
    // exits through exit(0) there (PLATFORM_EXIT_PROCESS_ON_SHUTDOWN), so
    // nothing returns to the entry point, and the last submitted citro3d
    // frame must be drained before GLAllocation::deleteTexturesAndDisplayLists()
    // frees C3D_Tex storage the in-flight frame may still be sampling.
    void shutdownFlush();
    // Called immediately before PLATFORM_EXIT_PROCESS_ON_SHUTDOWN ends the
    // process from inside Minecraft::shutdownMinecraftApplet(). Only the 3DS
    // needs it: exit(0) lands in libctru's __libctru_exit, which unmaps the
    // whole application heap with svcControlMemory(MEMOP_FREE) *before*
    // svcExitProcess -- while every other thread in the process is still
    // alive. libctru's GSP event thread (its stack is malloc'd from that
    // heap, gspgpu.c) is normally joined by gspExit() via gfxExit(), but the
    // exit(0) path never returns to main_3ds.cpp's shutdownServices(), so
    // nothing joins it and the first GSP interrupt it handles after the
    // unmap faults writing its dead stack (data abort, "Translation -
    // Section"; Luma dump 2026-10-03, crash on exiting from the HOME menu).
    // The 3DS implementation stops and joins that thread; PC and Wii exit
    // through runtimes that only reclaim a dying process's memory, so there
    // it is a no-op.
    void shutdownFinalize();
    int panoramaSampleGrid();
    void reportCrash(const std::string& description);
}
