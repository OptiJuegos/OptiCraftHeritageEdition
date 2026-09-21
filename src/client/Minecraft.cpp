#include "net/minecraft/src/WorldSettings.h"
#include "net/minecraft/src/WorldType.h"
#include "net/minecraft/src/WorldInfo.h"
#include "client/Minecraft.h"
#ifdef PS2_PLATFORM
#include "ps2/storage/save/Ps2SaveStorage.h"
#include "net/minecraft/src/GuiStorageMessage.h"
#endif
#include "platform/Log.h"
#include "platform/ConsoleAspectRatio.h"
#include "platform/PlatformTuning.h"
#include "platform/PlatformCompat.h"
#include "platform/world/StreamingFrameBudget.h"
#include "platform/ClientPlatformPolicy.h"
#include "platform/Diagnostics.h"
#include "platform/Profiler.h"
#include "platform/WorldLoadTrace.h"
#include "client/ClientProfiler.h"
#include "mods/ModManager.h"

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <memory>
#include <unordered_set>
#include <typeinfo>
#if defined(_WIN32) && !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
#include <windows.h>
#endif
#include "platform/RenderAPI.h"
#if PLATFORM_CLIENT_PAID_CHECK
#include <thread>
#endif
#include "pc/lwjgl/Display.h"
#include "pc/lwjgl/Keyboard.h"
#include "pc/lwjgl/Mouse.h"

#include "java/File.h"
#include "java/String.h"
#include "java/System.h"
#include "java/Runtime.h"

#include "net/minecraft/src/AchievementList.h"
#include "net/minecraft/src/AxisAlignedBB.h"
#include "net/minecraft/src/BiomeGenBase.h"
#include "net/minecraft/src/Block.h"
#include "net/minecraft/src/Container.h"
#include "net/minecraft/src/ChunkBlockMap.h"
#include "net/minecraft/src/EnumOptionsMappingHelper.h"
#include "net/minecraft/src/Item.h"
#include "net/minecraft/src/ItemBlock.h"
#include "net/minecraft/src/Material.h"
#include "net/minecraft/src/BlockGrass.h"
#include "net/minecraft/src/ChunkCoordinates.h"
#include "net/minecraft/src/ChunkProviderLoadOrGenerate.h"
#include "net/minecraft/src/ChunkProvider.h"
#include "net/minecraft/src/ColorizerFoliage.h"
#include "net/minecraft/src/ColorizerGrass.h"
#include "net/minecraft/src/ColorizerWater.h"
#include "net/minecraft/src/EffectRenderer.h"
#include "net/minecraft/src/EntityClientPlayerMP.h"
#include "net/minecraft/src/EntityPlayer.h"
#include "net/minecraft/src/EntityPlayerSP.h"
#include "net/minecraft/src/EntityRenderer.h"
#include "net/minecraft/src/EnumMovingObjectType.h"
#include "net/minecraft/src/EnumOptions.h"
#include "net/minecraft/src/FontRenderer.h"
#include "net/minecraft/src/GameResources.h"
#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/Config.h"
#include "net/minecraft/src/GuiParticle.h"
#include "net/minecraft/src/InventoryPlayer.h"
#include "net/minecraft/src/KeyBinding.h"
#include "net/minecraft/src/GLAllocation.h"
#include "net/minecraft/src/GuiAchievement.h"
#include "net/minecraft/src/GuiChat.h"
#include "net/minecraft/src/GuiConflictWarning.h"
#include "net/minecraft/src/MinecraftException.h"
#include "net/minecraft/src/GuiConnecting.h"
#include "net/minecraft/src/GuiErrorScreen.h"
#include "net/minecraft/src/GuiGameOver.h"
#include "net/minecraft/src/GuiIngame.h"
#include "net/minecraft/src/GuiIngameMenu.h"
#include "net/minecraft/src/GuiInventory.h"
#include "net/minecraft/src/StringTranslate.h"
#include "net/minecraft/src/GuiContainerCreative.h"
#include "net/minecraft/src/GuiMainMenu.h"
#include "net/minecraft/src/GuiSleepMP.h"
#include "net/minecraft/src/GuiUnused.h"
#include "net/minecraft/src/IChunkProvider.h"
#include "net/minecraft/src/ISaveFormat.h"
#include "net/minecraft/src/ISaveHandler.h"
#include "net/minecraft/src/ItemRenderer.h"
#include "net/minecraft/src/ItemStack.h"
#include "net/minecraft/src/LoadingScreenRenderer.h"
#include "net/minecraft/src/MathHelper.h"
#include "net/minecraft/src/ModelBiped.h"
#include "net/minecraft/src/MouseHelper.h"
#include "net/minecraft/src/MovementInputFromOptions.h"
#include "net/minecraft/src/MovingObjectPosition.h"
#include "net/minecraft/src/NetClientHandler.h"
#include "net/minecraft/src/OpenGlHelper.h"
#include "net/minecraft/src/PlayerController.h"
#include "net/minecraft/src/PlayerControllerMP.h"
#include "net/minecraft/src/PlayerControllerTest.h"
#include "net/minecraft/src/RenderBlocks.h"
#include "net/minecraft/src/RenderEngine.h"
#include "net/minecraft/src/RenderGlobal.h"
#include "net/minecraft/src/RenderManager.h"
#include "net/minecraft/src/AnvilSaveConverter.h"
#include "net/minecraft/src/ScaledResolution.h"
#include "net/minecraft/src/ScreenShotHelper.h"
#include "net/minecraft/src/Session.h"
#include "net/minecraft/src/SoundManager.h"
#include "net/minecraft/src/StatFileWriter.h"
#include "net/minecraft/src/StatList.h"
#include "net/minecraft/src/StatStringFormatKeyInv.h"
#include "net/minecraft/src/Teleporter.h"
#include "net/minecraft/src/Tessellator.h"
#include "net/minecraft/src/legacy/startup/StartupPresentation.h"
#include "net/minecraft/src/legacy/LegacyDebugOptions.h"

namespace
{
#if defined(_WIN32) && !defined(PS2_PLATFORM) && !defined(WII_PLATFORM)
    bool validateProcessHeap(const char *stage)
    {
        const char *label = stage != nullptr ? stage : "<unknown>";
        DWORD count = GetProcessHeaps(0, nullptr);
        if (count == 0)
        {
            MC_LOG_ERROR("heap", "GetProcessHeaps failed at %s error=%lu\n", label, (unsigned long)GetLastError());
            return false;
        }

        // Keep the validator allocation-free: allocating a std::vector here can itself
        // trip over already-corrupted heap metadata and hide the phase that damaged it.
        HANDLE heaps[128] = {};
        const DWORD capacity = static_cast<DWORD>(sizeof(heaps) / sizeof(heaps[0]));
        DWORD written = GetProcessHeaps(capacity, heaps);
        if (written == 0)
        {
            MC_LOG_ERROR("heap", "GetProcessHeaps enumeration failed at %s error=%lu\n", label, (unsigned long)GetLastError());
            return false;
        }
        if (written > capacity)
        {
            MC_LOG_ERROR("heap", "Too many process heaps at %s count=%lu capacity=%lu\n",
                label, (unsigned long)written, (unsigned long)capacity);
            written = capacity;
        }

        bool allValid = true;
        for (DWORD i = 0; i < written; ++i)
        {
            HANDLE heap = heaps[i];
            if (heap == nullptr)
                continue;
            if (HeapValidate(heap, 0, nullptr) == FALSE)
            {
                MC_LOG_ERROR("heap", "Heap invalid at %s index=%lu handle=%p processHeap=%d\n",
                    label, (unsigned long)i, static_cast<void *>(heap), heap == GetProcessHeap() ? 1 : 0);
                allValid = false;
            }
        }
        return allValid;
    }
#else
    bool validateProcessHeap(const char *) { return true; }
#endif

#if PLATFORM_DEFER_PORTAL_TRANSITION
    bool pendingPortalTransition = false;
    bool runningPortalTransition = false;
    int_t pendingPortalTargetDimension = 0;
#endif

    void configureChunkProviderCache(IChunkProvider *provider, int_t chunkX, int_t chunkZ, int_t renderDistance)
    {
        if (provider == nullptr)
            return;

        if (ChunkProviderLoadOrGenerate *cplg = dynamic_cast<ChunkProviderLoadOrGenerate *>(provider))
        {
            cplg->setChunkLoadRadiusFromRenderDistance(renderDistance);
            cplg->setCurrentChunkOver(chunkX, chunkZ);
            return;
        }

        if (ChunkProvider *cp = dynamic_cast<ChunkProvider *>(provider))
        {
            cp->setChunkLoadRadiusFromRenderDistance(renderDistance);
            cp->setCurrentChunkOver(chunkX, chunkZ);
        }
    }
}

#include "net/minecraft/src/TextureCompassFX.h"
#include "net/minecraft/src/TextureFlamesFX.h"
#include "net/minecraft/src/TextureLavaFX.h"
#include "net/minecraft/src/TextureLavaFlowFX.h"
#include "net/minecraft/src/TexturePackList.h"
#include "net/minecraft/src/TexturePortalFX.h"
#include "net/minecraft/src/TextureWatchFX.h"
#include "net/minecraft/src/TextureWaterFX.h"
#include "net/minecraft/src/TextureWaterFlowFX.h"
#include "java/JavaNetwork.h"
#include "net/minecraft/src/ThreadDownloadResources.h"
#include "net/minecraft/src/ThreadSleepForever.h"
#include "net/minecraft/src/Timer.h"
#include "net/minecraft/src/UnexpectedThrowable.h"
#include "net/minecraft/src/Vec3D.h"
#include "net/minecraft/src/World.h"

// Client boot breadcrumbs use one neutral category. Platform log sinks decide
// whether to mirror them to a persistent boot log.
#define PLATFORM_BOOT_LOG(...) MC_LOG_INFO("client.boot", __VA_ARGS__)
#define PLATFORM_BOOT_PREFIX "[CLIENT]"
#include "net/minecraft/src/WorldProvider.h"
#include "net/minecraft/src/WorldRenderer.h"
#include "net/minecraft/src/WorldClient.h"

// ─── Static members ───────────────────────────────────────────────────────────

#if PLATFORM_HAS_LIMITED_MEMORY
byte_t  Minecraft::field_28006_b[1] = {};
#else
byte_t  Minecraft::field_28006_b[0xa00000] = {};
#endif
long_t  Minecraft::frameTimes[512]          = {};
long_t  Minecraft::tickTimes[512]           = {};
int_t   Minecraft::numRecordedFrameTimes    = 0;
#if PLATFORM_CLIENT_PAID_CHECK
std::atomic<long_t> Minecraft::hasPaidCheckTime{0L};
#else
long_t Minecraft::hasPaidCheckTime = 0L;
#endif
Minecraft *Minecraft::theMinecraft          = nullptr;
File      *Minecraft::minecraftDir          = nullptr;

// ─── Constructor / Destructor ─────────────────────────────────────────────────

Minecraft::Minecraft(int_t width, int_t height, bool flag) :
    playerController(nullptr),
    displayWidth(width),
    displayHeight(height),
    theWorld(nullptr),
    renderGlobal(nullptr),
    thePlayer(nullptr),
    renderViewEntity(nullptr),
    effectRenderer(nullptr),
    session(nullptr),
    minecraftUri(),
    // Java 1.2.5 declares Minecraft.hideQuitButton = false and only MinecraftApplet
    // raises it for an embedded run without stand-alone=true. Defaulting to true here
    // put the stand-alone menu on the applet layout: a full-width Options button at
    // y+72, no Quit button, and GuiButtonLanguage at y+84 overlapping it.
    hideQuitButton(false),
    isGamePaused(false),
    renderEngine(nullptr),
    fontRenderer(nullptr),
    standardGalacticFontRenderer(nullptr),
    currentScreen(nullptr),
    loadingScreen(nullptr),
    entityRenderer(nullptr),
    ticksRan(0),
    guiAchievement(nullptr),
    ingameGUI(nullptr),
    skipRenderWorld(false),
    field_9242_w(nullptr),
    objectMouseOver(nullptr),
    gameSettings(nullptr),
    sndManager(nullptr),
    mouseHelper(nullptr),
    texturePackList(nullptr),
    running(true),
    debug(),
    cpuUsagePercent(0.0f),
    gpuUsagePercent(0.0f),
    inGameHasFocus(false),
    isRaining(false),
    fullscreen(flag),
    hasCrashed(false),
    timer(nullptr),
    downloadResourcesThread(nullptr),
    timerHackThread(nullptr),
    leftClickCounter(0),
    rightClickDelayTimer(0),
    tempDisplayWidth(width),
    tempDisplayHeight(height),
    mcDataDir(nullptr),
    saveLoader(nullptr),
    statFileWriter(nullptr),
    serverName(),
    serverPort(0),
    textureWaterFX(nullptr),
    textureLavaFX(nullptr),
    isTakingScreenshot(false),
    prevFrameTime(-1L),
    mouseTicksRan(0),
    systemTime(System::currentTimeMillis()),
    joinPlayerCounter(0)
{
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " ctor: static init begin\n");
    Material::initialize();
    Block::initialize();
    ChunkBlockMap::initialize();
    Item::initItems();
    WorldType::initialize();
    BiomeGenBase::initialize();
    AchievementList::initialize();
    EnumOptionsMappingHelper::initialize();
    Session::initialize();
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " ctor: static init done, allocating subsystems\n");

    timer        = new Timer(20.0f);
    loadingScreen = new LoadingScreenRenderer(this);
    guiAchievement = new GuiAchievement(this);
    field_9242_w   = new ModelBiped(0.0f);
    sndManager     = new SoundManager();
    textureWaterFX = new TextureWaterFX();
    textureLavaFX  = new TextureLavaFX();

    StatList::initStats();
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " ctor: done\n");

    theMinecraft = this;

    // Desktop only. This is Java's timer-granularity hack: a thread parked in
    // Thread.sleep(Long.MAX_VALUE) purely to keep Windows' system timer at 1 ms
    // resolution. It buys nothing on a console, and on the Wii it actively hangs
    // the boot -- std::thread over libogc plus a 0x7fffffff-millisecond sleep
    // (~24 days, which overflows on the way to nanoseconds) turns run() into a
    // hot loop that starves the main thread. Symptom: the game stops dead
    // between the constructor and startGame().
