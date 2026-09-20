#pragma once

#include <string>
#include <atomic>
#include <vector>

#include "java/Type.h"
#include "java/String.h"
#include "java/System.h"
#include "platform/PlatformConfig.h"
#include "platform/PlatformTuning.h"

#if !PLATFORM_PS2
struct SDL_Window;
#endif

class File;

class PlayerController;
class World;
class WorldSettings;
class RenderGlobal;
class EntityPlayerSP;
class EntityLiving;
class EffectRenderer;
class RenderEngine;
class FontRenderer;
class GuiScreen;
class LoadingScreenRenderer;
class EntityRenderer;
class GuiAchievement;
class GuiIngame;
class ModelBiped;
class MovingObjectPosition;
class GameSettings;
class SoundManager;
class MouseHelper;
class TexturePackList;
class ISaveFormat;
class StatFileWriter;
class TextureWaterFX;
class TextureLavaFX;
class ThreadDownloadResources;
class ThreadSleepForever;
class NetClientHandler;
class Session;
class Timer;
class UnexpectedThrowable;

class Minecraft;
namespace LegacyStartup
{
    void run(Minecraft* minecraft);
}

// net.minecraft.client.Minecraft + net.minecraft.src.MinecraftImpl (merged)
class Minecraft
{
public:
    struct WorldLoadOptions
    {
        bool naturalMobSpawningEnabled = true;
    };

    Minecraft(int_t width, int_t height, bool flag);
    ~Minecraft();

    static void start(const jstring *username, const jstring *sessionId);
    static Minecraft *getMinecraft();

    void run();
    void startGame();
    void runTick();
    void shutdown();

    void displayGuiScreen(GuiScreen *guiscreen);
    void purgeOwnedGuiScreens(); // free abandoned menu screens the Java GC would have collected
    void displayInGameMenu();
    void setIngameFocus();
    void setIngameNotInFocus();
    void toggleFullscreen();
    void refreshResources();
    FontRenderer *getStandardGalacticFontRenderer();

    void onMinecraftCrash(UnexpectedThrowable *unexpectedthrowable);
    virtual void displayUnexpectedThrowable(UnexpectedThrowable *unexpectedthrowable);

    void setServer(const std::string &s, int_t i);

    ISaveFormat *getSaveLoader();

    bool isMultiplayerWorld();
    bool isDebugKeepInventoryEnabled();

    void startWorld(const std::string &s, const std::string &s1, long_t l);
    void startWorld(const std::string &s, const std::string &s1, WorldSettings *settings);
    void startWorld(ISaveFormat *saveFormat, const std::string &s, const std::string &s1, WorldSettings *settings);
    void startWorld(ISaveFormat *saveFormat, const std::string &s, const std::string &s1, WorldSettings *settings,
                    const WorldLoadOptions &options);
    void usePortal(int_t targetDimension);
    void changeWorld1(World *world);
    void changeWorld2(World *world, const std::string &s);
    void changeWorld(World *world, const std::string &s, EntityPlayerSP *entityplayer);
    void respawn(bool flag, int_t i, bool copyPlayer = false);

    void installResource(const std::string &s, const std::string &file);

    std::string getDebugLine1(); // func_6241_m
    std::string getDebugLine2(); // func_6262_n
    std::string getDebugLine3(); // func_21002_o
    std::string getDebugLine4(); // func_6245_o

    static bool isGuiEnabled();
    static bool isFancyGraphicsEnabled();
    static bool isAmbientOcclusionEnabled();
    static bool isDebugInfoEnabled();

    bool lineIsCommand(const std::string &s);

    static File *getMinecraftDir();

    NetClientHandler *getSendQueue();

    // Static fields
    // Vanilla reserves 10 MB so the out-of-memory crash handler still has heap
    // to build the crash screen. Here this storage is static BSS, therefore the
    // memset in freeMemoryForCrash() never releases it. On fixed-memory consoles
    // it only inflates the DOL and steals MEM1 before the allocator starts.
    // Keep a token buffer for the legacy call site, not a fake 10 MB reserve.
#if PLATFORM_HAS_LIMITED_MEMORY
    static byte_t field_28006_b[1];
#else
    static byte_t field_28006_b[0xa00000];
#endif
    static long_t frameTimes[512];
    static long_t tickTimes[512];
    static int_t numRecordedFrameTimes;
#if PLATFORM_CLIENT_PAID_CHECK
    static std::atomic<long_t> hasPaidCheckTime;
#else
    static long_t hasPaidCheckTime;
#endif