#if PLATFORM_CLIENT_TIMER_HACK_THREAD
    timerHackThread = new ThreadSleepForever(this, "Timer hack thread");
#endif
}

Minecraft::~Minecraft()
{
    // Stop every worker before releasing objects they can reference. Java's GC
    // kept those graphs alive implicitly; C++ needs an explicit shutdown order.
    running = false;

    delete timerHackThread;
    timerHackThread = nullptr;

    if (downloadResourcesThread != nullptr)
    {
        try { downloadResourcesThread->closeMinecraft(); } catch (...) {}
        delete downloadResourcesThread;
        downloadResourcesThread = nullptr;
    }

    // StatStringFormatKeyInv stores a raw Minecraft*. Drop it while `this` is
    // still valid; setStatStringFormatter owns/replaces the formatter.
    if (AchievementList::openInventory != nullptr)
        AchievementList::openInventory->setStatStringFormatter(nullptr);

    // Screens are GC-managed in Java and can appear in both the active stack
    // and the deferred-delete queue. Destroy each pointer exactly once.
    std::unordered_set<GuiScreen *> screens;
    screens.insert(ownedGuiScreens.begin(), ownedGuiScreens.end());
    screens.insert(guiScreensToDelete.begin(), guiScreensToDelete.end());
    if (currentScreen != nullptr)
        screens.insert(currentScreen);
    for (GuiScreen *screen : screens)
        delete screen;
    ownedGuiScreens.clear();
    guiScreensToDelete.clear();
    currentScreen = nullptr;

    // Detach render-side references before deleting worlds/entities.
    if (renderGlobal != nullptr)
        renderGlobal->changeWorld(nullptr);
    if (effectRenderer != nullptr)
        effectRenderer->clearEffects(nullptr);
    if (RenderManager::instance != nullptr)
        RenderManager::instance->setWorld(nullptr);

    std::unordered_set<World *> worlds;
    worlds.insert(worldsToDelete.begin(), worldsToDelete.end());
    if (theWorld != nullptr)
        worlds.insert(theWorld);
    for (World *world : worlds)
        delete world;
    worldsToDelete.clear();
    theWorld = nullptr;
    thePlayer = nullptr;
    renderViewEntity = nullptr;

    delete renderGlobal;
    renderGlobal = nullptr;
    delete effectRenderer;
    effectRenderer = nullptr;
    delete entityRenderer;
    entityRenderer = nullptr;
    delete ingameGUI;
    ingameGUI = nullptr;

    delete RenderManager::instance;

    delete objectMouseOver;
    objectMouseOver = nullptr;

    // Objects below only borrow each other. Delete consumers before providers.
    delete standardGalacticFontRenderer;
    standardGalacticFontRenderer = nullptr;
    delete fontRenderer;
    fontRenderer = nullptr;
    delete renderEngine;
    renderEngine = nullptr;
    delete texturePackList;
    texturePackList = nullptr;
    delete gameSettings;
    gameSettings = nullptr;

    // Desktop StatFileWriter may own a StatsSyncher worker, so Session must
    // stay alive until after this delete. Console-local stats do not create it.
    delete statFileWriter;
    statFileWriter = nullptr;
    delete saveLoader;
    saveLoader = nullptr;
    delete mouseHelper;
    mouseHelper = nullptr;

    delete playerController;
    playerController = nullptr;
    delete loadingScreen;
    loadingScreen = nullptr;
    delete guiAchievement;
    guiAchievement = nullptr;
    delete field_9242_w;
    field_9242_w = nullptr;

    if (sndManager != nullptr)
    {
        try { sndManager->closeMinecraft(); } catch (...) {}
        delete sndManager;
        sndManager = nullptr;
    }

    delete textureWaterFX;
    textureWaterFX = nullptr;
    delete textureLavaFX;
    textureLavaFX = nullptr;
    delete timer;
    timer = nullptr;
    delete session;
    session = nullptr;

    ModManager::getInstance().shutdown();

    if (theMinecraft == this)
        theMinecraft = nullptr;
}

// ─── Static start ─────────────────────────────────────────────────────────────

void Minecraft::start(const jstring *username, const jstring *sessionId)
{
    Minecraft *mc = new Minecraft(
        ClientPlatformPolicy::initialWidth(),
        ClientPlatformPolicy::initialHeight(),
        false);
    mc->session = new Session(
        *username,
        *sessionId
    );
    mc->run();
    delete mc;
}

Minecraft *Minecraft::getMinecraft()
{
    return theMinecraft;
}

// ─── Crash handler ───────────────────────────────────────────────────────────

void Minecraft::onMinecraftCrash(UnexpectedThrowable *unexpectedthrowable)
{
    hasCrashed = true;
    displayUnexpectedThrowable(unexpectedthrowable);
}

void Minecraft::displayUnexpectedThrowable(UnexpectedThrowable *unexpectedthrowable)
{
    ClientPlatformPolicy::reportCrash(unexpectedthrowable ? unexpectedthrowable->description : "Unknown crash");
}

// ─── Server ──────────────────────────────────────────────────────────────────

void Minecraft::setServer(const std::string &s, int_t i)
{
    serverName = s;
    serverPort = i;
}

// ─── Directory helpers ────────────────────────────────────────────────────────

File *Minecraft::getMinecraftDir()
{
    if (minecraftDir == nullptr)
    {
        minecraftDir = File::open(ClientPlatformPolicy::minecraftDirectory());
        minecraftDir->mkdirs();
    }
    return minecraftDir;
}

File *Minecraft::getAppDir(const std::string & /*s*/)
{
    return getMinecraftDir();
}

// ─── startGame ───────────────────────────────────────────────────────────────

void Minecraft::startGame()
{
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " startGame begin\n");
    if (fullscreen)
    {
        lwjgl::Display::setFullscreen(true);
        displayWidth  = lwjgl::Display::getDisplayMode().getWidth();
        displayHeight = lwjgl::Display::getDisplayMode().getHeight();
        if (displayWidth  <= 0) displayWidth  = 1;
        if (displayHeight <= 0) displayHeight = 1;
    }
    else
    {
        lwjgl::Display::setDisplayMode(lwjgl::DisplayMode(displayWidth, displayHeight));
    }

    lwjgl::Display::setTitle("Minecraft Minecraft 1.2.5");
    lwjgl::Display::create();
    MC_LOG_DEBUG("client.display", "Display created %dx%d\n", displayWidth, displayHeight);

    OpenGlHelper::initializeTextures();

    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " getMinecraftDir begin\n");
    mcDataDir = getMinecraftDir();
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " getMinecraftDir ready: %s\n", mcDataDir ? mcDataDir->toString().c_str() : "(null)");
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " SaveConverter begin\n");
    if (ClientPlatformPolicy::saveConverterUsesSavesSubdirectory())
    {
        std::unique_ptr<File> savesDirectory(File::open(*mcDataDir, "saves"));
        saveLoader = new AnvilSaveConverter(savesDirectory->toString());
    }
    else
    {
#ifdef PS2_PLATFORM
        worldSaveRoot = Ps2SaveStorage::root();
        saveLoader = new AnvilSaveConverter(worldSaveRoot);
#else
        saveLoader = new AnvilSaveConverter(mcDataDir->toString());
#endif
    }
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " SaveConverter ready\n");
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " GameSettings begin\n");
    gameSettings = new GameSettings(this, mcDataDir->toString());
    ClientPlatformPolicy::applyGameSettingsDefaults(gameSettings);
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " GameSettings ready\n");
    MC_LOG_DEBUG("client.config", "renderDistance=%d preload=%d blocks cache=%d unload=%d "
           "gen=%d/tick mesh=%d/%dms loadMin=%dms warmup=%dms\n",
           (int)gameSettings->renderDistance,
           (int)PLATFORM_PRELOAD_RADIUS_BLOCKS,
           (int)PLATFORM_CHUNK_CACHE_RADIUS,
           (int)PLATFORM_CHUNK_UNLOAD_RADIUS,
           (int)PLATFORM_GENERATE_CHUNKS_PER_TICK,
           (int)PLATFORM_MAX_RENDERER_UPDATES_PER_FRAME,
           (int)PLATFORM_CHUNK_BUILD_BUDGET_MS,
           (int)PLATFORM_LOAD_TERRAIN_MIN_MS,
           (int)PLATFORM_LOAD_TERRAIN_WARMUP_MS);
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " TexturePackList begin\n");
    texturePackList = new TexturePackList(this, mcDataDir->toString());
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " TexturePackList ready\n");
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " RenderEngine begin\n");
    renderEngine = new RenderEngine(texturePackList, gameSettings);
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " RenderEngine ready\n");
    platformMemoryCheckpoint("startGame RenderEngine");
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " FontRenderer begin\n");
    fontRenderer = new FontRenderer(gameSettings, "/font/default.png", renderEngine);
    fontRenderer->setUnicodeFlag(StringTranslate::getInstance()->isUnicode());
    fontRenderer->setBidiFlag(StringTranslate::isBidirectional(gameSettings->language));
#if !PLATFORM_PS2
    standardGalacticFontRenderer = new FontRenderer(gameSettings, "/font/alternate.png", renderEngine);
#endif
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " FontRenderer ready\n");

    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " Colorizers begin\n");
    {
        std::vector<int_t> colorMap = renderEngine->readTextureImageData("/misc/watercolor.png");
        MC_LOG_DEBUG("client.resources", "water colormap=%u (expected 65536)\n", (unsigned)colorMap.size());
        ColorizerWater::setWaterBiomeColorizer(colorMap);
    }
    {
        std::vector<int_t> colorMap = renderEngine->readTextureImageData("/misc/grasscolor.png");
        MC_LOG_DEBUG("client.resources", "grass colormap=%u (expected 65536)\n", (unsigned)colorMap.size());
        ColorizerGrass::setGrassBiomeColorizer(colorMap);
    }
    {
        std::vector<int_t> colorMap = renderEngine->readTextureImageData("/misc/foliagecolor.png");
        MC_LOG_DEBUG("client.resources", "foliage colormap=%u (expected 65536)\n", (unsigned)colorMap.size());
        ColorizerFoliage::setFoliageBiomeColorizer(colorMap);
    }
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " Colorizers ready\n");
    platformMemoryCheckpoint("startGame Colorizers");

    ClientPlatformPolicy::preloadStartupTextures(renderEngine);
    platformMemoryCheckpoint("startGame texture preload");

    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " render helpers begin\n");
    new RenderManager();  // constructor sets RenderManager::instance = this
    entityRenderer = new EntityRenderer(this);
    RenderManager::instance->itemRenderer = new ItemRenderer(this);
    statFileWriter = new StatFileWriter(session, mcDataDir->toString());
    AchievementList::openInventory->setStatStringFormatter(new StatStringFormatKeyInv(this));
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " render helpers ready\n");

    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " sound settings begin\n");
    sndManager->loadSoundSettings(gameSettings);
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " sound settings ready\n");

    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " startup presentation begin\n");
    LegacyStartup::run(this);
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " startup presentation done\n");

    mouseHelper = new MouseHelper(nullptr);

    checkGLError("Pre startup");

    renderEnable(RenderCapability::Texture2D);
    renderShadeModel(RenderShadeModel::Smooth);
    renderClearDepth(1.0);
    renderEnable(RenderCapability::DepthTest);
    renderDepthFunc(RenderCompare::LessEqual);
    renderEnable(RenderCapability::AlphaTest);
    renderAlphaFunc(RenderCompare::Greater, 0.1f);
    renderCullFace(RenderFace::Back);
    renderMatrixMode(RenderMatrixMode::Projection);
    renderLoadIdentity();
    renderMatrixMode(RenderMatrixMode::ModelView);

    checkGLError("Startup");

    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " render capabilities ready\n");

    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " texture FX begin\n");
    // OptiFine C6 keeps the standard dynamic texture objects registered even
    // when their animation category is disabled. RenderEngine decides whether
    // generated frames advance or a custom texture remains on its static frame.
    renderEngine->registerTextureFX(textureLavaFX, false);
    renderEngine->registerTextureFX(textureWaterFX, false);
    renderEngine->registerTextureFX(new TexturePortalFX());
    renderEngine->registerTextureFX(new TextureCompassFX(this));
    renderEngine->registerTextureFX(new TextureWatchFX(this));
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " texture FX core ready\n");
    renderEngine->registerTextureFX(new TextureWaterFlowFX());
    renderEngine->registerTextureFX(new TextureLavaFlowFX());
    renderEngine->registerTextureFX(new TextureFlamesFX(0));
    renderEngine->registerTextureFX(new TextureFlamesFX(1));

    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " texture FX ready\n");
    platformMemoryCheckpoint("startGame textureFX");
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " RenderGlobal begin\n");
    renderGlobal = new RenderGlobal(this, renderEngine);
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " RenderGlobal ready\n");
    platformMemoryCheckpoint("startGame RenderGlobal");
    renderViewport(0, 0, displayWidth, displayHeight);
    effectRenderer = new EffectRenderer(theWorld, renderEngine);

    try
    {
        downloadResourcesThread = new ThreadDownloadResources(
            mcDataDir->toString(), this);
        downloadResourcesThread->start();
        MC_LOG_DEBUG("client.resources", "resource thread start requested\n");
    }
    catch (...) {}

    checkGLError("Post startup");

    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " IngameGUI begin\n");
    ingameGUI = new GuiIngame(this);
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " IngameGUI ready\n");

    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " ModManager init begin\n");
    ModManager::getInstance().init(this);
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " ModManager init ready\n");

    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " displayGuiScreen begin\n");
    if (!serverName.empty())
        displayGuiScreen(new GuiConnecting(this, serverName, serverPort));
    else
        displayGuiScreen(new GuiMainMenu());
#ifdef PS2_PLATFORM
    if (serverName.empty() && Ps2SaveStorage::configurationSaveFailed())
        displayGuiScreen(new GuiStorageMessage(currentScreen, gameSettings,
            "Insert a formatted Memory Card in slot 1 to save settings."));
#endif
    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " startGame end currentScreen=%p\n", (void*)currentScreen);
    platformMemoryCheckpoint("startGame end");
}

// ─── loadScreen ──────────────────────────────────────────────────────────────

void Minecraft::loadScreen()
{
    ScaledResolution scaledresolution(gameSettings, displayWidth, displayHeight);
    renderClear(RenderClearMask::Color | RenderClearMask::Depth);
    renderMatrixMode(RenderMatrixMode::Projection);
    renderLoadIdentity();
    renderOrtho(0.0, scaledresolution.field_25121_a, scaledresolution.field_25120_b, 0.0, 1000.0, 3000.0);
    renderMatrixMode(RenderMatrixMode::ModelView);
    renderLoadIdentity();
    renderTranslate(0.0f, 0.0f, -2000.0f);
    renderViewport(0, 0, displayWidth, displayHeight);
    renderClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    Tessellator *tessellator = &Tessellator::instance;
    renderDisable(RenderCapability::Lighting);
    renderEnable(RenderCapability::Texture2D);
    renderDisable(RenderCapability::Fog);
    renderBindTexture(renderEngine->getTexture("/title/mojang.png"));
    tessellator->startDrawingQuads();
    tessellator->setColorOpaque_I(0xffffff);
    tessellator->addVertexWithUV(0.0,          displayHeight, 0.0, 0.0, 0.0);
    tessellator->addVertexWithUV(displayWidth,  displayHeight, 0.0, 0.0, 0.0);
    tessellator->addVertexWithUV(displayWidth,  0.0,           0.0, 0.0, 0.0);
    tessellator->addVertexWithUV(0.0,           0.0,           0.0, 0.0, 0.0);
    tessellator->draw();

    int_t c  = 0x100;
    int_t c1 = 0x100;
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    tessellator->setColorOpaque_I(0xffffff);
    drawSplashScreenTexturedModalRect((scaledresolution.getScaledWidth() - c) / 2,
                (scaledresolution.getScaledHeight() - c1) / 2,
                0, 0, c, c1);

    renderDisable(RenderCapability::Lighting);
    renderDisable(RenderCapability::Fog);
    renderEnable(RenderCapability::AlphaTest);
    renderAlphaFunc(RenderCompare::Greater, 0.1f);
    lwjgl::Display::swapBuffers();
}

void Minecraft::drawSplashScreenTexturedModalRect(int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1)
{
    float f  = 0.00390625f;
    float f1 = 0.00390625f;
    Tessellator *tessellator = &Tessellator::instance;
    tessellator->startDrawingQuads();
    tessellator->addVertexWithUV(i + 0,  j + j1, 0.0, (float)(k + 0)  * f,  (float)(l + j1) * f1);
    tessellator->addVertexWithUV(i + i1, j + j1, 0.0, (float)(k + i1) * f,  (float)(l + j1) * f1);
    tessellator->addVertexWithUV(i + i1, j + 0,  0.0, (float)(k + i1) * f,  (float)(l + 0)  * f1);
    tessellator->addVertexWithUV(i + 0,  j + 0,  0.0, (float)(k + 0)  * f,  (float)(l + 0)  * f1);
    tessellator->draw();
}

// ─── GL error check ──────────────────────────────────────────────────────────

void Minecraft::checkGLError(const std::string &s)
{
    const int err = renderGetError();
    if (err == 0)
        return;

    static unsigned errorCount = 0;
    const unsigned count = ++errorCount;
    if (count <= 4 || (count & (count - 1)) == 0)
        MC_LOG_ERROR("render", "render error at=%s code=0x%x count=%u\n",
                     s.c_str(), (unsigned)err, count);
}

// ─── shutdown / focus ────────────────────────────────────────────────────────

void Minecraft::shutdown()
{
    running = false;
}

void Minecraft::setIngameFocus()
{
    if (mouseHelper == nullptr || !lwjgl::Display::isActive())
        return;
    if (inGameHasFocus)
        return;
    inGameHasFocus = true;
    mouseHelper->grabMouseCursor();
    displayGuiScreen(nullptr);
    leftClickCounter  = 10000;
    mouseTicksRan     = ticksRan + 10000;
}

void Minecraft::setIngameNotInFocus()
{
    if (!inGameHasFocus)
        return;
    if (mouseHelper == nullptr)
    {
        inGameHasFocus = false;
        return;
    }
    KeyBinding::unPressAllKeys();
    if (thePlayer != nullptr)
        thePlayer->resetPlayerKeyState();
    inGameHasFocus = false;
    mouseHelper->ungrabMouseCursor();
}

void Minecraft::shutdownMinecraftApplet()
{
    try
    {
        statFileWriter->prepareStatsForSync();
        statFileWriter->syncStats();

        try
        {
            if (downloadResourcesThread != nullptr)
                downloadResourcesThread->closeMinecraft();
        }
        catch (...) {}

        MC_LOG_INFO("client", "Stopping!\n");

        try { changeWorld1(nullptr); } catch (...) {}
        // No habra mas runTick que vacie la cola diferida: borrar el mundo ahora
        // (en shutdown no hay ningun World::tick() en la pila).
        try
        {
            for (World *w : worldsToDelete)
                delete w;
            worldsToDelete.clear();
            AxisAlignedBB::trimBoundingBoxPool();
            Vec3D::trimVectorPool();
        }
        catch (...) {}
        try { GLAllocation::deleteTexturesAndDisplayLists(); } catch (...) {}

        sndManager->closeMinecraft();
    }
    catch (...)
    {
        lwjgl::Display::swapBuffers();
#if PLATFORM_EXIT_PROCESS_ON_SHUTDOWN
        if (!hasCrashed)
            exit(0);
#else
        return;
#endif
    }
    lwjgl::Display::swapBuffers();
#if PLATFORM_EXIT_PROCESS_ON_SHUTDOWN
    if (!hasCrashed)
        exit(0);
#else
    return;
#endif
}

// ─── Main game loop ──────────────────────────────────────────────────────────

void Minecraft::run()
{
    running = true;

    PLATFORM_BOOT_LOG(PLATFORM_BOOT_PREFIX " run() begin\n");

    try
    {
        startGame();
    }
    catch (const std::exception &e)
    {
        MC_LOG_ERROR("client", "startGame exception: %s\n", e.what());
        onMinecraftCrash(new UnexpectedThrowable(std::string("Failed to start game: ") + e.what()));
        return;
    }
    catch (...)
    {
        MC_LOG_ERROR("client", "startGame exception: unknown\n");
        onMinecraftCrash(new UnexpectedThrowable("Failed to start game"));
        return;
    }

    try
    {
        long_t l = System::currentTimeMillis();
        int_t i  = 0;

        // CPU/GPU frame-time split for the F3 overlay (see cpuUsagePercent /
        // gpuUsagePercent in Minecraft.h). Accumulated every loop iteration,
        // turned into a percentage once a second below.
        long_t cpuGpuFrameNs = 0L;
        long_t cpuGpuSwapNs = 0L;


        while (running)
        {
            try
            {
                const long_t clientFrameStartNs = System::nanoTime();
                long_t clientRenderNs = 0;
                ClientProfiler::frameBegin();
                PlatformStreamingFrameBudget::beginFrame();
                AxisAlignedBB::clearBoundingBoxPool();
                Vec3D::initialize();

                if (lwjgl::Display::isCloseRequested())
                    shutdown();

                if (isGamePaused && theWorld != nullptr)
                {
                    float f = timer->renderPartialTicks;
                    timer->updateTimer();
                    timer->renderPartialTicks = f;
                }
                else
                {
                    timer->updateTimer();
                }

                long_t l1 = System::nanoTime();
                const int_t clientTicksThisFrame = timer->elapsedTicks;
                for (int_t j = 0; j < timer->elapsedTicks; j++)
                {
                    ticksRan++;
                    try
                    {
                        runTick();
                    }
                    catch (const MinecraftException &)
                    {
                        // Java only catches MinecraftException here (the session-lock conflict).
                        // Any other exception must propagate so the real error surfaces instead of
                        // being misreported as a "Level save conflict".
                        World *leakedWorld = theWorld;
                        theWorld = nullptr;
                        changeWorld1(nullptr);
                        // Java relied on GC here; with theWorld nulled, changeWorld1 sees no
                        // oldWorld and skips the save (correct for a lock conflict). Free it ourselves.
                        delete leakedWorld;
                        displayGuiScreen(new GuiConflictWarning());
                    }
                }
                long_t l2 = System::nanoTime() - l1;
                ClientProfiler::ticks(l2, clientTicksThisFrame);
                checkGLError("Pre render");
                RenderBlocks::fancyGrass = gameSettings->fancyGraphics;
                sndManager->setListenerPosition(thePlayer, timer->renderPartialTicks);
                renderEnable(RenderCapability::Texture2D);

                const long_t clientLightingStartNs = System::nanoTime();
                if (theWorld != nullptr)
                    theWorld->updatingLighting();
                ClientProfiler::lighting(System::nanoTime() - clientLightingStartNs);

                const long_t swapStartNs = System::nanoTime();
                if (!lwjgl::Keyboard::isKeyDown(0x41))
                    lwjgl::Display::update();
                ClientProfiler::displayUpdate(System::nanoTime() - swapStartNs);
                const long_t swapEndNs = System::nanoTime();
                cpuGpuSwapNs += swapEndNs - swapStartNs;
                cpuGpuFrameNs += swapEndNs - l1;

                if (thePlayer != nullptr && thePlayer->isEntityInsideOpaqueBlock())
                    gameSettings->thirdPersonView = 0;

                if (!skipRenderWorld)
                {
                    if (playerController != nullptr)
                        playerController->setPartialTime(timer->renderPartialTicks);

                    static int_t heapValidationRenderFrames = 0;
                    const bool validateHeapThisFrame = heapValidationRenderFrames < 16;
                    if (validateHeapThisFrame)
                        validateProcessHeap("before world render");

                    const long_t clientRenderStartNs = System::nanoTime();
                    entityRenderer->updateCameraAndRender(timer->renderPartialTicks);
                    clientRenderNs = System::nanoTime() - clientRenderStartNs;
                    ClientProfiler::render(clientRenderNs);

                    if (validateHeapThisFrame)
                    {
                        validateProcessHeap("after world render");
                        ++heapValidationRenderFrames;
                    }
                }

                ClientProfiler::frameEnd(
                    System::nanoTime() - clientFrameStartNs,
                    l2, clientRenderNs, clientTicksThisFrame,
                    WorldRenderer::chunksUpdated, theWorld, renderGlobal);

                if (!lwjgl::Display::isActive())
                {
                    if (fullscreen)
                        toggleFullscreen();
                    PlatformCompat::delay(10);
                }

#if PLATFORM_SHOW_LAGOMETER
                if (gameSettings->showDebugInfo)
                    displayDebugInfo(l2);
                else
                    prevFrameTime = System::nanoTime();
#else
                // Lagometer compiled out (PLATFORM_SHOW_LAGOMETER). Only the
                // frame-time histogram goes away; the F3 text readout is drawn
                // by GuiIngame and is unaffected. Keep the timestamp fresh so
                // re-enabling never starts from a stale delta.
                (void)l2;
                prevFrameTime = System::nanoTime();
#endif

                guiAchievement->updateAchievementWindow();

                // Last point in the iteration at which anything draws, and the
                // reason this sits here rather than after screenshotListener():
                // the alternate swap below has to stay the next call the backend
                // sees, so that the key-held path still presents the frame this
                // closes rather than an empty one.
                //
                // On the normal path the next Display::update() is a whole tick
                // and a lighting pass away, and closing the frame now lets the
                // backend overlap its present work with them instead of doing it
                // inside the swap. Everything between here and that swap changes
                // tracked state only; nothing submits geometry.
                renderSubmitFrame();

                if (lwjgl::Keyboard::isKeyDown(0x41))
                    lwjgl::Display::update();

                screenshotListener();

                // Resize check (no AWT canvas; use Display size directly)
                {
                    int_t w = lwjgl::Display::getDisplayMode().getWidth();
                    int_t h = lwjgl::Display::getDisplayMode().getHeight();
                    if (!fullscreen && (w != displayWidth || h != displayHeight))
                    {
                        displayWidth  = (w  <= 0) ? 1 : w;
                        displayHeight = (h <= 0) ? 1 : h;
                        resize(displayWidth, displayHeight);
                    }
                }

                checkGLError("Post render");
                i++;
                isGamePaused = !isMultiplayerWorld() &&
                               currentScreen != nullptr &&
                               currentScreen->doesGuiPauseGame();

                while (System::currentTimeMillis() >= l + 1000L)
                {
                    debug = std::to_string(i) + " fps, " +
                            std::to_string(WorldRenderer::chunksUpdated) + " chunk updates";
                    if (cpuGpuFrameNs > 0L)
                    {
                        gpuUsagePercent = (float)(100.0 * (double)cpuGpuSwapNs / (double)cpuGpuFrameNs);
                        cpuUsagePercent = 100.0f - gpuUsagePercent;
                    }
                    cpuGpuFrameNs = 0L;
                    cpuGpuSwapNs = 0L;
                    WorldRenderer::chunksUpdated = 0;
                    l += 1000L;
                    i = 0;

                }
            }
            catch (const MinecraftException &)
            {
                World *leakedWorld = theWorld;
                theWorld = nullptr;
                changeWorld1(nullptr);
                // See note above: free the orphaned world the Java GC would have collected.
                delete leakedWorld;
                displayGuiScreen(new GuiConflictWarning());
            }
            catch (const std::bad_alloc &)
            {
                // Java: catch(OutOfMemoryError)
                platformCaptureBadAlloc();
                freeMemoryForCrash();
#if PLATFORM_RETURN_TO_MENU_ON_OOM
                // The failed world has been released above; continuing it
                // would risk saving incomplete state.  This switch only hides
                // the verbose diagnostic page and returns safely to the menu.
                displayGuiScreen(new GuiMainMenu());
#else
                displayGuiScreen(new GuiErrorScreen());
#endif
            }
        }
    }
    catch (const std::exception &throwable)
    {
        // Java: catch(Throwable) -> print stack trace + onMinecraftCrash.
        // Surface the real error instead of swallowing it.
        freeMemoryForCrash();
        MC_LOG_ERROR("crash", "Unexpected error: %s\n", throwable.what());
        // On PS2/Wii, Minecraft::displayUnexpectedThrowable() only ever shows
        // unexpectedthrowable->description on screen -- MinecraftImpl's
        // fuller PanelCrashReport (which pulls what() back out of the stored
        // exception_ptr) is PC-only, see start(). Fold what() in here so the
        // console crash screen names the actual exception instead of always
        // reading "Unexpected error" with no way to tell one crash from another
        // without a USB Gecko / Dolphin log.
        onMinecraftCrash(new UnexpectedThrowable(std::string("Unexpected error: ") + throwable.what()));
    }
    catch (...)
    {
        freeMemoryForCrash();
        MC_LOG_ERROR("crash", "Unexpected error (non-std exception)\n");
        onMinecraftCrash(new UnexpectedThrowable("Unexpected error"));
    }

    shutdownMinecraftApplet();
}