    // Public fields
    PlayerController *playerController;
    int_t displayWidth;
    int_t displayHeight;
    World *theWorld;
    RenderGlobal *renderGlobal;
    EntityPlayerSP *thePlayer;
    EntityLiving *renderViewEntity;
    EffectRenderer *effectRenderer;
    Session *session;
    std::string minecraftUri;
    bool hideQuitButton;
    volatile bool isGamePaused;
    RenderEngine *renderEngine;
    FontRenderer *fontRenderer;
    FontRenderer *standardGalacticFontRenderer;
    GuiScreen *currentScreen;
    std::vector<GuiScreen *> ownedGuiScreens;    // every screen we showed; purged when the stack is abandoned
    std::vector<GuiScreen *> guiScreensToDelete; // deferred frees; flushed at runTick start (never delete a screen mid-handler)
    std::vector<World *> worldsToDelete;         // deferred world frees; flushed at runTick start (changeWorld can run mid-tick via MP packet handling -> never delete the world being ticked)
    LoadingScreenRenderer *loadingScreen;
    EntityRenderer *entityRenderer;
    int_t ticksRan;
    GuiAchievement *guiAchievement;
    GuiIngame *ingameGUI;
    bool skipRenderWorld;
    ModelBiped *field_9242_w;
    MovingObjectPosition *objectMouseOver;
    GameSettings *gameSettings;
    SoundManager *sndManager;
    MouseHelper *mouseHelper;
    TexturePackList *texturePackList;
    StatFileWriter *statFileWriter;
    volatile bool running;
    std::string debug;
    // Frame-time split around the buffer swap (lwjgl::Display::update()):
    // gpuUsagePercent is the share of the frame spent waiting on that swap
    // (vsync/present, a stand-in for GPU-bound time on platforms with no real
    // GPU utilization counter), cpuUsagePercent is the rest (tick + render
    // submission). Updated once a second, alongside `debug`. Shown on F3.
    float cpuUsagePercent;
    float gpuUsagePercent;
    bool inGameHasFocus;
    bool isRaining;
#if !PLATFORM_PS2
    SDL_Window *window;
#endif

private:
    friend void LegacyStartup::run(Minecraft* minecraft);

    void loadScreen();
    void drawSplashScreenTexturedModalRect(int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1);
    void checkGLError(const std::string &s);
    void shutdownMinecraftApplet();
    void screenshotListener();
    void displayDebugInfo(long_t l);
    void clickMouse(int_t i, bool flag);
    void clickMouse(int_t i);
    void clickMiddleMouseButton();
    void startCheckHasPaidThread();
    void freeMemoryForCrash();
    void forceReload();
    void convertMapFormat(const std::string &s, const std::string &s1);
    void preloadWorld(const std::string &s);
    void resize(int_t i, int_t j);

    static File *getAppDir(const std::string &s);
    static Minecraft *theMinecraft;
    static File *minecraftDir;

    bool fullscreen;
    bool hasCrashed;
    Timer *timer;
    ThreadDownloadResources *downloadResourcesThread;
    ThreadSleepForever *timerHackThread;
    int_t leftClickCounter;
    int_t rightClickDelayTimer;
    int_t tempDisplayWidth;
    int_t tempDisplayHeight;
    File *mcDataDir;
    ISaveFormat *saveLoader;
#ifdef PS2_PLATFORM
    std::string worldSaveRoot;
#endif
    std::string serverName;
    int_t serverPort;
    TextureWaterFX *textureWaterFX;
    TextureLavaFX *textureLavaFX;
    bool isTakingScreenshot;
    long_t prevFrameTime;
    int_t mouseTicksRan;
    long_t systemTime;
    int_t joinPlayerCounter;
};