// ─── freeMemoryForCrash ────────────────────────────────────────────────────────────

void Minecraft::freeMemoryForCrash()
{
    try { std::memset(field_28006_b, 0, sizeof(field_28006_b)); } catch (...) {}
    try
    {
        renderGlobal->clearWorldRenderers();
    }
    catch (...) {}
    try
    {
        AxisAlignedBB::resetBoundingBoxPool();
        Vec3D::resetVectorPool();
    }
    catch (...) {}
    try { changeWorld1(nullptr); } catch (...) {}
}

// ─── screenshotListener ──────────────────────────────────────────────────────

void Minecraft::screenshotListener()
{
    // F2 = 0x3C
    if (lwjgl::Keyboard::isKeyDown(0x3C))
    {
        if (!isTakingScreenshot)
        {
            isTakingScreenshot = true;
            ingameGUI->addChatMessage(
                ScreenShotHelper::saveScreenshot(
                    minecraftDir->toString(), displayWidth, displayHeight));
        }
    }
    else
    {
        isTakingScreenshot = false;
    }
}

// ─── displayDebugInfo ────────────────────────────────────────────────────────

void Minecraft::displayDebugInfo(long_t l)
{
    constexpr long_t l1 = 0xfe502aL;
    const int_t logicalDisplayWidth = ConsoleAspectRatio::getLogicalWidth(
        displayWidth, displayHeight, gameSettings->widescreen);
    const int_t logicalDisplayHeight = ConsoleAspectRatio::getLogicalHeight(displayHeight);

    if (prevFrameTime == -1L)
        prevFrameTime = System::nanoTime();

    long_t l2 = System::nanoTime();
    tickTimes[numRecordedFrameTimes & (512 - 1)] = l;
    frameTimes[numRecordedFrameTimes++ & (512 - 1)] = l2 - prevFrameTime;
    prevFrameTime = l2;

    renderClear(RenderClearMask::Depth);
    renderMatrixMode(RenderMatrixMode::Projection);
    renderLoadIdentity();
    renderOrtho(0.0, logicalDisplayWidth, logicalDisplayHeight, 0.0, 1000.0, 3000.0);
    renderMatrixMode(RenderMatrixMode::ModelView);
    renderLoadIdentity();
    renderTranslate(0.0f, 0.0f, -2000.0f);
    renderLineWidth(1.0f);
    renderDisable(RenderCapability::Texture2D);

    Tessellator *tessellator = &Tessellator::instance;
    int_t i = (int_t)(l1 / 0x30d40L);

    tessellator->startDrawing(renderPrimitiveValue(RenderPrimitive::Quads));
    tessellator->setColorOpaque_I(0x20000000);
    tessellator->addVertex(0.0,               logicalDisplayHeight - i, 0.0);
    tessellator->addVertex(0.0,               logicalDisplayHeight,     0.0);
    tessellator->addVertex(512,               logicalDisplayHeight,     0.0);
    tessellator->addVertex(512,               logicalDisplayHeight - i, 0.0);
    tessellator->setColorOpaque_I(0x20200000);
    tessellator->addVertex(0.0,               logicalDisplayHeight - i * 2, 0.0);
    tessellator->addVertex(0.0,               logicalDisplayHeight - i,     0.0);
    tessellator->addVertex(512,               logicalDisplayHeight - i,     0.0);
    tessellator->addVertex(512,               logicalDisplayHeight - i * 2, 0.0);
    tessellator->draw();

    long_t l3 = 0L;
    for (int_t j = 0; j < 512; j++)
        l3 += frameTimes[j];

    int_t k = (int_t)(l3 / 0x30d40L / 512L);

    tessellator->startDrawing(renderPrimitiveValue(RenderPrimitive::Quads));
    tessellator->setColorOpaque_I(0x20400000);
    tessellator->addVertex(0.0,  logicalDisplayHeight - k, 0.0);
    tessellator->addVertex(0.0,  logicalDisplayHeight,     0.0);
    tessellator->addVertex(512,  logicalDisplayHeight,     0.0);
    tessellator->addVertex(512,  logicalDisplayHeight - k, 0.0);
    tessellator->draw();

    tessellator->startDrawing(renderPrimitiveValue(RenderPrimitive::Lines));
    for (int_t i1 = 0; i1 < 512; i1++)
    {
        int_t j1 = ((i1 - numRecordedFrameTimes & 511) * 255) / 512;
        int_t k1 = (j1 * j1) / 255;
        k1 = (k1 * k1) / 255;
        int_t i2 = (k1 * k1) / 255;
        i2 = (i2 * i2) / 255;
        if (frameTimes[i1] > l1)
            tessellator->setColorOpaque_I(0xff000000 + k1 * 0x10000);
        else
            tessellator->setColorOpaque_I(0xff000000 + k1 * 256);

        long_t l4 = frameTimes[i1] / 0x30d40L;
        long_t l5 = tickTimes[i1]  / 0x30d40L;
        tessellator->addVertex((float)i1 + 0.5f, (float)(logicalDisplayHeight - l4) + 0.5f, 0.0);
        tessellator->addVertex((float)i1 + 0.5f, (float)logicalDisplayHeight        + 0.5f, 0.0);
        tessellator->setColorOpaque_I(0xff000000 + k1 * 0x10000 + k1 * 256 + k1);
        tessellator->addVertex((float)i1 + 0.5f, (float)(logicalDisplayHeight - l4)       + 0.5f, 0.0);
        tessellator->addVertex((float)i1 + 0.5f, (float)(logicalDisplayHeight - (l4 - l5)) + 0.5f, 0.0);
    }
    tessellator->draw();
    renderEnable(RenderCapability::Texture2D);
}

void Minecraft::displayInGameMenu()
{
    if (currentScreen != nullptr)
        return;
    displayGuiScreen(new GuiIngameMenu());
}

// ─── displayGuiScreen ────────────────────────────────────────────────────────

void Minecraft::displayGuiScreen(GuiScreen *guiscreen)
{
    if (dynamic_cast<GuiUnused *>(currentScreen) != nullptr)
        return;
    if (currentScreen != nullptr)
        currentScreen->onGuiClosed();
#if PLATFORM_SYNC_STATS_ON_GUI_CHANGE
    if (dynamic_cast<GuiMainMenu *>(guiscreen) != nullptr)
        statFileWriter->prepareStatsForSync();
    statFileWriter->syncStats();
#endif

    if (guiscreen == nullptr && theWorld == nullptr)
        guiscreen = new GuiMainMenu();
    else if (guiscreen == nullptr && thePlayer != nullptr && thePlayer->health <= 0)
        guiscreen = new GuiGameOver();

    if (dynamic_cast<GuiMainMenu *>(guiscreen) != nullptr)
        ingameGUI->clearChatMessages();

    currentScreen = guiscreen;
    if (guiscreen != nullptr)
    {
        // Track every screen we create. Java relied on GC; in C++ a screen and its
        // buttons leak unless we free them. We can't free on every transition because
        // child screens keep a parentScreen pointer (back-stack), so we defer to
        // purgeOwnedGuiScreens(), called once the whole stack is abandoned.
        // If this exact pointer was scrapped but not yet freed, revive it.
        for (size_t i = 0; i < guiScreensToDelete.size(); i++)
            if (guiScreensToDelete[i] == guiscreen)
            {
                guiScreensToDelete.erase(guiScreensToDelete.begin() + i);
                break;
            }
        bool tracked = false;
        for (GuiScreen *s : ownedGuiScreens)
            if (s == guiscreen) { tracked = true; break; }
        if (!tracked)
            ownedGuiScreens.push_back(guiscreen);

        setIngameNotInFocus();
        ScaledResolution scaledresolution(gameSettings, displayWidth, displayHeight);
        int_t i = scaledresolution.getScaledWidth();
        int_t j = scaledresolution.getScaledHeight();
        guiscreen->setWorldAndResolution(this, i, j);
        skipRenderWorld = false;
    }
    else
    {
        setIngameFocus();
        // Back to gameplay: the menu stack is fully abandoned, so no screen references
        // another anymore -> safe to destroy them all at once.
        purgeOwnedGuiScreens();
    }
}

void Minecraft::purgeOwnedGuiScreens()
{
    // DEFER the actual delete: this is reached from inside a screen's own input
    // handler (a screen closing itself via displayGuiScreen(nullptr)), so deleting
    // synchronously would free a screen whose method is still on the stack
    // (use-after-free, e.g. a GuiTextField touched after the screen dies). The
    // scrap list is flushed at the top of runTick where no screen code is running.
    for (GuiScreen *s : ownedGuiScreens)
        if (s != currentScreen)
            guiScreensToDelete.push_back(s);
    ownedGuiScreens.clear();
    if (currentScreen != nullptr)
        ownedGuiScreens.push_back(currentScreen);
}

// ─── Mouse click helpers ──────────────────────────────────────────────────────

void Minecraft::clickMouse(int_t i, bool flag)
{
    if (!flag)
        leftClickCounter = 0;
    if (i == 0 && leftClickCounter > 0)
        return;

    if (flag && objectMouseOver != nullptr &&
        objectMouseOver->typeOfHit == EnumMovingObjectType::TILE && i == 0)
    {
        int_t j = objectMouseOver->blockX;
        int_t k = objectMouseOver->blockY;
        int_t lv = objectMouseOver->blockZ;
        playerController->sendBlockRemoving(j, k, lv, objectMouseOver->sideHit);
        if (thePlayer->canPlayerEdit(j, k, lv))
        {
            effectRenderer->addBlockHitEffects(j, k, lv, objectMouseOver->sideHit);
            thePlayer->swingItem();
        }
    }
    else
    {
        playerController->resetBlockRemoving();
    }
}

void Minecraft::clickMouse(int_t i)
{
    if (i == 0 && leftClickCounter > 0)
        return;
    if (i == 0)
        thePlayer->swingItem();
    if (i == 1)
        rightClickDelayTimer = 4;

    bool flag = true;
    if (objectMouseOver == nullptr)
    {
        if (i == 0 && playerController->isNotCreative())
            leftClickCounter = 10;
    }
    else if (objectMouseOver->typeOfHit == EnumMovingObjectType::ENTITY)
    {
        if (i == 0)
            playerController->attackEntity(thePlayer, objectMouseOver->entityHit);
        if (i == 1)
            playerController->interactWithEntity(thePlayer, objectMouseOver->entityHit);
    }
    else if (objectMouseOver->typeOfHit == EnumMovingObjectType::TILE)
    {
        int_t j  = objectMouseOver->blockX;
        int_t k  = objectMouseOver->blockY;
        int_t lv = objectMouseOver->blockZ;
        int_t i1 = objectMouseOver->sideHit;
        if (i == 0)
        {
            playerController->clickBlock(j, k, lv, objectMouseOver->sideHit);
        }
        else
        {
            ItemStack *itemstack1 = thePlayer->inventory->getCurrentItem();
            int_t j1 = (itemstack1 == nullptr) ? 0 : itemstack1->stackSize;
            if (playerController->sendPlaceBlock(thePlayer, theWorld, itemstack1, j, k, lv, i1))
            {
                flag = false;
                thePlayer->swingItem();
            }
            if (itemstack1 == nullptr)
                return;
            if (itemstack1->stackSize == 0)
                thePlayer->inventory->mainInventory[thePlayer->inventory->currentItem] = nullptr;
            else if (itemstack1->stackSize != j1 || playerController->isInCreativeMode())
                entityRenderer->getItemRenderer()->resetEquippedProgressAfterBlockPlace();
        }
    }

    if (flag && i == 1)
    {
        ItemStack *itemstack = thePlayer->inventory->getCurrentItem();
        if (itemstack != nullptr && playerController->sendUseItem(thePlayer, theWorld, itemstack))
            entityRenderer->getItemRenderer()->resetEquippedProgressAfterItemUse();
    }
}

void Minecraft::clickMiddleMouseButton()
{
    if (objectMouseOver == nullptr || theWorld == nullptr || thePlayer == nullptr)
        return;

    const bool creative = thePlayer->capabilities.isCreativeMode;
    int_t itemId = theWorld->getBlockId(objectMouseOver->blockX, objectMouseOver->blockY, objectMouseOver->blockZ);
    if (!creative)
    {
        if (itemId == Block::grass->blockID) itemId = Block::dirt->blockID;
        if (itemId == Block::stairDouble->blockID) itemId = Block::stairSingle->blockID;
        if (itemId == Block::bedrock->blockID) itemId = Block::stone->blockID;
    }

    int_t metadata = 0;
    bool matchDamage = false;
    if (itemId > 0 && itemId < Item::ITEM_LIST_SIZE && Item::itemsList[itemId] != nullptr &&
        Item::itemsList[itemId]->getHasSubtypes())
    {
        metadata = theWorld->getBlockMetadata(objectMouseOver->blockX, objectMouseOver->blockY, objectMouseOver->blockZ);
        matchDamage = true;
    }

    if (itemId > 0 && itemId < Item::ITEM_LIST_SIZE && dynamic_cast<ItemBlock *>(Item::itemsList[itemId]) != nullptr)
    {
        Block *block = itemId < Block::BLOCK_REGISTRY_SIZE ? Block::blocksList[itemId] : nullptr;
        if (block != nullptr)
        {
            const int_t droppedId = block->idDropped(metadata, thePlayer->worldObj->rand, 0);
            if (droppedId > 0)
                itemId = droppedId;
        }
    }

    thePlayer->inventory->setCurrentItem(itemId, metadata, matchDamage, creative);
    if (creative && thePlayer->inventorySlots != nullptr)
    {
        const int_t slot = static_cast<int_t>(thePlayer->inventorySlots->slots.size()) - 9 +
                           thePlayer->inventory->currentItem;
        playerController->sendSlotPacket(thePlayer->inventory->getStackInSlot(thePlayer->inventory->currentItem), slot);
    }
}

// ─── toggleFullscreen ────────────────────────────────────────────────────────

void Minecraft::toggleFullscreen()
{
    try
    {
        fullscreen = !fullscreen;
        if (fullscreen)
        {
            lwjgl::Display::setFullscreen(true);
            displayWidth  = lwjgl::Display::getDisplayMode().getWidth();
            displayHeight = lwjgl::Display::getDisplayMode().getHeight();
            if (displayWidth  <= 0) displayWidth  = 1;
            if (displayHeight <= 0) displayHeight = 1;
        }
        else
        {
            displayWidth  = tempDisplayWidth;
            displayHeight = tempDisplayHeight;
            if (displayWidth  <= 0) displayWidth  = 1;
            if (displayHeight <= 0) displayHeight = 1;
            lwjgl::Display::setDisplayMode(lwjgl::DisplayMode(displayWidth, displayHeight));
        }
        if (currentScreen != nullptr)
            resize(displayWidth, displayHeight);

        lwjgl::Display::setFullscreen(fullscreen);
        lwjgl::Display::update();
    }
    catch (...) {}
}

void Minecraft::resize(int_t i, int_t j)
{
    if (i <= 0) i = 1;
    if (j <= 0) j = 1;
    displayWidth  = i;
    displayHeight = j;
    if (currentScreen != nullptr)
    {
        ScaledResolution scaledresolution(gameSettings, i, j);
        int_t k = scaledresolution.getScaledWidth();
        int_t lv = scaledresolution.getScaledHeight();
        currentScreen->setWorldAndResolution(this, k, lv);
    }
}

// ─── startCheckHasPaidThread (check paid) ───────────────────────────────────────────────

void Minecraft::startCheckHasPaidThread()
{
#if PLATFORM_CLIENT_PAID_CHECK
    if (session == nullptr)
        return;

    // Java's Thread object is GC-managed. Do not detach a C++ worker that keeps
    // a raw Minecraft* alive: copy the only data the request needs instead.
    const std::string username = session->username;
    const std::string sessionId = session->sessionId;
    std::thread([username, sessionId]()
    {
        const std::string url = "https://login.minecraft.net/session?name="
            + username + "&session=" + sessionId;
        if (JavaNetwork::getResponseCode(url) == 400)
            Minecraft::hasPaidCheckTime.store(System::currentTimeMillis(), std::memory_order_relaxed);
    }).detach();
#endif
}

// ─── runTick ─────────────────────────────────────────────────────────────────

void Minecraft::runTick()
{
    if (rightClickDelayTimer > 0)
        --rightClickDelayTimer;

    ModManager::getInstance().onTick();

#if PLATFORM_DEFER_PORTAL_TRANSITION
    if (pendingPortalTransition)
    {
        const int_t targetDimension = pendingPortalTargetDimension;
        pendingPortalTransition = false;
        runningPortalTransition = true;
        usePortal(targetDimension);
        runningPortalTransition = false;
    }
#endif

    // Safe point to free GUI screens scrapped by purgeOwnedGuiScreens(): no screen
    // handler is on the call stack here.
    if (!guiScreensToDelete.empty())
    {
        validateProcessHeap("before deferred GUI destruction");
        for (GuiScreen *s : guiScreensToDelete)
        {
            if (s == nullptr)
                continue;
            MC_LOG_DEBUG("heap", "Deleting deferred GUI ptr=%p type=%s\n",
                static_cast<void *>(s), typeid(*s).name());
            delete s;
            if (!validateProcessHeap("after deferred GUI destruction"))
                break;
        }
        guiScreensToDelete.clear();
    }

    // Safe point to free worlds abandoned by changeWorld(): no World::tick() is on the
    // call stack here (the MP dimension-change path defers the delete to right here).
    if (!worldsToDelete.empty())
    {
        validateProcessHeap("before deferred World destruction");
        // The local player is the one object that can cross a World boundary.  Scrub it
        // again immediately before destroying the abandoned worlds: packet handling may
        // have replaced/respawned it after changeWorld() performed its first detach.
        // This also makes the ownership rule explicit for every path that queues a World.
        if (thePlayer != nullptr && theWorld != nullptr)
        {
            for (World *w : worldsToDelete)
                if (w != nullptr && w != theWorld)
                    w->detachEntityForWorldChange(thePlayer);

            thePlayer->setWorld(theWorld);
            renderViewEntity = thePlayer;
        }

        {
            char tag[48];
            std::snprintf(tag, sizeof(tag), "world delete x%u pre",
                          (unsigned)worldsToDelete.size());
            platformMemoryCheckpoint(tag);
        }
        for (World *w : worldsToDelete)
        {
            MC_LOG_DEBUG("heap", "Deleting deferred World ptr=%p\n", static_cast<void *>(w));
            delete w;
            if (!validateProcessHeap("after deferred World destruction"))
                break;
        }
        worldsToDelete.clear();
        platformMemoryCheckpoint("world delete post");

        // When returning to the main menu there is no active world left, so the
        // temporary Java-style math pools can release their overflow heap storage.
        // clearBoundingBoxPool()/Vec3D::initialize() only reset the per-frame index;
        // they intentionally keep overflow entries for reuse while a world is active.
        if (theWorld == nullptr)
        {
            AxisAlignedBB::trimBoundingBoxPool();
            Vec3D::trimVectorPool();
            platformMemoryCheckpoint("back-to-menu post-trim");

            // WorldClient only borrows NetClientHandler. Once its old world is gone,
            // the multiplayer controller can release the handler, join its network
            // threads, and finally destroy the native socket at this safe point.
            if (dynamic_cast<PlayerControllerMP *>(playerController) != nullptr)
            {
                delete playerController;
                playerController = nullptr;
            }
        }
    }

    #if PLATFORM_CLIENT_PAID_CHECK
    if (ticksRan == 6000)
        startCheckHasPaidThread();
    #endif

    long_t clientPhaseStartNs = System::nanoTime();
    statFileWriter->updateStatsSync();
    ClientProfiler::tickPhase("stats", System::nanoTime() - clientPhaseStartNs);

    clientPhaseStartNs = System::nanoTime();
    ingameGUI->updateTick();
    entityRenderer->getMouseOver(1.0f);
    ClientProfiler::tickPhase("mouseOver", System::nanoTime() - clientPhaseStartNs);

    if (thePlayer != nullptr)
    {
        clientPhaseStartNs = System::nanoTime();
        IChunkProvider *ichunkprovider = theWorld->getIChunkProvider();
        int_t jv = MathHelper::floor_float((float)thePlayer->posX) >> 4;
        int_t i1 = MathHelper::floor_float((float)thePlayer->posZ) >> 4;
        configureChunkProviderCache(ichunkprovider, jv, i1, gameSettings->renderDistance);
        ClientProfiler::tickPhase("chunkCacheCfg", System::nanoTime() - clientPhaseStartNs);
    }

    if (!isGamePaused && theWorld != nullptr)
    {
        clientPhaseStartNs = System::nanoTime();
        playerController->updateController();
        ClientProfiler::tickPhase("controller", System::nanoTime() - clientPhaseStartNs);
    }

    // PS2 background asset jobs must progress even when a renderer cached the
    // numeric texture ID after the first request. Without this pump, a transient
    // USB/PAK failure could leave the checkerboard bound forever.
    renderEngine->updateBackgroundTextureLoads();
    renderBindTexture(renderEngine->getTexture("/terrain.png"));

    if (!isGamePaused)
    {
        clientPhaseStartNs = System::nanoTime();
        static int_t s_dynamicTexturePhase = 0;
        const int_t everyN = PLATFORM_DYNAMIC_TEXTURE_INTERVAL_TICKS;
        const bool runDynamicTextures = (everyN <= 1) || ((s_dynamicTexturePhase % everyN) == 0);
        ++s_dynamicTexturePhase;
        if (runDynamicTextures)
            renderEngine->updateDynamicTextures();
        ClientProfiler::tickPhase("dynTex", System::nanoTime() - clientPhaseStartNs);
    }

    if (currentScreen == nullptr && thePlayer != nullptr)
    {
        if (thePlayer->health <= 0)
            displayGuiScreen(nullptr);
        else if (thePlayer->isPlayerSleeping() && theWorld != nullptr && theWorld->multiplayerWorld)
            displayGuiScreen(new GuiSleepMP());
    }
    else if (currentScreen != nullptr &&
             dynamic_cast<GuiSleepMP *>(currentScreen) != nullptr &&
             !thePlayer->isPlayerSleeping())
    {
        displayGuiScreen(nullptr);
    }

    if (currentScreen != nullptr)
    {
        leftClickCounter = 10000;
        mouseTicksRan    = ticksRan + 10000;
    }

    if (currentScreen != nullptr)
    {
        currentScreen->handleInput();
        if (currentScreen != nullptr)
        {
            if (currentScreen->guiParticles != nullptr)
                currentScreen->guiParticles->updateParticles();
            currentScreen->updateScreen();
        }
    }

    if (currentScreen == nullptr || currentScreen->field_948_f)
    {
        clientPhaseStartNs = System::nanoTime();
        while (lwjgl::Mouse::next())
        {
            const int_t eventButton = lwjgl::Mouse::getEventButton();
            const bool eventState = lwjgl::Mouse::getEventButtonState();
            if (eventButton >= 0)
            {
                KeyBinding::setKeyBindState(eventButton - 100, eventState);
                if (eventState)
                    KeyBinding::onTick(eventButton - 100);
            }

            long_t lv = System::currentTimeMillis() - systemTime;
            if (lv <= 200L)
            {
                int_t wheel = lwjgl::Mouse::getEventDWheel();
                if (wheel != 0)
                {
                    thePlayer->inventory->changeCurrentItem(wheel);
                    if (gameSettings->field_22275_C)
                    {
                        if (wheel > 0) wheel = 1;
                        if (wheel < 0) wheel = -1;
                        gameSettings->field_22272_F += (float)wheel * 0.25f;
                    }
                }

                if (currentScreen == nullptr)
                {
                    if (!inGameHasFocus && eventState)
                        setIngameFocus();
                }
                else
                {
                    currentScreen->handleMouseInput();
                }
            }
        }

        if (leftClickCounter > 0)
            --leftClickCounter;

        while (lwjgl::Keyboard::next())
        {
            const int_t eventKey = lwjgl::Keyboard::getEventKey();
            const bool eventState = lwjgl::Keyboard::getEventKeyState();
            KeyBinding::setKeyBindState(eventKey, eventState);
            if (eventState)
                KeyBinding::onTick(eventKey);

            if (!eventState)
                continue;

            if (eventKey == lwjgl::Keyboard::KEY_F11)
            {
                toggleFullscreen();
                continue;
            }

            if (currentScreen != nullptr)
            {
                currentScreen->handleKeyboardInput();
                continue;
            }

            if (eventKey == lwjgl::Keyboard::KEY_ESCAPE)
                displayInGameMenu();
            if (eventKey == lwjgl::Keyboard::KEY_S && lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_F3))
                forceReload();
            if (eventKey == lwjgl::Keyboard::KEY_A && lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_F3))
                renderGlobal->loadRenderers();
            if (eventKey == lwjgl::Keyboard::KEY_T && lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_F3))
                refreshResources();
            if (eventKey == lwjgl::Keyboard::KEY_F && lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_F3))
            {
                bool shift = lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_LSHIFT) ||
                             lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_RSHIFT);
                gameSettings->setOptionValue(EnumOptions::RENDER_DISTANCE, shift ? -1 : 1);
            }
            if (eventKey == lwjgl::Keyboard::KEY_F1)
                gameSettings->hideGUI = !gameSettings->hideGUI;
            if (eventKey == lwjgl::Keyboard::KEY_F3)
            {
                if (gameSettings->legacyUI)
                    displayGuiScreen(new LegacyDebugOptions(nullptr, gameSettings));
                else
                    gameSettings->showDebugInfo = !gameSettings->showDebugInfo;
            }
            if (eventKey == lwjgl::Keyboard::KEY_F5)
            {
                ++gameSettings->thirdPersonView;
                if (gameSettings->thirdPersonView > 2)
                    gameSettings->thirdPersonView = 0;
            }
            if (eventKey == lwjgl::Keyboard::KEY_F8)
                gameSettings->smoothCamera = !gameSettings->smoothCamera;

            for (int_t slot = 0; slot < 9; ++slot)
            {
                if (eventKey == lwjgl::Keyboard::KEY_1 + slot)
                    thePlayer->inventory->currentItem = slot;
            }

            // Keep the OptiFine-style standalone fog binding as an extension.
            // F3+F above follows the vanilla 1.2.5 path.
            if (eventKey == gameSettings->keyBindToggleFog->keyCode &&
                !lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_F3))
            {
                bool shift = lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_LSHIFT) ||
                             lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_RSHIFT);
                gameSettings->setOptionValue(EnumOptions::RENDER_DISTANCE, shift ? -1 : 1);
            }
        }

        while (gameSettings->keyBindInventory->isPressed())
        {
            if (playerController->isInCreativeMode())
                displayGuiScreen(new GuiContainerCreative(thePlayer));
            else
                displayGuiScreen(new GuiInventory(thePlayer));
        }

        while (gameSettings->keyBindDrop->isPressed())
            thePlayer->dropCurrentItem();

        while (isMultiplayerWorld() && gameSettings->keyBindChat->isPressed())
            displayGuiScreen(new GuiChat());

        if (isMultiplayerWorld() && currentScreen == nullptr &&
            (lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_SLASH) ||
             lwjgl::Keyboard::isKeyDown(lwjgl::Keyboard::KEY_DIVIDE)))
        {
            displayGuiScreen(new GuiChat("/"));
        }

        if (thePlayer->isUsingItem())
        {
            if (!gameSettings->keyBindUseItem->pressed)
                playerController->onStoppedUsingItem(thePlayer);

            while (gameSettings->keyBindAttack->isPressed()) {}
            while (gameSettings->keyBindUseItem->isPressed()) {}
            while (gameSettings->keyBindPickBlock->isPressed()) {}
        }
        else
        {
            while (gameSettings->keyBindAttack->isPressed())
                clickMouse(0);
            while (gameSettings->keyBindUseItem->isPressed())
                clickMouse(1);
            while (gameSettings->keyBindPickBlock->isPressed())
                clickMiddleMouseButton();
        }

        if (gameSettings->keyBindUseItem->pressed && rightClickDelayTimer == 0 && !thePlayer->isUsingItem())
            clickMouse(1);

        clickMouse(0, currentScreen == nullptr && gameSettings->keyBindAttack->pressed && inGameHasFocus);
        ClientProfiler::tickPhase("input", System::nanoTime() - clientPhaseStartNs);
    }

    if (theWorld != nullptr)
    {
        if (thePlayer != nullptr)
        {
            joinPlayerCounter++;
            if (joinPlayerCounter == 30)
            {
                joinPlayerCounter = 0;
                clientPhaseStartNs = System::nanoTime();
                theWorld->joinEntityInSurroundings(thePlayer);
                ClientProfiler::tickPhase("joinChunks", System::nanoTime() - clientPhaseStartNs);
            }
        }
        if (theWorld->getWorldInfo() != nullptr && theWorld->getWorldInfo()->isHardcoreModeEnabled())
            theWorld->difficultySetting = 3;
        else
            theWorld->difficultySetting = gameSettings->difficulty;
        if (theWorld->multiplayerWorld)
            theWorld->difficultySetting = 1;

        if (!isGamePaused)
        {
            clientPhaseStartNs = System::nanoTime();
            entityRenderer->updateRenderer();
            ClientProfiler::tickPhase("erTick", System::nanoTime() - clientPhaseStartNs);
        }
        if (!isGamePaused)
            renderGlobal->updateClouds();
        if (!isGamePaused)
        {
            if (theWorld->field_27172_i > 0)
                theWorld->field_27172_i--;
            clientPhaseStartNs = System::nanoTime();
            theWorld->updateEntities();
            ClientProfiler::tickPhase("entities", System::nanoTime() - clientPhaseStartNs);
        }
        if (!isGamePaused || isMultiplayerWorld())
        {
            theWorld->setAllowedMobSpawns(theWorld->difficultySetting > 0, true);
            clientPhaseStartNs = System::nanoTime();
            theWorld->tick();
            ClientProfiler::tickPhase("worldTick", System::nanoTime() - clientPhaseStartNs);
        }
        if (!isGamePaused && theWorld != nullptr)
        {
            clientPhaseStartNs = System::nanoTime();
            theWorld->randomDisplayUpdates(
                MathHelper::floor_double(thePlayer->posX),
                MathHelper::floor_double(thePlayer->posY),
                MathHelper::floor_double(thePlayer->posZ));
            ClientProfiler::tickPhase("randomDisplay", System::nanoTime() - clientPhaseStartNs);
        }
        if (!isGamePaused)
        {
            clientPhaseStartNs = System::nanoTime();
            effectRenderer->updateEffects();
            ClientProfiler::tickPhase("effects", System::nanoTime() - clientPhaseStartNs);
        }
    }

    systemTime = System::currentTimeMillis();
}

// ─── forceReload ─────────────────────────────────────────────────────────────

FontRenderer *Minecraft::getStandardGalacticFontRenderer()
{
    if (standardGalacticFontRenderer == nullptr && gameSettings != nullptr && renderEngine != nullptr)
        standardGalacticFontRenderer = new FontRenderer(gameSettings, "/font/alternate.png", renderEngine);
    return standardGalacticFontRenderer != nullptr ? standardGalacticFontRenderer : fontRenderer;
}

void Minecraft::refreshResources()
{
    if (renderEngine == nullptr)
        return;

    renderEngine->refreshTextures();
    if (fontRenderer != nullptr)
        fontRenderer->refresh(renderEngine);
    if (standardGalacticFontRenderer != nullptr)
        standardGalacticFontRenderer->refresh(renderEngine);
}

void Minecraft::forceReload()
{
    MC_LOG_DEBUG("client", "FORCING RELOAD!\n");

    // The old code replaced sndManager without closing/deleting the previous
    // one. Every forced reload leaked the three SoundPool containers and their
    // SoundPoolEntry allocations.
    if (sndManager != nullptr)
    {
        sndManager->closeMinecraft();
        delete sndManager;
        sndManager = nullptr;
    }

    sndManager = new SoundManager();
    sndManager->loadSoundSettings(gameSettings);

    if (downloadResourcesThread != nullptr)
        downloadResourcesThread->reloadResources();
}

// ─── isMultiplayerWorld ──────────────────────────────────────────────────────

bool Minecraft::isMultiplayerWorld()
{
    return theWorld != nullptr && theWorld->multiplayerWorld;
}

bool Minecraft::isDebugKeepInventoryEnabled()
{
    return gameSettings != nullptr && gameSettings->debugKeepInventory && !isMultiplayerWorld();
}

// ─── World loading ───────────────────────────────────────────────────────────

void Minecraft::startWorld(const std::string &s, const std::string &s1, long_t l)
{
    WorldType::initialize();
    WorldSettings settings(l, 0, true, false, WorldType::DEFAULT);
    startWorld(s, s1, &settings);
}

void Minecraft::startWorld(const std::string &s, const std::string &s1, WorldSettings *settings)
{
    startWorld(getSaveLoader(), s, s1, settings);
}

void Minecraft::startWorld(ISaveFormat *saveFormat, const std::string &s, const std::string &s1, WorldSettings *settings)
{
    startWorld(saveFormat, s, s1, settings, WorldLoadOptions());
}

void Minecraft::startWorld(ISaveFormat *saveFormat, const std::string &s, const std::string &s1, WorldSettings *settings,
                           const WorldLoadOptions &options)
{
    if (saveFormat == nullptr)
        return;
#ifdef PS2_PLATFORM
    if (saveFormat == saveLoader && (!Ps2SaveStorage::available(Ps2SaveStorage::target()) ||
        (Ps2SaveStorage::target() == Ps2SaveStorage::Target::MemoryCard && s == "OPTICRAFT_CFG")))
    {
        displayGuiScreen(new GuiStorageMessage(currentScreen != nullptr ? currentScreen : new GuiMainMenu(), gameSettings,
            "World storage unavailable. Check the selected device in Game Options."));
        return;
    }
#endif

    // On constrained consoles, release resources that only belong to the menu
    // stack before the World constructor starts allocating chunks and storage.
    ClientPlatformPolicy::releaseWorldEntryAssets(renderEngine);

    // Brackets the whole creation path, not just the constructor: everything the
    // loading screen waits on -- the World object, the initial spawn search and
    // the spawn-chunk preload inside changeWorld2 -- runs under this scope, and
    // WorldLoadTrace is inert outside it.
    WORLD_LOAD_SCOPE("startWorld");
    WorldLoadTrace::step("changeWorld1");
    changeWorld1(nullptr);
    if (saveFormat->isOldMapFormat(s))
    {
        WorldLoadTrace::step("convertMapFormat");
        loadingScreen->printText("Converting World to " + saveFormat->getSaveFormatName());
        loadingScreen->displayLoadingString("This may take a while :)");
        if (!saveFormat->convertMapFormat(s, loadingScreen))
            return;
    }

    WorldLoadTrace::step("getSaveLoader");
    ISaveHandler *isavehandler = saveFormat->getSaveLoader(s, false);
    WorldLoadTrace::step("new World");
    platformHardwareCheckpoint("before new World");
    World *world = new World(isavehandler, s1, settings);
    world->setNaturalMobSpawningEnabled(options.naturalMobSpawningEnabled);
    platformHardwareCheckpoint("after new World");
    WorldLoadTrace::step("changeWorld2");
    if (world->isNewWorld)
    {
        statFileWriter->readStat(StatList::createWorldStat, 1);
        statFileWriter->readStat(StatList::startGameStat, 1);
        changeWorld2(world, "Generating level");
    }
    else
    {
        statFileWriter->readStat(StatList::loadWorldStat, 1);
        statFileWriter->readStat(StatList::startGameStat, 1);
        changeWorld2(world, "Loading level");
    }
}

void Minecraft::usePortal(int_t targetDimension)
{
#if PLATFORM_DEFER_PORTAL_TRANSITION
    if (!runningPortalTransition)
    {
        pendingPortalTargetDimension = targetDimension;
        pendingPortalTransition = true;
        return;
    }
#endif

    if (thePlayer == nullptr || theWorld == nullptr)
        return;

    const int_t previousDimension = thePlayer->dimension;
    WorldProvider *destinationProvider = WorldProvider::getProviderForDimension(targetDimension);
    if (destinationProvider == nullptr)
        return;

    MC_LOG_DEBUG("world", "Changing dimension %d -> %d\n",
                 (int)previousDimension, (int)targetDimension);

    thePlayer->dimension = targetDimension;
    theWorld->setEntityDead(thePlayer);
    thePlayer->isDead = false;

    double targetX = thePlayer->posX;
    double targetZ = thePlayer->posZ;
    double coordinateScale = 1.0;
    if (previousDimension > -1 && targetDimension == -1)
        coordinateScale = 0.125;
    else if (previousDimension == -1 && targetDimension > -1)
        coordinateScale = 8.0;

    targetX *= coordinateScale;
    targetZ *= coordinateScale;

    World *oldWorld = theWorld;
    World *destinationWorld = nullptr;
    std::string transitionMessage;

    if (targetDimension == -1)
    {
        thePlayer->setLocationAndAngles(targetX, thePlayer->posY, targetZ,
                                        thePlayer->rotationYaw, thePlayer->rotationPitch);
        if (thePlayer->isEntityAlive())
            oldWorld->updateEntityWithOptionalForce(thePlayer, false);

        destinationWorld = new World(oldWorld, destinationProvider);
        transitionMessage = "Entering the Nether";
    }
    else if (targetDimension == 0)
    {
        if (thePlayer->isEntityAlive())
        {
            thePlayer->setLocationAndAngles(targetX, thePlayer->posY, targetZ,
                                            thePlayer->rotationYaw, thePlayer->rotationPitch);
            oldWorld->updateEntityWithOptionalForce(thePlayer, false);
        }

        destinationWorld = new World(oldWorld, destinationProvider);
        transitionMessage = previousDimension == -1 ? "Leaving the Nether" : "Leaving the End";
    }
    else
    {
        destinationWorld = new World(oldWorld, destinationProvider);
        std::unique_ptr<ChunkCoordinates> entrance(destinationWorld->getEntrancePortalLocation());
        if (entrance != nullptr)
        {
            targetX = (double)entrance->x;
            thePlayer->posY = (double)entrance->y;
            targetZ = (double)entrance->z;
        }

        thePlayer->setLocationAndAngles(targetX, thePlayer->posY, targetZ, 90.0f, 0.0f);
        if (thePlayer->isEntityAlive())
            destinationWorld->updateEntityWithOptionalForce(thePlayer, false);
        transitionMessage = "Entering the End";
    }

#if PLATFORM_RELEASE_OLD_WORLD_BEFORE_PORTAL
    oldWorld->detachEntityForWorldChange(thePlayer);
    oldWorld->saveWorldIndirectly(loadingScreen);
    renderViewEntity = nullptr;
    if (renderGlobal != nullptr)
        renderGlobal->changeWorld(nullptr);
    if (effectRenderer != nullptr)
        effectRenderer->clearEffects(nullptr);
    theWorld = nullptr;
    platformMemoryCheckpoint("portal old world pre-delete");
    delete oldWorld;
    platformMemoryCheckpoint("portal old world deleted");
#endif

    changeWorld(destinationWorld, transitionMessage, thePlayer);
    thePlayer->worldObj = theWorld;

    if (thePlayer->isEntityAlive() && previousDimension < 1)
    {
        thePlayer->setLocationAndAngles(targetX, thePlayer->posY, targetZ,
                                        thePlayer->rotationYaw, thePlayer->rotationPitch);
        theWorld->updateEntityWithOptionalForce(thePlayer, false);
        Teleporter().placeInPortal(theWorld, thePlayer);
    }
}

void Minecraft::changeWorld1(World *world)
{
    changeWorld2(world, "");
}

void Minecraft::changeWorld2(World *world, const std::string &s)
{
    changeWorld(world, s, nullptr);
}

void Minecraft::changeWorld(World *world, const std::string &s, EntityPlayerSP *entityplayer)
{
    World *oldWorld = theWorld;
    EntityPlayerSP *transferredPlayer = entityplayer;
    if (transferredPlayer == nullptr && world != nullptr && world->multiplayerWorld)
        transferredPlayer = thePlayer;
    if (oldWorld != nullptr && transferredPlayer != nullptr)
        oldWorld->detachEntityForWorldChange(transferredPlayer);

    statFileWriter->prepareStatsForSync();
    statFileWriter->syncStats();
    renderViewEntity = nullptr;
    loadingScreen->printText(s);
    loadingScreen->displayLoadingString("");
    const long_t loadScreenStart = System::currentTimeMillis();
    sndManager->playStreaming("", 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

    if (oldWorld != nullptr)
        oldWorld->saveWorldIndirectly(loadingScreen);

    theWorld = world;
    if (renderEngine != nullptr && world == nullptr)
        renderEngine->setBackgroundTextureLoadingEnabled(false);

    // Un jugador TRANSFERIDO (portal SP, o cambio de dimension MP) conserva el mismo
    // objeto EntityPlayer, pero su worldObj seguia apuntando al mundo viejo. Como ese
    // mundo se libera (diferido) al terminar el cambio, dejaba worldObj colgando y el
    // siguiente getMouseOver()->rayTrace()->worldObj->rayTraceBlocks() crasheaba con
    // this=0xFFFF... Reasignarlo al nuevo mundo aqui. (En la carga normal SP el jugador
    // se obtiene del propio mundo via findEntityByClass, asi que ya trae el worldObj correcto.)
    if (transferredPlayer != nullptr && world != nullptr)
        transferredPlayer->setWorld(world);

    if (world != nullptr)
    {
        playerController->onWorldChanged(world);
        if (!isMultiplayerWorld())
        {
            if (entityplayer == nullptr)
                thePlayer = (EntityPlayerSP *)world->findEntityByClass(typeid(EntityPlayerSP));
        }
        else if (thePlayer != nullptr)
        {
            thePlayer->preparePlayerToSpawn();
            if (world != nullptr)
                world->entityJoinedWorld(thePlayer);
        }

        if (!world->multiplayerWorld)
        {
            platformMemoryCheckpoint("changeWorld pre-preload");
            preloadWorld(s);
            platformMemoryCheckpoint("changeWorld post-preload");
        }

        if (thePlayer == nullptr)
        {
            thePlayer = (EntityPlayerSP *)playerController->createPlayer(world);
            platformMemoryCheckpoint("changeWorld player created");
            // Entity::preparePlayerToSpawn walks posY upward until the spawn box
            // stops colliding, with no iteration cap. Marked on both sides
            // because an unbounded search loop is the one call in this block
            // that can fail to return rather than fail loudly.
            thePlayer->preparePlayerToSpawn();
            platformMemoryCheckpoint("changeWorld player spawned");
            playerController->flipPlayer(thePlayer);
        }
        platformMemoryCheckpoint("changeWorld player ready");

        delete thePlayer->movementInput;
        thePlayer->movementInput = new MovementInputFromOptions(gameSettings);

        playerController->initializePlayer(thePlayer);
        platformMemoryCheckpoint("changeWorld controller ready");

        if (entityplayer != nullptr)
            world->emptyMethod1();

        IChunkProvider *ichunkprovider = world->getIChunkProvider();
        int_t ii = MathHelper::floor_float((float)thePlayer->posX) >> 4;
        int_t jj = MathHelper::floor_float((float)thePlayer->posZ) >> 4;
        configureChunkProviderCache(ichunkprovider, ii, jj, gameSettings->renderDistance);

        platformMemoryCheckpoint("changeWorld pre-spawnChunks");
        world->spawnPlayerWithLoadedChunks(thePlayer);
        platformMemoryCheckpoint("changeWorld post-spawnChunks");
        if (world->isNewWorld)
        {
#if PLATFORM_SKIP_NEW_WORLD_FULL_SAVE
            MC_LOG_DEBUG("save", "new world initial full save skipped by platform policy\n");
#else
            world->saveWorldIndirectly(loadingScreen);
#endif
        }

        renderViewEntity = thePlayer;
    }
    else
    {
        thePlayer = nullptr;
    }

    if (renderGlobal != nullptr)
        renderGlobal->changeWorld(world);
    platformMemoryCheckpoint("changeWorld renderGlobal ready");
    if (effectRenderer != nullptr)
        effectRenderer->clearEffects(world);

#if PLATFORM_LOAD_TERRAIN_WARMUP_MS > 0 || PLATFORM_LOAD_TERRAIN_MIN_MS > 0
    // RenderGlobal has just queued every section in the new renderer grid.
    // Build nearby meshes while the loading screen is still visible so the
    // first gameplay frames do not inherit the full renderer backlog. Normal
    // local world entry also keeps a short minimum presentation time, measured
    // from the start of changeWorld(); portal transfers are never delayed just
    // to satisfy that minimum.
    if (world != nullptr && !world->multiplayerWorld && thePlayer != nullptr &&
        renderGlobal != nullptr)
    {
        loadingScreen->displayLoadingString("Preparing terrain");
        const bool enforceMinimumLoadTime = entityplayer == nullptr;
        const long_t warmupStart = System::currentTimeMillis();
        const long_t minimumEnd = loadScreenStart + PLATFORM_LOAD_TERRAIN_MIN_MS;
        const long_t warmupEnd = warmupStart + PLATFORM_LOAD_TERRAIN_WARMUP_MS;
        const int_t warmupQueuedAtStart = renderGlobal->pendingRendererUpdateCount();
        bool meshesReady = warmupQueuedAtStart == 0;
#if PLATFORM_WII && PLATFORM_ASYNC_CHUNK_GENERATION
        ChunkProvider *warmupChunkProvider = dynamic_cast<ChunkProvider *>(world->getIChunkProvider());
#endif

        do
        {
            if (!meshesReady)
            {
                renderGlobal->updateRenderers(thePlayer, false);
                meshesReady = renderGlobal->pendingRendererUpdateCount() == 0;
            }

#if PLATFORM_WII && PLATFORM_ASYNC_CHUNK_GENERATION
            // A Wii renderer can enqueue missing ChunkCache dependencies while
            // updateRenderers() runs. Dispatch/publish them immediately and yield
            // so the lower-priority generation LWP gets CPU before the next retry.
            if (warmupChunkProvider != nullptr && !meshesReady)
                warmupChunkProvider->serviceAsyncChunkStreaming();
            if (!meshesReady)
                PlatformCompat::delay(1);
#endif

            const long_t now = System::currentTimeMillis();
            if (meshesReady && enforceMinimumLoadTime && now < minimumEnd)
                PlatformCompat::delay(1);

            const long_t elapsed = now - warmupStart;
            const int_t progress = PLATFORM_LOAD_TERRAIN_WARMUP_MS > 0
                ? (int_t)((elapsed * 100L) / PLATFORM_LOAD_TERRAIN_WARMUP_MS)
                : 100;
            loadingScreen->setLoadingProgress(progress > 100 ? 100 : progress);
        }
        while ((!meshesReady || (enforceMinimumLoadTime && System::currentTimeMillis() < minimumEnd)) &&
               System::currentTimeMillis() < warmupEnd);

        loadingScreen->setLoadingProgress(100);
        if (meshesReady)
        {
            platformMemoryCheckpoint("changeWorld warmup complete");
        }
        else
        {
            const int_t stillQueued = renderGlobal->pendingRendererUpdateCount();
            const int_t built = warmupQueuedAtStart - stillQueued;
            MC_LOG_INFO("client.load", "warmup capped: built %d/%d sections in %dms\n",
                   (int)built, (int)warmupQueuedAtStart,
                   (int)PLATFORM_LOAD_TERRAIN_WARMUP_MS);
            platformMemoryCheckpoint("changeWorld warmup capped");
        }
    }
#endif

    // El borrado del mundo viejo se DIFIERE al inicio del proximo runTick. Motivo:
    // en multiplayer, changeWorld puede ejecutarse en medio del tick del mundo viejo
    // (WorldClient::tick() -> processReadPackets() -> handleRespawn() -> changeWorld()
    // al cambiar de dimension). Borrarlo aqui de forma sincrona liberaria el World cuyo
    // tick() sigue en la pila -> use-after-free al volver (this->pendingBlockChanges, etc.).
    // Mismo patron que guiScreensToDelete.
    if (oldWorld != nullptr && oldWorld != world)
        worldsToDelete.push_back(oldWorld);
    if (world == nullptr)
    {
        AxisAlignedBB::clearBoundingBoxPool();
        Vec3D::initialize();
    }
    else if (renderEngine != nullptr)
    {
        renderEngine->setBackgroundTextureLoadingEnabled(true);
    }


    systemTime = 0L;
}

// ─── respawn ─────────────────────────────────────────────────────────────────

void Minecraft::respawn(bool flag, int_t i, bool copyPlayerState)
{
    if (!theWorld->multiplayerWorld && !theWorld->worldProvider->canRespawnHere())
        usePortal(0);

    ChunkCoordinates *chunkcoordinates  = nullptr; // borrowed from the old player
    ChunkCoordinates *chunkcoordinates1 = nullptr;
    ChunkCoordinates fallbackSpawn;
    bool ownsBedSpawn = false;
    bool flag1 = true;

    if (thePlayer != nullptr && !flag)
    {
        chunkcoordinates = thePlayer->getPlayerSpawnCoordinate();
        if (chunkcoordinates != nullptr)
        {
            chunkcoordinates1 = EntityPlayer::getNearestBedSpawnLocation(theWorld, chunkcoordinates);
            ownsBedSpawn = chunkcoordinates1 != nullptr;
            if (chunkcoordinates1 == nullptr)
                thePlayer->addChatMessage("tile.bed.notValid");
        }
    }

    if (chunkcoordinates1 == nullptr)
    {
        fallbackSpawn = theWorld->getSpawnPoint();
        chunkcoordinates1 = &fallbackSpawn;
        flag1 = false;
    }

    IChunkProvider *ichunkprovider = theWorld->getIChunkProvider();
    configureChunkProviderCache(ichunkprovider, chunkcoordinates1->x >> 4, chunkcoordinates1->z >> 4, gameSettings->renderDistance);

    theWorld->setSpawnLocation();
    theWorld->updateEntityList();
    int_t j = 0;
    EntityPlayerSP *oldPlayer = thePlayer;
    if (thePlayer != nullptr)
    {
        j = thePlayer->entityId;
        theWorld->setEntityDead(thePlayer);
    }

    renderViewEntity = nullptr;
    thePlayer = (EntityPlayerSP *)playerController->createPlayer(theWorld);
    if (copyPlayerState && oldPlayer != nullptr)
        thePlayer->copyPlayer(oldPlayer);
    else if (isDebugKeepInventoryEnabled() && oldPlayer != nullptr &&
             thePlayer->inventory != nullptr && oldPlayer->inventory != nullptr)
        thePlayer->inventory->copyInventory(oldPlayer->inventory);
    thePlayer->dimension = i;
    renderViewEntity     = thePlayer;
    thePlayer->preparePlayerToSpawn();

    if (flag1)
    {
        thePlayer->setPlayerSpawnCoordinate(chunkcoordinates);
        thePlayer->setLocationAndAngles(
            (float)chunkcoordinates1->x + 0.5f,
            (float)chunkcoordinates1->y + 0.1f,
            (float)chunkcoordinates1->z + 0.5f,
            0.0f, 0.0f);
    }

    if (ownsBedSpawn)
        delete chunkcoordinates1;

    playerController->flipPlayer(thePlayer);
    theWorld->spawnPlayerWithLoadedChunks(thePlayer);
    thePlayer->movementInput = new MovementInputFromOptions(gameSettings);
    thePlayer->entityId      = j;
    thePlayer->handleItemUseFinish();
    playerController->initializePlayer(thePlayer);
    preloadWorld("Respawning");

    if (dynamic_cast<GuiGameOver *>(currentScreen) != nullptr)
        displayGuiScreen(nullptr);
}

// ─── convertMapFormat / preloadWorld ──────────────────────────────────────────

void Minecraft::convertMapFormat(const std::string &s, const std::string &s1)
{
    loadingScreen->printText("Converting World to " + saveLoader->getSaveFormatName());
    loadingScreen->displayLoadingString("This may take a while :)");
    saveLoader->convertMapFormat(s, loadingScreen);
    startWorld(s, s1, static_cast<long_t>(0));
}

void Minecraft::preloadWorld(const std::string &s)
{
    loadingScreen->printText(s);
    loadingScreen->displayLoadingString("Building terrain");
    platformMemoryCheckpoint("preloadWorld begin");

    // Preload radius in blocks. The loop below steps 16 at a time over [-c, c] on
    // both axes, so this is ((2c/16)+1)^2 chunk columns generated SYNCHRONOUSLY
    // on the loading screen, at ~80 KB each:
    //
    //     c = 128  ->  17 x 17 = 289 columns  (~23 MB)   <- the desktop value
    //     c =  64  ->   9 x  9 =  81 columns  (~6.5 MB)
    //     c =  32  ->   5 x  5 =  25 columns  (~2 MB)
    //
    // This used to be platform-gated, so other targets took the
    // hardcoded 128 whether or not it had 23 MB to spare -- the Wii did not, and
    // this was its largest single allocation. It is a memory budget, so it is
    // PLATFORM_BOUNDED_WORLD's to decide.
    //
    // Note the cost does not come back on its own: the unloadAllChunks() at the
    // end of this function cannot free any of it, because eviction needs a chunk
    // to be outside the unload radius AND untouched for
    // PLATFORM_MIN_UNUSED_TICKS_BEFORE_UNLOAD ticks, and these were all touched
    // moments ago. The first pass frees nothing, returns false, and the drain
    // loop exits. Whatever is preloaded here is resident for the session.
#if PLATFORM_BOUNDED_WORLD || PLATFORM_PC_LEGACY
    int_t c  = PLATFORM_PRELOAD_RADIUS_BLOCKS;
#else
    int_t c  = 128;
#endif
    int_t i  = 0;
    int_t j  = (c * 2) / 16 + 1;
    j *= j;

    IChunkProvider   *ichunkprovider = theWorld->getIChunkProvider();
    ChunkCoordinates chunkcoordinates = theWorld->getSpawnPoint();
    if (thePlayer != nullptr)
    {
        chunkcoordinates.x = (int_t)thePlayer->posX;
        chunkcoordinates.z = (int_t)thePlayer->posZ;
    }

    configureChunkProviderCache(ichunkprovider, chunkcoordinates.x >> 4, chunkcoordinates.z >> 4, gameSettings->renderDistance);

#if PLATFORM_PS2 || PLATFORM_WII || PLATFORM_PC_LEGACY
    // Gameplay generation can be deferred on low-end profiles, but this loading
    // screen is intentionally synchronous. Reuse the spawn-generation bypass so
    // console requests cannot return temporary blank chunks and Legacy PC keeps
    // initial decoration out of the gameplay populate queue.
    const bool previousFindingSpawnPoint = theWorld->findingSpawnPoint;
    theWorld->findingSpawnPoint = true;
#endif

    // Touching a column generates (and, on this screen, populates) its chunk.
    auto preloadColumn = [&](int_t x, int_t z)
    {
        theWorld->getBlockId(x, 64, z);
#if PLATFORM_PRELOAD_LIGHTING_STEPS > 0
        for (int_t lightStep = 0;
             lightStep < PLATFORM_PRELOAD_LIGHTING_STEPS && theWorld->updatingLighting();
             ++lightStep) {}
#else
        while (theWorld->updatingLighting()) {}
#endif
    };

    for (int_t k = -c; k <= c; k += 16)
    {
        for (int_t l = -c; l <= c; l += 16)
        {
            loadingScreen->setLoadingProgress((i++ * 100) / j);
            preloadColumn(chunkcoordinates.x + k, chunkcoordinates.z + l);
        }
    }

    // A dimension with a resident region (the End on consoles) gets the whole
    // region generated here, once, instead of streaming it under the dragon.
    // The chunk cache never evicts these (World::isChunkResident).
    const int_t residentRadius = theWorld->worldProvider != nullptr
        ? theWorld->worldProvider->getResidentChunkRadius() : -1;
    if (residentRadius >= 0)
    {
        platformMemoryCheckpoint("preloadWorld resident region begin");
        const int_t residentSide = residentRadius * 2 + 1;
        const int_t residentTotal = residentSide * residentSide;
        int_t residentDone = 0;
        for (int_t chunkX = -residentRadius; chunkX <= residentRadius; ++chunkX)
        {
            for (int_t chunkZ = -residentRadius; chunkZ <= residentRadius; ++chunkZ)
            {
                loadingScreen->setLoadingProgress((residentDone++ * 100) / residentTotal);
                preloadColumn(chunkX * 16 + 8, chunkZ * 16 + 8);
            }
        }
        platformMemoryCheckpoint("preloadWorld resident region end");
    }

#if PLATFORM_PS2 || PLATFORM_WII || PLATFORM_PC_LEGACY
    theWorld->findingSpawnPoint = previousFindingSpawnPoint;
#endif

    loadingScreen->displayLoadingString("Simulating world for a bit");
#if PLATFORM_UNLOAD_AFTER_PRELOAD
    theWorld->unloadAllChunks();
#endif

    // The other end of the "preloadWorld begin" line above. The delta between the
    // two is what the preload radius actually cost, which is the number to look
    // at before touching PLATFORM_PRELOAD_RADIUS_BLOCKS again.
    platformMemoryCheckpoint("preloadWorld end");
}

// ─── installResource ─────────────────────────────────────────────────────────

void Minecraft::installResource(const std::string &s, const std::string &file)
{
    size_t idx = s.find('/');
    if (idx == std::string::npos) return;
    std::string s1 = s.substr(0, idx);
    std::string s2 = s.substr(idx + 1);

    if (s1 == "sound" || s1 == "newsound")
        sndManager->addSound(s2, file);
    else if (s1 == "legacy")
        sndManager->addSound("random/" + s2, file);
    else if (s1 == "streaming")
        sndManager->addStreaming(s2, file);
    else if (s1 == "music" || s1 == "newmusic")
        sndManager->addMusic(s2, file);
}

// ─── Accessors ────────────────────────────────────────────────────────────────

ISaveFormat *Minecraft::getSaveLoader()
{
#ifdef PS2_PLATFORM
    const std::string selectedRoot = Ps2SaveStorage::root();
    if (theWorld == nullptr && worldSaveRoot != selectedRoot)
    {
        if (saveLoader != nullptr) saveLoader->flushCache();
        delete saveLoader;
        saveLoader = new AnvilSaveConverter(selectedRoot);
        worldSaveRoot = selectedRoot;
    }
#endif
    return saveLoader;
}

std::string Minecraft::getDebugLine1() // func_6241_m
{
    return renderGlobal->getDebugInfoRenders();
}

std::string Minecraft::getDebugLine2() // func_6262_n
{
    return renderGlobal->getDebugInfoEntities();
}

std::string Minecraft::getDebugLine3() // func_21002_o
{
    return theWorld->getChunkProviderStats();
}

std::string Minecraft::getDebugLine4() // func_6245_o
{
    return "P: " + effectRenderer->getStatistics() + ". T: " + theWorld->getLoadedEntityStats();
}

// ─── Static helpers ──────────────────────────────────────────────────────────

bool Minecraft::isGuiEnabled()
{
    return theMinecraft == nullptr || !theMinecraft->gameSettings->hideGUI;
}

bool Minecraft::isFancyGraphicsEnabled()
{
    return theMinecraft != nullptr && theMinecraft->gameSettings->fancyGraphics;
}

bool Minecraft::isAmbientOcclusionEnabled()
{
#if PLATFORM_FORCE_FULLBRIGHT_TERRAIN || PLATFORM_ENABLE_GREEDY_MESH
    // Ambient occlusion cannot change a pixel on a profile that forces
    // fullbright terrain, so the honest answer here is "no" regardless of the
    // option.
    //
    // Under PLATFORM_FORCE_FULLBRIGHT_TERRAIN, ChunkCache::getBrightness()
    // returns 1.0f before it looks anything up. The AO path
    // (RenderBlocks::renderStandardBlockWithAmbientOcclusion) then spends twenty
    // to forty aoBrightnessAt() queries and a per-vertex, per-face interpolation
    // to blend a set of values that are all the same constant -- producing
    // exactly the uniform shade the far cheaper
    // renderStandardBlockWithColorMultiplier path produces directly.
    //
    // Greedy faces also require one colour per quad: RenderBlocks AO shades the
    // four corners independently, which cannot be represented by its merge key.
    // Gated here, in the one accessor, rather than at the four RenderBlocks call
    // sites plus TileEntityRendererPiston: they all ask the same question and
    // must not be able to disagree.
    //
    // It also unblocks merging. Greedy meshing keys faces on their shade, so AO's
    // per-corner values split faces that are in fact identically lit; with AO off
    // those faces merge (see PS2_ENABLE_GREEDY_MESH).
    return false;
#else
    return theMinecraft != nullptr && theMinecraft->gameSettings->ambientOcclusion;
#endif
}

bool Minecraft::isDebugInfoEnabled()
{
    return theMinecraft != nullptr && theMinecraft->gameSettings->showDebugInfo;
}

bool Minecraft::lineIsCommand(const std::string &s)
{
    (void)s;
    return false;
}

NetClientHandler *Minecraft::getSendQueue()
{
    EntityClientPlayerMP *mp = (thePlayer->getEntityClassID() == EntityClientPlayerMP::CLASS_ID) ? static_cast<EntityClientPlayerMP*>(thePlayer) : nullptr;
    if (mp != nullptr)
        return mp->sendQueue;
    return nullptr;
}
