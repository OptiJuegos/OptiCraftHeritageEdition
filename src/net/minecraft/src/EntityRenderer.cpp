#include "EntityRenderer.h"
#include "Minecraft.h"
#include "Gui.h"
#include "ItemRenderer.h"
#include "EntityLiving.h"
#include "Potion.h"
#include "PotionEffect.h"
#include "MouseFilter.h"
#include "RenderGlobal.h"
#include "RenderBlocks.h"
#include "EffectRenderer.h"
#include "World.h"
#include "WorldInfo.h"
#include "GameSettings.h"
#include "legacy/LegacyLook.h"
#include "legacy/LegacyColorGradePass.h"
#include "KeyBinding.h"
#include "Config.h"
#include "PlayerController.h"
#include "MovingObjectPosition.h"
#include "Vec3D.h"
#include "PlayerControllerTest.h"
#include "ActiveRenderInfo.h"
#include "AxisAlignedBB.h"
#include "Entity.h"
#include "Material.h"
#include "EntityPlayer.h"
#include "Block.h"
#include "BlockLeaves.h"
#include "EntityPlayerSP.h"
#include "MouseHelper.h"
#include "ScaledResolution.h"
#include "GuiIngame.h"
#include "GuiScreen.h"
#include "GuiParticle.h"
#include "ChunkProviderLoadOrGenerate.h"
#include "ChunkProvider.h"
#include "ClippingHelperImpl.h"
#include "Frustrum.h"
#include "ICamera.h"
#include "RenderEngine.h"
#include "RenderHelper.h"
#include "pc/lwjgl/Display.h"
#include "pc/lwjgl/Keyboard.h"
#include "pc/lwjgl/Mouse.h"
#include "InventoryPlayer.h"
#include "WorldChunkManager.h"
#include "BiomeGenBase.h"
#include "EntitySmokeFX.h"
#include "EntityRainFX.h"
#include "Tessellator.h"
#include "WorldProvider.h"
#include "IChunkProvider.h"
#include "OpenGlHelper.h"
#include "java/BufferedImage.h"
#include "java/Arithmetic.h"
#include "java/Math.h"
#include "MathHelper.h"
#include "platform/PlatformCompat.h"
#include "platform/Profiler.h"
#include "platform/ExtendedProfiler.h"
#include "Profiler.h"
#include "platform/RenderTerrainAPI.h"

// GLU replacement para gluPerspective
#include <algorithm>
#include <cmath>
#include "platform/ConsoleAspectRatio.h"
#include "platform/PlatformTuning.h"
#if PLATFORM_PC_LEGACY
#include "pc/render/PcLegacyLightmapCache.h"
#endif
#if PLATFORM_HAS_VIRTUAL_KEYBOARD
#include "VirtualKeyboard.h"
#endif

#if PLATFORM_DIRECT_ANALOG_MOVEMENT
#include "platform/Input.h"
#endif

#if defined(PS2_PLATFORM)
namespace
{
    int ps2TerrainDaylightBucket(float daylight)
    {
        if (daylight <= 0.0f)
            return 0;
        if (daylight >= 1.0f)
            return 15;
        return static_cast<int>(daylight * 15.0f + 0.5f);
    }
}
#endif

#if PLATFORM_ENABLE_CHUNK_PREFETCH
namespace
{
    int platformPrefetchRadius(int renderDistance)
    {
        renderDistance &= 3;
        int blocks = 64 << (3 - renderDistance);
        if (blocks > 400)
            blocks = 400;

        const int rendererDiameter = blocks / 16 + 1;
        int radius = rendererDiameter / 2 + 1;
        if (radius > PLATFORM_CHUNK_CACHE_RADIUS)
            radius = PLATFORM_CHUNK_CACHE_RADIUS;
        return radius;
    }

    void prefetchNearbyChunks(IChunkProvider* chunkProvider, int centerChunkX,
                                     int centerChunkZ, int renderDistance,
                                     double movementX, double movementZ)
    {
#if PLATFORM_ENABLE_CHUNK_PREFETCH
        if (chunkProvider == nullptr)
            return;

        static int s_prefetchFrame = 0;
        if (++s_prefetchFrame < PLATFORM_PREFETCH_CHUNK_INTERVAL_FRAMES)
            return;
        s_prefetchFrame = 0;

        const int radius = platformPrefetchRadius(renderDistance);
#if PLATFORM_ASYNC_CHUNK_GENERATION || PLATFORM_INCREMENTAL_CHUNK_GENERATION
        ChunkProvider* streamingProvider = dynamic_cast<ChunkProvider*>(chunkProvider);
#endif
        int forwardX = 0;
        int forwardZ = 0;
        const double absX = movementX < 0.0 ? -movementX : movementX;
        const double absZ = movementZ < 0.0 ? -movementZ : movementZ;
        if (absX >= absZ && absX > 0.001)
            forwardX = movementX > 0.0 ? 1 : -1;
        else if (absZ > 0.001)
            forwardZ = movementZ > 0.0 ? 1 : -1;

        int queued = 0;
#if PLATFORM_ASYNC_CHUNK_GENERATION
        int_t skippedX[PLATFORM_ASYNC_GENERATION_QUEUE_LIMIT + 1] = {};
        int_t skippedZ[PLATFORM_ASYNC_GENERATION_QUEUE_LIMIT + 1] = {};
        int skippedCount = 0;
#endif
        while (queued < PLATFORM_PREFETCH_CHUNKS_PER_STEP)
        {
            int bestX = 0;
            int bestZ = 0;
            int bestScore = -0x7fffffff;
            bool found = false;

            for (int dz = -radius; dz <= radius; ++dz)
            {
                for (int dx = -radius; dx <= radius; ++dx)
                {
                    if (dx == 0 && dz == 0)
                        continue;

                    const int cx = centerChunkX + dx;
                    const int cz = centerChunkZ + dz;
                    if (chunkProvider->chunkExists(cx, cz))
                        continue;
#if PLATFORM_INCREMENTAL_CHUNK_GENERATION
                    if (streamingProvider != nullptr && streamingProvider->isChunkGenerationPending(cx, cz))
                        continue;
#endif
#if PLATFORM_ASYNC_CHUNK_GENERATION
                    bool skipped = false;
                    for (int n = 0; n < skippedCount; ++n)
                    {
                        if (skippedX[n] == cx && skippedZ[n] == cz)
                        {
                            skipped = true;
                            break;
                        }
                    }
                    if (skipped)
                        continue;
#endif

                    const int absDx = dx < 0 ? -dx : dx;
                    const int absDz = dz < 0 ? -dz : dz;
                    const int forward = dx * forwardX + dz * forwardZ;
#if PLATFORM_PS2
                    const int chebyshevDistance = std::max(absDx, absDz);
                    const int manhattanDistance = absDx + absDz;
                    const int score = -chebyshevDistance * 64 - manhattanDistance * 4 + forward * 2;
#else
                    const int distance = absDx + absDz;
                    const int score = forward * 16 - distance;
#endif
                    if (!found || score > bestScore)
                    {
                        found = true;
                        bestScore = score;
                        bestX = cx;
                        bestZ = cz;
                    }
                }
            }

            if (!found)
                break;

#if PLATFORM_ASYNC_CHUNK_GENERATION
            if (streamingProvider != nullptr)
            {
                const ChunkProvider::ChunkRequestStatus status = streamingProvider->requestChunkDetailed(bestX, bestZ);
                if (status == ChunkProvider::ChunkRequestStatus::Accepted)
                {
                    ++queued;
                    continue;
                }
                if (status == ChunkProvider::ChunkRequestStatus::QueueFull ||
                    status == ChunkProvider::ChunkRequestStatus::Inactive)
                    break;

                if (skippedCount >= PLATFORM_ASYNC_GENERATION_QUEUE_LIMIT + 1)
                    break;
                skippedX[skippedCount] = bestX;
                skippedZ[skippedCount] = bestZ;
                ++skippedCount;
                continue;
            }
#endif
            chunkProvider->provideChunk(bestX, bestZ);
            ++queued;
        }
        (void)queued;
#else
        (void)chunkProvider;
        (void)centerChunkX;
        (void)centerChunkZ;
        (void)renderDistance;
        (void)movementX;
        (void)movementZ;
#endif
    }
}
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Static members
bool EntityRenderer::anaglyphEnabled = false;
int EntityRenderer::anaglyphField = 0;

static void perspectiveGL(double fovY, double aspect, double zNear, double zFar)
{
#if PLATFORM_FLOAT_VERTEX_MATH
    // Not a cycle argument -- this runs once per frame. It is a code-size one:
    // libm's double tan() is the argument-reduction tables plus its kernel, and
    // this is the only live call to it in the port (util/GLU.cpp has none), so
    // taking the float path can drop the whole object from a link that has a
    // 16 KB instruction cache to fit into. The result feeds a float projection
    // matrix either way.
    const float halfHeight = tanf((float)fovY * ((float)M_PI / 360.0f)) * (float)zNear;
    const float halfWidth = halfHeight * (float)aspect;
    renderFrustum(-halfWidth, halfWidth, -halfHeight, halfHeight, zNear, zFar);
#else
    double fH = tan(fovY * M_PI / 360.0) * zNear;  // half-height at near plane
    double fW = fH * aspect;                          // half-width = height * aspect
    renderFrustum(-fW, fW, -fH, fH, zNear, zFar);
#endif
}

EntityRenderer::EntityRenderer(Minecraft* minecraft)
{
    farPlaneDistance = 0.0f;
    pointedEntity = nullptr;
    
    mouseFilterXAxis = new MouseFilter();
    mouseFilterYAxis = new MouseFilter();
    smoothCamYaw = 0.0f;
    smoothCamPitch = 0.0f;
    smoothCamFilterX = 0.0f;
    smoothCamFilterY = 0.0f;
    smoothCamPartialTicks = 0.0f;
    
    cameraDistance = 4.0f;       // field_22228_r
    prevCameraDistance = 4.0f;   // field_22227_s
    cameraYaw = 0.0;             // Java 1.2.5 double
    prevCameraYaw = 0.0f;        // field_22225_u
    cameraPitch = 0.0;           // Java 1.2.5 double
    prevCameraPitch = 0.0f;      // field_22223_w
    cameraRoll = 0.0f;           // field_22222_x
    prevCameraRoll = 0.0f;       // field_22221_y
    cameraRollTarget = 0.0f;     // field_22220_z
    prevCameraRollTarget = 0.0f; // field_22230_A
    
    cloudFog = false;
    zoomMode = false;
    cameraZoom = 1.0;
    cameraYawOffset = 0.0;
    cameraPitchOffset = 0.0;
    
    prevFrameTime = std::chrono::steady_clock::now();
    field_28133_I = 0;
    
    rainSoundCounter = 0;
    rainCoordsInitialized = false;
    
    
    fogColorRed = 0.0f;
    fogColorGreen = 0.0f;
    fogColorBlue = 0.0f;
    fogColor2 = 0.0f;
    fogColor1 = 0.0f;
    fovModifierHand = 1.0f;
    fovModifierHandPrev = 1.0f;
    lightmapTexture = -1;
    lightmapColors.assign(256, static_cast<int_t>(0xff000000u));
    lightmapUpdateNeeded = true;
#if PLATFORM_PC_LEGACY
#  if PC_LEGACY_LIGHTMAP_CACHE
    pcLegacyLightmapCache = new PcLegacyLightmapCache(PC_LEGACY_LIGHTMAP_INTERVAL_TICKS);
#  else
    pcLegacyLightmapCache = nullptr;
#  endif
#endif
#if defined(PS2_PLATFORM)
    ps2TerrainLightBucket = -1;
    ps2TerrainLightningActive = false;
#endif
    torchFlickerX = 0.0f;
    torchFlickerDX = 0.0f;
    torchFlickerY = 0.0f;
    torchFlickerDY = 0.0f;
    
    rendererUpdateCount = 0;
    debugViewDirection = 0;
    
    mc = minecraft;
    itemRenderer = new ItemRenderer(minecraft);
#if !defined(PS2_PLATFORM)
    BufferedImage lightmapImage(16, 16);
    lightmapTexture = minecraft->renderEngine->allocateAndSetupTexture(&lightmapImage, true);
#endif
    updatedWorldProvider = nullptr;
}

// OptiFine: regenera worldProvider->lightBrightnessTable usando ofBrightness.
// brightness=0 reproduce vanilla (k=3, minLevel=0.05); brightness=1 aclara las
// zonas oscuras (k=0 -> rampa lineal hasta 1.0).
void EntityRenderer::updateWorldLightLevels()
{
    if (mc == nullptr || mc->theWorld == nullptr || mc->theWorld->worldProvider == nullptr)
        return;

    Config::setLightLevels(mc->theWorld->worldProvider->lightBrightnessTable);
    lightmapUpdateNeeded = true;
#if PLATFORM_PC_LEGACY && PC_LEGACY_LIGHTMAP_CACHE
    if (pcLegacyLightmapCache != nullptr)
        pcLegacyLightmapCache->invalidate();
#endif
#if defined(PS2_PLATFORM)
    ps2TerrainLightBucket = ps2TerrainDaylightBucket(mc->theWorld->func_35464_b(1.0f));
    ps2TerrainLightningActive = mc->theWorld->field_27172_i > 0;
    if (mc->renderGlobal != nullptr)
        mc->renderGlobal->markAllRenderersDirty();
#endif
}

EntityRenderer::~EntityRenderer()
{
#if PLATFORM_PC_LEGACY
    delete pcLegacyLightmapCache;
    pcLegacyLightmapCache = nullptr;
#endif
    delete mouseFilterXAxis;
    delete mouseFilterYAxis;
    delete itemRenderer;
    if (lightmapTexture >= 0 && mc != nullptr && mc->renderEngine != nullptr)
        mc->renderEngine->deleteTexture(lightmapTexture);
}

void EntityRenderer::updateTorchFlicker()
{
#if PLATFORM_FLOAT_VERTEX_MATH
    const float dxFirst = static_cast<float>(Math::random());
    const float dxSecond = static_cast<float>(Math::random());
    const float dxScaleA = static_cast<float>(Math::random());
    const float dxScaleB = static_cast<float>(Math::random());
    const float dyFirst = static_cast<float>(Math::random());
    const float dySecond = static_cast<float>(Math::random());
    const float dyScaleA = static_cast<float>(Math::random());
    const float dyScaleB = static_cast<float>(Math::random());
    torchFlickerDX += (dxFirst - dxSecond) * dxScaleA * dxScaleB;
    torchFlickerDY += (dyFirst - dySecond) * dyScaleA * dyScaleB;
    torchFlickerDX *= 0.9f;
    torchFlickerDY *= 0.9f;
#else
    const double dxFirst = Math::random();
    const double dxSecond = Math::random();
    const double dxScaleA = Math::random();
    const double dxScaleB = Math::random();
    const double dyFirst = Math::random();
    const double dySecond = Math::random();
    const double dyScaleA = Math::random();
    const double dyScaleB = Math::random();
    torchFlickerDX = static_cast<float>(static_cast<double>(torchFlickerDX) +
        (dxFirst - dxSecond) * dxScaleA * dxScaleB);
    torchFlickerDY = static_cast<float>(static_cast<double>(torchFlickerDY) +
        (dyFirst - dySecond) * dyScaleA * dyScaleB);
    torchFlickerDX = static_cast<float>(static_cast<double>(torchFlickerDX) * 0.9);
    torchFlickerDY = static_cast<float>(static_cast<double>(torchFlickerDY) * 0.9);
#endif
    torchFlickerX += (torchFlickerDX - torchFlickerX);
    torchFlickerY += (torchFlickerDY - torchFlickerY);
    lightmapUpdateNeeded = true;
}

void EntityRenderer::updateLightmap()
{
    World *world = mc != nullptr ? mc->theWorld : nullptr;
    if (world == nullptr || world->worldProvider == nullptr)
        return;

    const float daylight = world->func_35464_b(1.0f);
    for (int_t i = 0; i < 256; ++i)
    {
        float sky = world->worldProvider->lightBrightnessTable[i / 16] * (daylight * 0.95f + 0.05f);
        float block = world->worldProvider->lightBrightnessTable[i % 16] * (torchFlickerX * 0.1f + 1.5f);
        if (world->field_27172_i > 0)
            sky = world->worldProvider->lightBrightnessTable[i / 16];

        float redSky = sky * (daylight * 0.65f + 0.35f);
        float greenSky = redSky;
        float greenBlock = block * ((block * 0.6f + 0.4f) * 0.6f + 0.4f);
        float blueBlock = block * (block * block * 0.6f + 0.4f);
        float red = redSky + block;
        float green = greenSky + greenBlock;
        float blue = sky + blueBlock;
        red = red * 0.96f + 0.03f;
        green = green * 0.96f + 0.03f;
        blue = blue * 0.96f + 0.03f;

        if (world->worldProvider->worldType == 1)
        {
            red = 0.22f + block * 0.75f;
            green = 0.28f + greenBlock * 0.75f;
            blue = 0.25f + blueBlock * 0.75f;
        }

        const float gamma = mc->gameSettings->ofBrightness;
        red = std::min(1.0f, red);
        green = std::min(1.0f, green);
        blue = std::min(1.0f, blue);

        float gammaInvR = 1.0f - red;
        float gammaInvG = 1.0f - green;
        float gammaInvB = 1.0f - blue;
        const float gammaR = 1.0f - gammaInvR * gammaInvR * gammaInvR * gammaInvR;
        const float gammaG = 1.0f - gammaInvG * gammaInvG * gammaInvG * gammaInvG;
        const float gammaB = 1.0f - gammaInvB * gammaInvB * gammaInvB * gammaInvB;
        red = red * (1.0f - gamma) + gammaR * gamma;
        green = green * (1.0f - gamma) + gammaG * gamma;
        blue = blue * (1.0f - gamma) + gammaB * gamma;

        red = red * 0.96f + 0.03f;
        green = green * 0.96f + 0.03f;
        blue = blue * 0.96f + 0.03f;
        red = std::min(1.0f, red);
        green = std::min(1.0f, green);
        blue = std::min(1.0f, blue);

#if PLATFORM_PS2
        if (mc->gameSettings != nullptr && mc->gameSettings->legacyLook)
            legacyLookRgb(red, green, blue);
#endif

        int_t r = static_cast<int_t>(red * 255.0f);
        int_t g = static_cast<int_t>(green * 255.0f);
        int_t b = static_cast<int_t>(blue * 255.0f);
        lightmapColors[i] = static_cast<int_t>(0xff000000u | (static_cast<uint_t>(r) << 16) |
                                               (static_cast<uint_t>(g) << 8) | static_cast<uint_t>(b));
    }

#if defined(PS2_PLATFORM)
    renderSetLightmapColors(reinterpret_cast<const std::uint32_t*>(lightmapColors.data()),
                            static_cast<int>(lightmapColors.size()));

    const int terrainLightBucket = ps2TerrainDaylightBucket(daylight);
    const bool lightningActive = world->field_27172_i > 0;
    const bool initialized = ps2TerrainLightBucket >= 0;
    if (initialized &&
        (terrainLightBucket != ps2TerrainLightBucket || lightningActive != ps2TerrainLightningActive) &&
        mc->renderGlobal != nullptr)
    {
        mc->renderGlobal->markAllRenderersDirty();
    }
    ps2TerrainLightBucket = terrainLightBucket;
    ps2TerrainLightningActive = lightningActive;
#else
    if (lightmapTexture >= 0)
        mc->renderEngine->updateTextureSubImage(lightmapColors, 16, 16, lightmapTexture);
#endif
#if PLATFORM_PC_LEGACY && PC_LEGACY_LIGHTMAP_CACHE
    lightmapUpdateNeeded = false;
    if (pcLegacyLightmapCache != nullptr)
    {
        pcLegacyLightmapCache->markUpdated(
            rendererUpdateCount, world->worldProvider, mc->gameSettings->ofBrightness,
            world->field_27172_i > 0, mc->gameSettings->legacyLook);
    }
#endif
}

void EntityRenderer::disableLightmap(double)
{
    OpenGlHelper::setActiveTexture(OpenGlHelper::lightmapTexUnit);
    renderDisable(RenderCapability::Texture2D);
    OpenGlHelper::setActiveTexture(OpenGlHelper::defaultTexUnit);
}

void EntityRenderer::enableLightmap(double)
{
#if defined(PS2_PLATFORM)
    OpenGlHelper::setActiveTexture(OpenGlHelper::lightmapTexUnit);
    renderEnable(RenderCapability::Texture2D);
    OpenGlHelper::setActiveTexture(OpenGlHelper::defaultTexUnit);
#else
    if (lightmapTexture < 0)
        return;
    OpenGlHelper::setActiveTexture(OpenGlHelper::lightmapTexUnit);
    renderMatrixMode(RenderMatrixMode::Texture);
    renderLoadIdentity();
    renderScale(0.00390625f, 0.00390625f, 0.00390625f);
    renderTranslate(8.0f, 8.0f, 8.0f);
    renderMatrixMode(RenderMatrixMode::ModelView);
    mc->renderEngine->bindTexture(lightmapTexture);
    renderTextureParameters(true, false, true);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    renderEnable(RenderCapability::Texture2D);
    OpenGlHelper::setActiveTexture(OpenGlHelper::defaultTexUnit);
#endif
}

void EntityRenderer::updateRenderer()
{
    updateTorchFlicker();
    float targetFovMultiplier = 1.0f;
    EntityPlayerSP *player = dynamic_cast<EntityPlayerSP *>(mc->renderViewEntity != nullptr ? mc->renderViewEntity : mc->thePlayer);
    if (player != nullptr)
        targetFovMultiplier = player->getFOVMultiplier();
    fovModifierHandPrev = fovModifierHand;
    fovModifierHand += (targetFovMultiplier - fovModifierHand) * 0.5f;

    // Interpolacion de niebla
    fogColor2 = fogColor1;
    prevCameraDistance = cameraDistance;
    prevCameraYaw = cameraYaw;
    prevCameraPitch = cameraPitch;
    prevCameraRoll = cameraRoll;
    prevCameraRollTarget = cameraRollTarget;

    if (mc->gameSettings->smoothCamera)
    {
        float sensitivity = mc->gameSettings->mouseSensitivity * 0.6f + 0.2f;
        float sensitivityCubed = sensitivity * sensitivity * sensitivity * 8.0f;
        smoothCamFilterX = mouseFilterXAxis->smooth(smoothCamYaw, 0.05f * sensitivityCubed);
        smoothCamFilterY = mouseFilterYAxis->smooth(smoothCamPitch, 0.05f * sensitivityCubed);
        smoothCamPartialTicks = 0.0f;
        smoothCamYaw = 0.0f;
        smoothCamPitch = 0.0f;
    }
    
    if (mc->renderViewEntity == nullptr)
    {
        mc->renderViewEntity = mc->thePlayer;
    }
    
    if (mc->theWorld == nullptr || mc->renderViewEntity == nullptr)
    {
        rendererUpdateCount++;
        itemRenderer->updateEquippedItem();
        return;
    }

    // Calcular brillo de luz en la posicion de la entidad vista
    float lightBrightness = mc->theWorld->getLightBrightness(
        MathHelper::floor_double(mc->renderViewEntity->posX),
        MathHelper::floor_double(mc->renderViewEntity->posY),
        MathHelper::floor_double(mc->renderViewEntity->posZ)
    );
    
    float renderDistFactor = (float)(3 - mc->gameSettings->renderDistance) / 3.0f;
    float targetFog = lightBrightness * (1.0f - renderDistFactor) + renderDistFactor;
    
    fogColor1 += (targetFog - fogColor1) * 0.1f;
    rendererUpdateCount++;
    
    itemRenderer->updateEquippedItem();
    addRainParticles();
}

void EntityRenderer::getMouseOver(float partialTicks)
{
    if (mc->renderViewEntity == nullptr || mc->theWorld == nullptr)
        return;

    // A dimension respawn replaces the C++ World object explicitly, unlike Java's GC.
    // Refresh this non-owning link before any ray trace follows the camera entity.
    mc->renderViewEntity->setWorld(mc->theWorld);

    const bool testController = dynamic_cast<PlayerControllerTest *>(mc->playerController) != nullptr;
    double blockReach = testController ? 32.0 : mc->playerController->getBlockReachDistance();
    delete mc->objectMouseOver;
    mc->objectMouseOver = mc->renderViewEntity->rayTrace(blockReach, partialTicks);

    double closestBlockDistance = blockReach;
    Vec3D *viewPos = mc->renderViewEntity->getPosition(partialTicks);

    double entityReach = blockReach;
    if (!testController && mc->playerController->extendedReach())
    {
        entityReach = 6.0;
        closestBlockDistance = entityReach;
    }
    else if (!testController)
    {
        if (entityReach > 3.0)
            closestBlockDistance = 3.0;
        entityReach = closestBlockDistance;
    }

    if (mc->objectMouseOver != nullptr)
        closestBlockDistance = mc->objectMouseOver->hitVec->distanceTo(viewPos);

    Vec3D *lookVec = mc->renderViewEntity->getLook(partialTicks);
    Vec3D *lookEnd = viewPos->addVector(lookVec->xCoord * entityReach,
                                       lookVec->yCoord * entityReach,
                                       lookVec->zCoord * entityReach);

    pointedEntity = nullptr;
    const float expandSize = 1.0f;
    const auto &entities = mc->theWorld->getEntitiesWithinAABBExcludingEntity(
        mc->renderViewEntity,
        mc->renderViewEntity->boundingBox
            ->addCoord(lookVec->xCoord * entityReach,
                       lookVec->yCoord * entityReach,
                       lookVec->zCoord * entityReach)
            ->expand(expandSize, expandSize, expandSize));

    double closestEntityDistance = closestBlockDistance;
    for (Entity *entity : entities)
    {
        if (!entity->canBeCollidedWith())
            continue;

        const float collisionBorder = entity->getCollisionBorderSize();
        AxisAlignedBB *expandedBB = entity->boundingBox->expand(collisionBorder, collisionBorder, collisionBorder);
        MovingObjectPosition *hit = expandedBB->calculateIntercept(viewPos, lookEnd);

        if (expandedBB->isVecInside(viewPos))
        {
            if (closestEntityDistance > 0.0 || closestEntityDistance == 0.0)
            {
                pointedEntity = entity;
                closestEntityDistance = 0.0;
            }
        }
        else if (hit != nullptr)
        {
            const double hitDistance = viewPos->distanceTo(hit->hitVec);
            if (hitDistance < closestEntityDistance || closestEntityDistance == 0.0)
            {
                pointedEntity = entity;
                closestEntityDistance = hitDistance;
            }
        }
        delete hit;
    }

    if (pointedEntity != nullptr &&
        (closestEntityDistance < closestBlockDistance || mc->objectMouseOver == nullptr))
    {
        delete mc->objectMouseOver;
        mc->objectMouseOver = new MovingObjectPosition(pointedEntity);
    }
}

float EntityRenderer::getFOVModifier(float partialTicks, bool applyFovModifiers)
{
    if (debugViewDirection > 0)
        return 90.0f;

    EntityLiving *entityliving = mc->renderViewEntity;
    float fov = 70.0f;
    if (applyFovModifiers)
    {
        fov += mc->gameSettings->fovSetting * 40.0f;
        fov *= fovModifierHandPrev + (fovModifierHand - fovModifierHandPrev) * partialTicks;
    }

    // OptiFine 1.2.5 HD C6 (lr.java): hold the configured zoom binding,
    // enable smooth camera while zoomed, and restore fresh filters on release.
    bool zoomActive = false;
    // [FIX CRÍTICO WII] Verificación defensiva contra nullptr en ofKeyBindZoom para evitar DSI Exception / Segfault si no está instanciado.
    if (mc->gameSettings->ofKeyBindZoom != nullptr)
    {
        if (mc->gameSettings->ofKeyBindZoom->keyCode < 0)
            zoomActive = lwjgl::Mouse::isButtonDown(mc->gameSettings->ofKeyBindZoom->keyCode + 100);
        else
            zoomActive = lwjgl::Keyboard::isKeyDown(mc->gameSettings->ofKeyBindZoom->keyCode);
    }

    if (zoomActive)
    {
        if (!zoomMode)
        {
            zoomMode = true;
            mc->gameSettings->smoothCamera = true;
        }
        if (zoomMode)
            fov /= 4.0f;
    }
    else if (zoomMode)
    {
        zoomMode = false;
        mc->gameSettings->smoothCamera = false;

        delete mouseFilterXAxis;
        delete mouseFilterYAxis;
        mouseFilterXAxis = new MouseFilter();
        mouseFilterYAxis = new MouseFilter();
    }

    if (entityliving->health <= 0)
    {
        const float deathTime = (float)entityliving->deathTime + partialTicks;
        fov /= (1.0f - 500.0f / (deathTime + 500.0f)) * 2.0f + 1.0f;
    }

    const int_t viewBlockId = ActiveRenderInfo::getBlockIdAtEntityViewpoint(mc->theWorld, entityliving, partialTicks);
    if (viewBlockId > 0 && viewBlockId < Block::BLOCK_REGISTRY_SIZE && Block::blocksList[viewBlockId] != nullptr &&
        Block::blocksList[viewBlockId]->blockMaterial == Material::water)
    {
        fov *= 60.0f / 70.0f;
    }

    return fov + prevCameraRoll + (cameraRoll - prevCameraRoll) * partialTicks;
}

void EntityRenderer::hurtCameraEffect(float partialTicks)
{
    EntityLiving* entityliving = mc->renderViewEntity;
    float hurtTime = (float)entityliving->hurtTime - partialTicks;
    
    if (entityliving->health <= 0)
    {
        float deathTime = (float)entityliving->deathTime + partialTicks;
        renderRotate(40.0f - 8000.0f / (deathTime + 200.0f), 0.0f, 0.0f, 1.0f);
    }
    
    if (hurtTime < 0.0f)
    {
        return;
    }
    
    hurtTime /= entityliving->maxHurtTime;
    hurtTime = MathHelper::sin(hurtTime * hurtTime * hurtTime * hurtTime * 3.1415927f);
    
    float attackedYaw = entityliving->attackedAtYaw;
    
    renderRotate(-attackedYaw, 0.0f, 1.0f, 0.0f);
    renderRotate(-hurtTime * 14.0f, 0.0f, 0.0f, 1.0f);
    renderRotate(attackedYaw, 0.0f, 1.0f, 0.0f);
}

void EntityRenderer::setupViewBobbing(float partialTicks)
{
    EntityPlayer* entityplayer = mc->renderViewEntity->isPlayer() ? static_cast<EntityPlayer*>(mc->renderViewEntity) : nullptr;
    
    if (entityplayer == nullptr)
    {
        return;
    }
    
    float walkDelta = entityplayer->distanceWalkedModified - entityplayer->prevDistanceWalkedModified;
    float walkProgress = -(entityplayer->distanceWalkedModified + walkDelta * partialTicks);
    float bobAmount = entityplayer->field_775_e + (entityplayer->field_774_f - entityplayer->field_775_e) * partialTicks;
    float cameraPitchBob = entityplayer->cameraPitch + (entityplayer->field_9328_R - entityplayer->cameraPitch) * partialTicks;
    
    renderTranslate(MathHelper::sin(walkProgress * 3.1415927f) * bobAmount * 0.5f,
                 -std::abs(MathHelper::cos(walkProgress * 3.1415927f) * bobAmount),
                 0.0f);
    
    renderRotate(MathHelper::sin(walkProgress * 3.1415927f) * bobAmount * 3.0f,
              0.0f, 0.0f, 1.0f);
    
    renderRotate(std::abs(MathHelper::cos(walkProgress * 3.1415927f - 0.2f) * bobAmount) * 5.0f,
              1.0f, 0.0f, 0.0f);
    
    renderRotate(cameraPitchBob, 1.0f, 0.0f, 0.0f);
}

void EntityRenderer::orientCamera(float partialTicks)
{
    EntityLiving* entityliving = mc->renderViewEntity;
    float eyeHeight = entityliving->yOffset - 1.62f;
    
#if PLATFORM_FLOAT_VERTEX_MATH
    float posX = static_cast<float>(entityliving->prevPosX) +
        (static_cast<float>(entityliving->posX) - static_cast<float>(entityliving->prevPosX)) * partialTicks;
    float posY = static_cast<float>(entityliving->prevPosY) +
        (static_cast<float>(entityliving->posY) - static_cast<float>(entityliving->prevPosY)) * partialTicks - eyeHeight;
    float posZ = static_cast<float>(entityliving->prevPosZ) +
        (static_cast<float>(entityliving->posZ) - static_cast<float>(entityliving->prevPosZ)) * partialTicks;
#else
    double posX = entityliving->prevPosX + (entityliving->posX - entityliving->prevPosX) * (double)partialTicks;
    double posY = (entityliving->prevPosY + (entityliving->posY - entityliving->prevPosY) * (double)partialTicks) - (double)eyeHeight;
    double posZ = entityliving->prevPosZ + (entityliving->posZ - entityliving->prevPosZ) * (double)partialTicks;
#endif
    
    renderRotate(prevCameraRollTarget + (cameraRollTarget - prevCameraRollTarget) * partialTicks,
              0.0f, 0.0f, 1.0f);
    
    if (entityliving->isPlayerSleeping())
    {
        eyeHeight += 1.0f;
        renderTranslate(0.0f, 0.3f, 0.0f);
        
        if (!mc->gameSettings->field_22273_E)
        {
            int bedX = MathHelper::floor_double(entityliving->posX);
            int bedY = MathHelper::floor_double(entityliving->posY);
            int bedZ = MathHelper::floor_double(entityliving->posZ);
            
            int blockId = mc->theWorld->getBlockId(bedX, bedY, bedZ);
            
            if (blockId == Block::blockBed->blockID)
            {
                int meta = mc->theWorld->getBlockMetadata(bedX, bedY, bedZ);
                int direction = meta & 3;
                renderRotate((float)(direction * 90), 0.0f, 1.0f, 0.0f);
            }
            
            renderRotate(entityliving->prevRotationYaw + 
                      (entityliving->rotationYaw - entityliving->prevRotationYaw) * partialTicks + 180.0f,
                      0.0f, -1.0f, 0.0f);
            
            renderRotate(entityliving->prevRotationPitch + 
                      (entityliving->rotationPitch - entityliving->prevRotationPitch) * partialTicks,
                      -1.0f, 0.0f, 0.0f);
        }
    }
    else if (mc->gameSettings->thirdPersonView)
    {
#if PLATFORM_FLOAT_VERTEX_MATH
        float camDist = prevCameraDistance + (cameraDistance - prevCameraDistance) * partialTicks;
#else
        double camDist = prevCameraDistance + (cameraDistance - prevCameraDistance) * partialTicks;
#endif
        
        if (mc->gameSettings->field_22273_E)
        {
            const float camYaw = static_cast<float>(prevCameraYaw + (cameraYaw - prevCameraYaw) * partialTicks);
            const float camPitch = static_cast<float>(prevCameraPitch + (cameraPitch - prevCameraPitch) * partialTicks);
            
            renderTranslate(0.0f, 0.0f, (float)(-camDist));
            renderRotate(camPitch, 1.0f, 0.0f, 0.0f);
            renderRotate(camYaw, 0.0f, 1.0f, 0.0f);
        }
        else
        {
            float entityYaw = entityliving->rotationYaw;
            float entityPitch = entityliving->rotationPitch;
            if (mc->gameSettings->thirdPersonView == 2)
                entityPitch += 180.0f;
            
#if PLATFORM_FLOAT_VERTEX_MATH
            const float offsetX = -MathHelper::sin((entityYaw / 180.0f) * 3.1415927f) *
                                  MathHelper::cos((entityPitch / 180.0f) * 3.1415927f) * camDist;
            const float offsetZ = MathHelper::cos((entityYaw / 180.0f) * 3.1415927f) *
                                  MathHelper::cos((entityPitch / 180.0f) * 3.1415927f) * camDist;
            const float offsetY = -MathHelper::sin((entityPitch / 180.0f) * 3.1415927f) * camDist;
#else
            const double offsetX = (double)(-MathHelper::sin((entityYaw / 180.0f) * 3.1415927f) *
                                             MathHelper::cos((entityPitch / 180.0f) * 3.1415927f)) * camDist;
            const double offsetZ = (double)(MathHelper::cos((entityYaw / 180.0f) * 3.1415927f) *
                                             MathHelper::cos((entityPitch / 180.0f) * 3.1415927f)) * camDist;
            const double offsetY = (double)(-MathHelper::sin((entityPitch / 180.0f) * 3.1415927f)) * camDist;
#endif
            
            // Raycast para evitar paredes en tercera persona
            for (int i = 0; i < 8; i++)
            {
                float offsetXSign = (float)((i & 1) * 2 - 1) * 0.1f;
                float offsetYSign = (float)((i >> 1 & 1) * 2 - 1) * 0.1f;
                float offsetZSign = (float)((i >> 2 & 1) * 2 - 1) * 0.1f;
                
                Vec3D* rayStart = Vec3D::createVector(
                    static_cast<double>(posX + offsetXSign),
                    static_cast<double>(posY + offsetYSign),
                    static_cast<double>(posZ + offsetZSign)
                );

                Vec3D* rayEnd = Vec3D::createVector(
                    static_cast<double>((posX - offsetX) + offsetXSign + offsetZSign),
                    static_cast<double>((posY - offsetY) + offsetYSign),
                    static_cast<double>((posZ - offsetZ) + offsetZSign)  // Nota: Java tiene bug aqui con +f8 dos veces
                );
                
                MovingObjectPosition* mop = mc->theWorld->rayTraceBlocks(rayStart, rayEnd);
                
                if (mop == nullptr)
                {
                    continue;
                }
                
                double hitDist = mop->hitVec->distanceTo(Vec3D::createVector(posX, posY, posZ));
                // rayTraceBlocks() returns a heap MovingObjectPosition (Block::collisionRayTrace
                // does `new`). Java relied on GC; here this third-person-camera loop ran up to 8
                // times per frame and leaked every hit. The hitVec is pool-owned, so only the
                // MovingObjectPosition itself must be freed.
                delete mop;

#if PLATFORM_FLOAT_VERTEX_MATH
                if (static_cast<float>(hitDist) < camDist)
                {
                    camDist = static_cast<float>(hitDist);
                }
#else
                if (hitDist < camDist)
                {
                    camDist = hitDist;
                }
#endif
            }
            
            if (mc->gameSettings->thirdPersonView == 2)
                renderRotate(180.0f, 0.0f, 1.0f, 0.0f);
            renderRotate(entityliving->rotationPitch - entityPitch, 1.0f, 0.0f, 0.0f);
            renderRotate(entityliving->rotationYaw - entityYaw, 0.0f, 1.0f, 0.0f);
            renderTranslate(0.0f, 0.0f, (float)(-camDist));
            renderRotate(entityYaw - entityliving->rotationYaw, 0.0f, 1.0f, 0.0f);
            renderRotate(entityPitch - entityliving->rotationPitch, 1.0f, 0.0f, 0.0f);
        }
    }
    else
    {
        // Primera persona normal
        renderTranslate(0.0f, 0.0f, -0.1f);
    }
    
    if (!mc->gameSettings->field_22273_E)
    {
        renderRotate(entityliving->prevRotationPitch + 
                  (entityliving->rotationPitch - entityliving->prevRotationPitch) * partialTicks,
                  1.0f, 0.0f, 0.0f);
        
        renderRotate(entityliving->prevRotationYaw + 
                  (entityliving->rotationYaw - entityliving->prevRotationYaw) * partialTicks + 180.0f,
                  0.0f, 1.0f, 0.0f);
    }
    
    renderTranslate(0.0f, eyeHeight, 0.0f);
    
    // Recalcular posicion para cloudFog
    posX = entityliving->prevPosX + (entityliving->posX - entityliving->prevPosX) * (double)partialTicks;
    posY = (entityliving->prevPosY + (entityliving->posY - entityliving->prevPosY) * (double)partialTicks) - (double)eyeHeight;
    posZ = entityliving->prevPosZ + (entityliving->posZ - entityliving->prevPosZ) * (double)partialTicks;
    
    cloudFog = mc->renderGlobal->isCloudFog(posX, posY, posZ, partialTicks);
}

void EntityRenderer::setupCameraTransform(float partialTicks, int anaglyphPass)
{
    farPlaneDistance = static_cast<float>(Config::getRenderDistanceFine());
    // [WII 16:9] ConsoleAspectRatio calcula la relación de aspecto anamórfica exacta según mc->gameSettings->widescreen
    // para proyectar el mundo cúbico sin estiramiento horizontal en televisores 16:9 con señal EFB 640x480.
#if PLATFORM_FLOAT_VERTEX_MATH
    float projectionAspect = static_cast<float>(ConsoleAspectRatio::getProjectionAspect(
        mc->displayWidth, mc->displayHeight, mc->gameSettings->widescreen));
#else
    double projectionAspect = ConsoleAspectRatio::getProjectionAspect(
        mc->displayWidth, mc->displayHeight, mc->gameSettings->widescreen);
#endif
#if defined(PS2_PLATFORM)
    if (mc->isSplitScreenActive() && mc->thePlayer2 != nullptr && mc->gameSettings->widescreen)
    {
        projectionAspect *= 2.0;
    }
#endif
    
    renderMatrixMode(RenderMatrixMode::Projection);
    renderLoadIdentity();
    
    float anaglyphOffset = 0.07f;
    
    if (mc->gameSettings->anaglyph)
    {
        renderTranslate((float)(-(anaglyphPass * 2 - 1)) * anaglyphOffset, 0.0f, 0.0f);
    }
    
    if (cameraZoom != 1.0)
    {
        renderTranslate((float)cameraYawOffset, (float)(-cameraPitchOffset), 0.0f);
        renderScaleDouble(cameraZoom, cameraZoom, 1.0);
        
        perspectiveGL(getFOVModifier(partialTicks, true),
                      projectionAspect,
                      PLATFORM_NEAR_PLANE, farPlaneDistance * 2.0f);
    }
    else
    {
        perspectiveGL(getFOVModifier(partialTicks, true),
                      projectionAspect,
                      PLATFORM_NEAR_PLANE, farPlaneDistance * 2.0f);
    }
    
    if (mc->playerController != nullptr && mc->playerController->func_35643_e())
        renderScale(1.0f, 2.0f / 3.0f, 1.0f);

    renderMatrixMode(RenderMatrixMode::ModelView);
    renderLoadIdentity();
    
    if (mc->gameSettings->anaglyph)
    {
        renderTranslate((float)(anaglyphPass * 2 - 1) * 0.1f, 0.0f, 0.0f);
    }
    
#if !PLATFORM_SKIP_CAMERA_FX
    hurtCameraEffect(partialTicks);
    
    if (mc->gameSettings->viewBobbing)
    {
        setupViewBobbing(partialTicks);
    }
    
    // Efecto de portal (Nether)
    float portalTime = mc->thePlayer->prevTimeInPortal + 
                       (mc->thePlayer->timeInPortal - mc->thePlayer->prevTimeInPortal) * partialTicks;
    
    if (portalTime > 0.0f)
    {
        float portalScale = 5.0f / (portalTime * portalTime + 5.0f) - portalTime * 0.04f;
        portalScale *= portalScale;
        const float portalRotationSpeed = mc->thePlayer->isPotionActive(Potion::confusion) ? 7.0f : 20.0f;
        
        renderRotate(((float)rendererUpdateCount + partialTicks) * portalRotationSpeed, 0.0f, 1.0f, 1.0f);
        renderScale(1.0f / portalScale, 1.0f, 1.0f);
        renderRotate(-((float)rendererUpdateCount + partialTicks) * portalRotationSpeed, 0.0f, 1.0f, 1.0f);
    }
#endif
    
    orientCamera(partialTicks);

    if (debugViewDirection > 0)
    {
        const int_t direction = debugViewDirection - 1;
        if (direction == 1) renderRotate(90.0f, 0.0f, 1.0f, 0.0f);
        if (direction == 2) renderRotate(180.0f, 0.0f, 1.0f, 0.0f);
        if (direction == 3) renderRotate(-90.0f, 0.0f, 1.0f, 0.0f);
        if (direction == 4) renderRotate(90.0f, 1.0f, 0.0f, 0.0f);
        if (direction == 5) renderRotate(-90.0f, 1.0f, 0.0f, 0.0f);
    }
}

void EntityRenderer::renderHand(float partialTicks, int anaglyphPass)
{
    if (debugViewDirection > 0)
        return;

    // [WII 16:9] Utiliza projectionAspect idéntico al de la cámara del mundo para que la mano
    // en primera persona no sufra ensanchamiento visual en modo panorámico.
#if PLATFORM_FLOAT_VERTEX_MATH
    float projectionAspect = static_cast<float>(ConsoleAspectRatio::getProjectionAspect(
        mc->displayWidth, mc->displayHeight, mc->gameSettings->widescreen));
#else
    double projectionAspect = ConsoleAspectRatio::getProjectionAspect(
        mc->displayWidth, mc->displayHeight, mc->gameSettings->widescreen);
#endif
#if defined(PS2_PLATFORM)
    if (mc->isSplitScreenActive() && mc->thePlayer2 != nullptr && mc->gameSettings->widescreen)
    {
        projectionAspect *= 2.0;
    }
#endif

    renderMatrixMode(RenderMatrixMode::Projection);
    renderLoadIdentity();

    const float anaglyphOffset = 0.07f;
    if (mc->gameSettings->anaglyph)
    {
        renderTranslate((float)(-(anaglyphPass * 2 - 1)) * anaglyphOffset, 0.0f, 0.0f);
    }

    if (cameraZoom != 1.0)
    {
        renderTranslate((float)cameraYawOffset, (float)(-cameraPitchOffset), 0.0f);
        renderScaleDouble(cameraZoom, cameraZoom, 1.0);
    }

    perspectiveGL(getFOVModifier(partialTicks, false),
                  projectionAspect,
                  PLATFORM_NEAR_PLANE, farPlaneDistance * 2.0f);

    if (mc->playerController != nullptr && mc->playerController->func_35643_e())
        renderScale(1.0f, 2.0f / 3.0f, 1.0f);

    renderMatrixMode(RenderMatrixMode::ModelView);
    renderLoadIdentity();

    if (mc->gameSettings->anaglyph)
    {
        renderTranslate((float)(anaglyphPass * 2 - 1) * 0.1f, 0.0f, 0.0f);
    }

    renderPushMatrix();
    hurtCameraEffect(partialTicks);

    if (mc->gameSettings->viewBobbing)
    {
        setupViewBobbing(partialTicks);
    }

    if (mc->gameSettings->thirdPersonView == 0 &&
        !mc->renderViewEntity->isPlayerSleeping() &&
        !mc->gameSettings->hideGUI &&
        (mc->playerController == nullptr || !mc->playerController->func_35643_e()))
    {
        enableLightmap(partialTicks);
        itemRenderer->renderItemInFirstPerson(partialTicks);
        disableLightmap(partialTicks);
    }

    renderPopMatrix();

    if (mc->gameSettings->thirdPersonView == 0 && !mc->renderViewEntity->isPlayerSleeping())
    {
        itemRenderer->renderOverlays(partialTicks);
        hurtCameraEffect(partialTicks);
    }

    if (mc->gameSettings->viewBobbing)
    {
        setupViewBobbing(partialTicks);
    }
}

void EntityRenderer::updateCameraAndRender(float partialTicks)
{
    Profiler::profilingEnabled = Config::isProfilerEnabled();
    PlatformCompat::setSmoothInputThreadPriority(Config::isSmoothInput());

    // OptiFine C6 applies these detail settings continuously so changes remain
    // correct after a world/texture reload as well as after the option click.
    RenderBlocks::fancyGrass = Config::isGrassFancy() || Config::isBetterGrassFancy();
    if (Block::leaves != nullptr)
        Block::leaves->setGraphicsLevel(Config::isTreesFancy());

    World *settingsWorld = mc->theWorld;
    if (settingsWorld != nullptr)
    {
        WorldInfo *worldInfo = settingsWorld->getWorldInfo();
        if (!Config::isWeatherEnabled() && worldInfo != nullptr)
            worldInfo->setRaining(false);

        // C6 only forces time in local Creative worlds. Multiplayer time remains
        // server authoritative.
        if (!settingsWorld->multiplayerWorld && worldInfo != nullptr && worldInfo->getGameType() == 1)
        {
            const long_t time = worldInfo->getWorldTime();
            const long_t timeOfDay = time % 24000LL;
            if (Config::isTimeDayOnly())
            {
                if (timeOfDay <= 1000LL)
                    worldInfo->setWorldTime(time - timeOfDay + 1001LL);
                else if (timeOfDay >= 11000LL)
                    worldInfo->setWorldTime(time - timeOfDay + 24001LL);
            }
            else if (Config::isTimeNightOnly())
            {
                if (timeOfDay <= 14000LL)
                    worldInfo->setWorldTime(time - timeOfDay + 14001LL);
                else if (timeOfDay >= 22000LL)
                    worldInfo->setWorldTime(time - timeOfDay + 24000LL + 14001LL);
            }
        }
    }

    if (lightmapUpdateNeeded)
    {
#if PLATFORM_PC_LEGACY && PC_LEGACY_LIGHTMAP_CACHE
        World *lightmapWorld = mc->theWorld;
        if (lightmapWorld != nullptr && lightmapWorld->worldProvider != nullptr &&
            pcLegacyLightmapCache != nullptr &&
            pcLegacyLightmapCache->shouldUpdate(
                rendererUpdateCount, lightmapWorld->worldProvider,
                mc->gameSettings->ofBrightness, lightmapWorld->field_27172_i > 0,
                mc->gameSettings->legacyLook, true))
        {
            updateLightmap();
        }
#else
        updateLightmap();
#endif
    }

    // OptiFine: si cambio el worldProvider (p.ej. al entrar a un mundo o cambiar de
    // dimension) reaplica el brillo a la nueva tabla lightBrightnessTable.
    World* world = mc->theWorld;
    if (world != nullptr && world->worldProvider != nullptr)
    {
        if (updatedWorldProvider != world->worldProvider)
        {
            updateWorldLightLevels();
            updatedWorldProvider = world->worldProvider;
        }
    }

    bool isActive = lwjgl::Display::isActive();
    
    if (!isActive)
    {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - prevFrameTime).count();
        
        if (elapsed > 500)
        {
            mc->displayInGameMenu();
        }
    }
    else
    {
        prevFrameTime = std::chrono::steady_clock::now();
    }
    
    // Manejo de camara/mouse
    if (mc->inGameHasFocus && mc->thePlayer != nullptr)
    {
#if PLATFORM_DIRECT_ANALOG_MOVEMENT
#if PLATFORM_DIRECT_CAMERA_ENABLED
        // Direct-gamepad profiles read pad state directly instead of going through the fake mouse
        // queue. This prevents automatic camera rotation from stale mouse/menu
        // deltas while keeping PC behavior untouched.
        static std::chrono::steady_clock::time_point lastPadCameraTime = std::chrono::steady_clock::now();
        const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        float cameraDt = std::chrono::duration_cast<std::chrono::duration<float>>(now - lastPadCameraTime).count();
        lastPadCameraTime = now;
        if (cameraDt < 0.0f) cameraDt = 0.0f;
        if (cameraDt > PLATFORM_DIRECT_CAMERA_MAX_DT) cameraDt = PLATFORM_DIRECT_CAMERA_MAX_DT;

        const PlatformGamepadSnapshot pad = platformGamepadSnapshot(0);
        if (pad.connected)
        {
            const float rx = pad.rightX;
            const float ry = pad.rightY;
            if (rx != 0.0f || ry != 0.0f)
            {
                float sensitivity = mc->gameSettings->mouseSensitivity * 0.6f + 0.2f;
                float sensitivityCubed = sensitivity * sensitivity * sensitivity * PLATFORM_DIRECT_CAMERA_SCALE;
                // Preserve the previous 60 Hz feel, but do not make the look
                // speed depend on the current rendering frame rate.
                const float frameScale = cameraDt * PLATFORM_DIRECT_CAMERA_REFERENCE_FPS;
                float deltaX = rx * sensitivityCubed * frameScale;
                float deltaY = ry * sensitivityCubed * frameScale;
#if PLATFORM_DIRECT_CAMERA_INVERT_X
                deltaX = -deltaX;
#endif
#if PLATFORM_DIRECT_CAMERA_INVERT_Y
                deltaY = -deltaY;
#endif
                int invertMultiplier = mc->gameSettings->invertMouse ? -1 : 1;
                EntityPlayerSP *p1 = mc->thePlayerOne ? mc->thePlayerOne : mc->thePlayer;
                if (p1 != nullptr)
                    p1->turnEntity(deltaX, deltaY * (float)invertMultiplier);
            }
        }
#if defined(PS2_PLATFORM)
        if (mc->isSplitScreenActive() && mc->thePlayer2 != nullptr)
        {
            const PlatformGamepadSnapshot pad2 = platformGamepadSnapshot(1);
            if (pad2.connected)
            {
                const float rx2 = pad2.rightX;
                const float ry2 = pad2.rightY;
                if (rx2 != 0.0f || ry2 != 0.0f)
                {
                    float sensitivity = mc->gameSettings->mouseSensitivity * 0.6f + 0.2f;
                    float sensitivityCubed = sensitivity * sensitivity * sensitivity * PLATFORM_DIRECT_CAMERA_SCALE;
                    const float frameScale = cameraDt * PLATFORM_DIRECT_CAMERA_REFERENCE_FPS;
                    float deltaX = rx2 * sensitivityCubed * frameScale;
                    float deltaY = ry2 * sensitivityCubed * frameScale;
#if PLATFORM_DIRECT_CAMERA_INVERT_X
                    deltaX = -deltaX;
#endif
#if PLATFORM_DIRECT_CAMERA_INVERT_Y
                    deltaY = -deltaY;
#endif
                    int invertMultiplier = mc->gameSettings->invertMouse ? -1 : 1;
                    mc->thePlayer2->turnEntity(deltaX, deltaY * (float)invertMultiplier);
                }
            }
        }
#endif
#else
        mc->mouseHelper->mouseXYChange();
        float sensitivity = mc->gameSettings->mouseSensitivity * 0.6f + 0.2f;
        float sensitivityCubed = sensitivity * sensitivity * sensitivity * 8.0f;
        float deltaX = (float)mc->mouseHelper->deltaX * sensitivityCubed;
        float deltaY = (float)mc->mouseHelper->deltaY * sensitivityCubed;
        int invertMultiplier = mc->gameSettings->invertMouse ? -1 : 1;
        mc->thePlayer->turnEntity(deltaX, deltaY * (float)invertMultiplier);
#endif
#else
        mc->mouseHelper->mouseXYChange();
        
        float sensitivity = mc->gameSettings->mouseSensitivity * 0.6f + 0.2f;
        float sensitivityCubed = sensitivity * sensitivity * sensitivity * 8.0f;
        
        float deltaX = (float)mc->mouseHelper->deltaX * sensitivityCubed;
        float deltaY = (float)mc->mouseHelper->deltaY * sensitivityCubed;
        
        int invertMultiplier = mc->gameSettings->invertMouse ? -1 : 1;
        
        if (mc->gameSettings->smoothCamera)
        {
            smoothCamYaw += deltaX;
            smoothCamPitch += deltaY;
            const float framePartial = partialTicks - smoothCamPartialTicks;
            smoothCamPartialTicks = partialTicks;
            deltaX = smoothCamFilterX * framePartial;
            deltaY = smoothCamFilterY * framePartial;
        }
        
        mc->thePlayer->turnEntity(deltaX, deltaY * (float)invertMultiplier);
#endif
    }
    
    if (mc->skipRenderWorld)
    {
        return;
    }
    
    anaglyphEnabled = mc->gameSettings->anaglyph;
    
    ScaledResolution scaledResolution(mc->gameSettings, mc->displayWidth, mc->displayHeight);
    int scaledWidth = scaledResolution.getScaledWidth();
    int scaledHeight = scaledResolution.getScaledHeight();
    
    // Posicion del mouse escalada
    int mouseX, mouseY;
    PlatformCompat::getMouseState(&mouseX, &mouseY);
    int scaledMouseX = (mouseX * scaledWidth) / mc->displayWidth;
    int scaledMouseY = (mouseY * scaledHeight) / mc->displayHeight;
    
    // Caracteres para limitFramerate: 0='\0', 1='x' (120), 2='(' (40)
    char fpsLimitChar = '\0';
    if (mc->gameSettings->limitFramerate == 1)
    {
        fpsLimitChar = 'x';  // ~120 fps
    }
    else if (mc->gameSettings->limitFramerate == 2)
    {
        fpsLimitChar = '(';  // ~40 fps
    }
    // [FIX CRÍTICO WII] Prevenir división por cero si fpsLimitChar es '\0' (0).
    // Si limitFramerate es 0 (ilimitado) o tiene un valor anómalo, establecemos 120L para evitar congelamiento fatal en PowerPC.
    const long limitFps = (fpsLimitChar > '\0') ? static_cast<long>(fpsLimitChar) : 120L;
    
    if (mc->theWorld != nullptr)
    {
#if PLATFORM_PS2
        renderSetLegacyPresentationGamma(mc->gameSettings != nullptr && mc->gameSettings->legacyLook);
#endif
        int64_t targetTime = 0;
        if (mc->gameSettings->limitFramerate != 0)
        {
            targetTime = field_28133_I + (int64_t)(1000000000LL / limitFps);
        }

        if (mc->isSplitScreenActive() && mc->thePlayer2 != nullptr)
        {
            renderSplitScreen(partialTicks, targetTime);
        }
        else
        {
            renderWorld(partialTicks, targetTime);

#if PLATFORM_PS2
            renderSetLegacyPresentationGamma(false);
#endif

            legacyLookApplyWorldGrade(mc);

            // Renderizar GUI
            if (!mc->gameSettings->hideGUI || mc->currentScreen != nullptr)
            {
#if PLATFORM_PROFILE_RENDER_PHASES
                const std::uint32_t cycHud = platformProfileRenderPhaseBegin();
#endif
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
                const PlatformDrawSnapshot hudDrawStart = platformProfileDrawSnapshot();
#endif
                mc->ingameGUI->renderGameOverlay(partialTicks, mc->currentScreen != nullptr,
                                                 scaledMouseX, scaledMouseY);
#if PLATFORM_PROFILE_RENDER_PHASES
                platformProfileRenderPhaseEnd(cycHud, PlatformRenderPhase::Hud);
#endif
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
                platformProfileDrawCategory(PlatformDrawCategory::Gui, hudDrawStart);
#endif
            }
        }
    }
    else
    {
        // Menu principal / sin mundo
        renderViewport(0, 0, mc->displayWidth, mc->displayHeight);
        renderMatrixMode(RenderMatrixMode::Projection);
        renderLoadIdentity();
        renderMatrixMode(RenderMatrixMode::ModelView);
        renderLoadIdentity();
        
        setupOverlayRendering();
        
        if (mc->gameSettings->limitFramerate == 2)
        {
            int64_t sleepTime = (field_28133_I + (int64_t)(1000000000LL / limitFps) - 
                                std::chrono::duration_cast<std::chrono::nanoseconds>(
                                    std::chrono::steady_clock::now().time_since_epoch()).count()) / 1000000LL;
            
            if (sleepTime < 0)
            {
                sleepTime += 10;
            }
            
            if (sleepTime > 0 && sleepTime < 500)
            {
                PlatformCompat::delay((uint32_t)sleepTime);
            }
        }
        
        field_28133_I = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }
    
    // Renderizar pantalla actual (GUI)
    if (mc->currentScreen != nullptr)
    {
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
        const PlatformDrawSnapshot screenDrawStart = platformProfileDrawSnapshot();
#endif
#if PLATFORM_GUI_FORCE_DEPTH_DISABLED
        // Native console GUI backends must not inherit depth state from either
        // the world or the Mojang splash. Screens that render 3D items
        // explicitly enable depth around those draws.
        // The depth buffer is still cleared below because renderClear() forces
        // GX Z writes independently of the logical DepthTest state.
        renderDisable(RenderCapability::DepthTest);
#endif
        renderClear(RenderClearMask::Depth);  // 256 = GL_DEPTH_BUFFER_BIT

        mc->currentScreen->drawScreen(scaledMouseX, scaledMouseY, partialTicks);

#if PLATFORM_HAS_VIRTUAL_KEYBOARD
        // On-screen keyboard overlay (drawn on top of the focused text screen).
        if (VirtualKeyboard::instance().isActive())
            VirtualKeyboard::instance().render(mc->fontRenderer,
                mc->currentScreen->width, mc->currentScreen->height);
#endif

        if (mc->currentScreen != nullptr && mc->currentScreen->guiParticles != nullptr)
        {
            mc->currentScreen->guiParticles->renderParticles(partialTicks);
        }
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
        platformProfileDrawCategory(PlatformDrawCategory::Gui, screenDrawStart);
#endif
    }
}

void EntityRenderer::renderSplitScreen(float partialTicks, int64_t renderTimeLimitNano)
{
    EntityPlayerSP *entryPlayer = mc->thePlayer;
    EntityPlayerSP *p1 = mc->thePlayerOne ? mc->thePlayerOne : mc->thePlayer;
    EntityPlayerSP *p2 = mc->thePlayer2;
    if (p1 == nullptr || p2 == nullptr)
        return;

    const int fullW = mc->displayWidth;
    const int fullH = mc->displayHeight;
    const int halfH = fullH / 2;

    MovingObjectPosition *hr1 = nullptr;

    for (int i = 0; i < 2; ++i)
    {
        EntityPlayerSP *lp = (i == 0) ? p1 : p2;
        mc->thePlayer = lp;
        mc->renderViewEntity = lp;
        mc->displayHeight = halfH;
        viewportOffsetY = i * halfH;

        if (itemRenderer != nullptr)
            itemRenderer->refreshItem();

        renderViewport(0, viewportOffsetY, fullW, halfH);

        getMouseOver(partialTicks);
        if (i == 0)
        {
            hr1 = mc->objectMouseOver;
            mc->objectMouseOver = nullptr;
        }
        else
        {
            delete mc->objectMouseOver2;
            mc->objectMouseOver2 = mc->objectMouseOver;
            mc->objectMouseOver = nullptr;
        }

        renderWorld(partialTicks, renderTimeLimitNano);

        if (!mc->gameSettings->hideGUI || mc->currentScreen != nullptr)
        {
            setupOverlayRendering();
            ScaledResolution scaledResolution(mc->gameSettings, mc->displayWidth, mc->displayHeight);
            int scaledWidth = scaledResolution.getScaledWidth();
            int scaledHeight = scaledResolution.getScaledHeight();
            int mouseX, mouseY;
            PlatformCompat::getMouseState(&mouseX, &mouseY);
            int scaledMouseX = (mouseX * scaledWidth) / mc->displayWidth;
            int scaledMouseY = (mouseY * scaledHeight) / mc->displayHeight;

            mc->ingameGUI->renderGameOverlay(partialTicks, mc->currentScreen != nullptr,
                                             scaledMouseX, scaledMouseY);
        }
    }

    viewportOffsetY = 0;
    mc->displayHeight = fullH;
    mc->thePlayer = entryPlayer;
    mc->renderViewEntity = p1;
    mc->objectMouseOver = hr1;

    renderViewport(0, 0, fullW, fullH);

    // Render 2px black horizontal split divider bar
    setupOverlayRendering();
    ScaledResolution fullResolution(mc->gameSettings, fullW, fullH);
    int guiW = fullResolution.getScaledWidth();
    int guiH = fullResolution.getScaledHeight();
    int midY = guiH / 2;
    Gui::drawRect(0, midY - 1, guiW, midY + 1, 0xFF000000);
}

void EntityRenderer::renderWorld(float partialTicks, int64_t renderTimeLimitNano)
{
    renderEnable(RenderCapability::CullFace);
    renderEnable(RenderCapability::DepthTest);
    
    if (mc->renderViewEntity == nullptr)
    {
        mc->renderViewEntity = mc->thePlayer;
    }
    
#if !PLATFORM_SKIP_BLOCK_SELECTION_BOX || PLATFORM_ENABLE_BLOCK_RAYTRACE
    // PS2 still needs rayTrace for placing/breaking blocks and for the crack overlay.
    // Only the visual selection box is optional.
    getMouseOver(partialTicks);
#else
    mc->objectMouseOver = nullptr;
#endif
    
    EntityLiving* entityliving = mc->renderViewEntity;
    RenderGlobal* renderglobal = mc->renderGlobal;
    EffectRenderer* effectrenderer = mc->effectRenderer;
    
#if PLATFORM_FLOAT_VERTEX_MATH
    const float renderPosX = static_cast<float>(entityliving->lastTickPosX) +
        (static_cast<float>(entityliving->posX) - static_cast<float>(entityliving->lastTickPosX)) * partialTicks;
    const float renderPosY = static_cast<float>(entityliving->lastTickPosY) +
        (static_cast<float>(entityliving->posY) - static_cast<float>(entityliving->lastTickPosY)) * partialTicks;
    const float renderPosZ = static_cast<float>(entityliving->lastTickPosZ) +
        (static_cast<float>(entityliving->posZ) - static_cast<float>(entityliving->lastTickPosZ)) * partialTicks;
#else
    const double renderPosX = entityliving->lastTickPosX + (entityliving->posX - entityliving->lastTickPosX) * (double)partialTicks;
    const double renderPosY = entityliving->lastTickPosY + (entityliving->posY - entityliving->lastTickPosY) * (double)partialTicks;
    const double renderPosZ = entityliving->lastTickPosZ + (entityliving->posZ - entityliving->lastTickPosZ) * (double)partialTicks;
#endif
    
    IChunkProvider* chunkProvider = mc->theWorld->getIChunkProvider();
    
    int chunkX = JavaArithmetic::intShr(MathHelper::floor_double(renderPosX), 4);
    int chunkZ = JavaArithmetic::intShr(MathHelper::floor_double(renderPosZ), 4);
    if (ChunkProviderLoadOrGenerate* chunkLoadOrGen = dynamic_cast<ChunkProviderLoadOrGenerate*>(chunkProvider))
    {
        chunkLoadOrGen->setChunkLoadRadiusFromRenderDistance(mc->gameSettings->renderDistance);
        chunkLoadOrGen->setCurrentChunkOver(chunkX, chunkZ);
    }
    else if (ChunkProvider* chunkProviderMap = dynamic_cast<ChunkProvider*>(chunkProvider))
    {
        chunkProviderMap->setChunkLoadRadiusFromRenderDistance(mc->gameSettings->renderDistance);
        chunkProviderMap->setCurrentChunkOver(chunkX, chunkZ);
#if PLATFORM_ASYNC_CHUNK_GENERATION
        chunkProviderMap->serviceAsyncChunkStreaming();
#endif
#if PLATFORM_INCREMENTAL_CHUNK_GENERATION
        chunkProviderMap->serviceFrameGeneration();
#endif
    }
#if PLATFORM_ENABLE_CHUNK_PREFETCH
    prefetchNearbyChunks(chunkProvider, chunkX, chunkZ, mc->gameSettings->renderDistance,
                         entityliving->motionX, entityliving->motionZ);
#endif
    
    // Loop para anaglifo (0 = ojo izquierdo, 1 = ojo derecho)
    for (int eye = 0; eye < 2; eye++)
    {
        if (mc->gameSettings->anaglyph)
        {
            anaglyphField = eye;
            
            if (anaglyphField == 0)
            {
                renderColorMask(false, true, true, false);
            }
            else
            {
                renderColorMask(true, false, false, false);
            }
        }
        
        renderViewport(0, viewportOffsetY, mc->displayWidth, mc->displayHeight);
        
        updateFogColor(partialTicks);


        renderClear(RenderClearMask::Color | RenderClearMask::Depth);  // 16640
        
        renderEnable(RenderCapability::CullFace);
        setupCameraTransform(partialTicks, eye);
        ActiveRenderInfo::updateRenderInfo(mc->thePlayer, mc->gameSettings->thirdPersonView == 2);
#if PLATFORM_NATIVE_TERRAIN_PIPELINE
        renderTerrainCaptureCamera();
#endif
        
        ClippingHelperImpl::getInstance();
        
        // Renderizar cielo (solo en distancias cortas)
        if (mc->gameSettings->renderDistance < 2)
        {
            setupFog(-1, partialTicks);
#if PLATFORM_PROFILE_RENDER_PHASES
            const std::uint32_t cycSky = platformProfileRenderPhaseBegin();
#endif
            renderglobal->renderSky(partialTicks);
#if PLATFORM_PROFILE_RENDER_PHASES
            platformProfileRenderPhaseEnd(cycSky, PlatformRenderPhase::Sky);
#endif
        }
        
        renderEnable(RenderCapability::Fog);
        setupFog(1, partialTicks);
        
        if (mc->gameSettings->ambientOcclusion)
        {
            renderShadeModel(RenderShadeModel::Smooth);
        }
        
        // Frustum culling
        Frustrum frustrum;
        frustrum.setPosition(renderPosX, renderPosY, renderPosZ);
        
#if PLATFORM_PROFILE_RENDER_PHASES
        const std::uint32_t cycFrustum = platformProfileRenderPhaseBegin();
#endif
        mc->renderGlobal->clipRenderersByFrustrum(&frustrum, partialTicks);
#if PLATFORM_PROFILE_RENDER_PHASES
        platformProfileRenderPhaseEnd(cycFrustum, PlatformRenderPhase::Frustum);
#endif

        // Actualizar renderers (construccion de chunks)
        if (eye == 0)
        {
#if PLATFORM_MESH_BUDGET
            // One renderer budget step per rendered frame. The vanilla
            // time-limit loop below calls updateRenderers repeatedly within the
            // same frame, which defeats a per-call budget completely: the budget
            // bounds one call, the loop just makes the call again until the same
            // total work is done, and chunk loading is the same large hitch it
            // was with the meter reading "6 ms" every time.
            //
            // This is why PLATFORM_MESH_BUDGET gates both halves. Turning on the
            // budget without turning off this loop measures beautifully and
            // changes nothing the player can feel.
#if PLATFORM_PROFILE_RENDER_PHASES
            const std::uint32_t cycBuild = platformProfileRenderPhaseBegin();
#endif
            mc->renderGlobal->updateRenderers(entityliving, false);
#if PLATFORM_PROFILE_RENDER_PHASES
            platformProfileRenderPhaseEnd(cycBuild, PlatformRenderPhase::Build);
#endif
#else
            int64_t timeLeft;
            do
            {
                if (mc->renderGlobal->updateRenderers(entityliving, false) || renderTimeLimitNano == 0)
                {
                    break;
                }
                
                timeLeft = renderTimeLimitNano - std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count();
            }
            while (timeLeft >= 0 && timeLeft <= 1000000000LL);  // 0x3b9aca00
#endif
        }
        
        // Renderizar solidos (paso 0)
        setupFog(0, partialTicks);
        // The Wii native terrain pass reads the tracked fog state like every
        // other renderer instead of forcing fog off, so both paths enable it
        // here rather than only in the fallback below.
        renderEnable(RenderCapability::Fog);
        const int terrainTexture = mc->renderEngine->getTexture("/terrain.png");
        const bool terrainOpaquePass = renderTerrainBeginPass(terrainTexture, RenderTerrainPass::Opaque);
        
        RenderHelper::disableStandardItemLighting();
#if PLATFORM_PROFILE_RENDER_PHASES
        const std::uint32_t cycPass0 = platformProfileRenderPhaseBegin();
#endif
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
        const PlatformDrawSnapshot opaqueDrawStart = platformProfileDrawSnapshot();
#endif
        renderglobal->sortAndRender(entityliving, 0, partialTicks);
        if (terrainOpaquePass)
            renderTerrainEndPass(RenderTerrainPass::Opaque);
#if PLATFORM_PROFILE_RENDER_PHASES
        platformProfileRenderPhaseEnd(cycPass0, PlatformRenderPhase::Opaque);
#endif
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
        platformProfileDrawCategory(PlatformDrawCategory::Terrain, opaqueDrawStart);
#endif

        renderShadeModel(RenderShadeModel::Flat);
        RenderHelper::enableStandardItemLighting();
        
        // Entidades
#if PLATFORM_PROFILE_RENDER_PHASES
        const std::uint32_t cycEnts = platformProfileRenderPhaseBegin();
#endif
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
        const PlatformDrawSnapshot entitiesDrawStart = platformProfileDrawSnapshot();
#endif
#if !PLATFORM_SKIP_WORLD_ENTITIES
        renderglobal->renderEntities(entityliving->getPosition(partialTicks), &frustrum, partialTicks);
#else
        // Low-cost profiles may skip ordinary world entities, but keep the
        // camera/player entity in third person so F5 remains usable.
        if (mc->gameSettings->thirdPersonView)
            renderglobal->renderEntities(entityliving->getPosition(partialTicks), &frustrum, partialTicks);
#endif
#if PLATFORM_PROFILE_RENDER_PHASES
        platformProfileRenderPhaseEnd(cycEnts, PlatformRenderPhase::Entities);
#endif
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
        platformProfileDrawCategory(PlatformDrawCategory::Entities, entitiesDrawStart);
#endif
        
        // Efectos (particulas de destruccion de bloques, etc.)
#if !PLATFORM_SKIP_WORLD_PARTICLES
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
        const PlatformDrawSnapshot particlesDrawStart = platformProfileDrawSnapshot();
#endif
        enableLightmap(partialTicks);
        effectrenderer->renderLitParticles(entityliving, partialTicks);
#endif
        
        RenderHelper::disableStandardItemLighting();
        
        setupFog(0, partialTicks);
        
        // Particulas
#if !PLATFORM_SKIP_WORLD_PARTICLES
        effectrenderer->renderParticles(entityliving, partialTicks);
        disableLightmap(partialTicks);
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
        platformProfileDrawCategory(PlatformDrawCategory::Particles, particlesDrawStart);
#endif
#endif
        
        // Seleccion de bloque bajo agua
#if !PLATFORM_SKIP_BLOCK_SELECTION_BOX || PLATFORM_ENABLE_BLOCK_BREAK_OVERLAY
        if (mc->objectMouseOver != nullptr && 
            entityliving->isInsideOfMaterial(Material::water) && 
            entityliving->isPlayer())
        {
            EntityPlayer* entityplayer = static_cast<EntityPlayer*>(entityliving);
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
            const PlatformDrawSnapshot selectionDrawStart = platformProfileDrawSnapshot();
#endif
            
            renderDisable(RenderCapability::AlphaTest);
            renderglobal->drawBlockBreaking(entityplayer, mc->objectMouseOver, 0, 
                                           entityplayer->inventory->getCurrentItem(), partialTicks);
#if !PLATFORM_SKIP_BLOCK_SELECTION_BOX
            renderglobal->drawSelectionBox(entityplayer, mc->objectMouseOver, 0,
                                          entityplayer->inventory->getCurrentItem(), partialTicks);
#endif
            renderEnable(RenderCapability::AlphaTest);
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
            platformProfileDrawCategory(PlatformDrawCategory::Selection, selectionDrawStart);
#endif
        }
#endif
        
        // Renderizar transparentes (paso 1)
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
        const PlatformDrawSnapshot translucentDrawStart = platformProfileDrawSnapshot();
#endif
#if !PLATFORM_SIMPLE_TRANSPARENT_TERRAIN
#if !PLATFORM_NATIVE_TERRAIN_PIPELINE
        renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
#endif
        setupFog(0, partialTicks);
#if !PLATFORM_NATIVE_TERRAIN_PIPELINE
        renderEnable(RenderCapability::Blend);
        renderDisable(RenderCapability::CullFace);
#endif
        mc->renderEngine->bindTexture(mc->renderEngine->getTexture("/terrain.png"));
#if PLATFORM_NATIVE_TERRAIN_PIPELINE
        bool nativeTransparentUsedFallback = false;
#endif
        
        if (Config::isWaterFancy())
        {
            if (mc->gameSettings->ambientOcclusion)
            {
                renderShadeModel(RenderShadeModel::Smooth);
            }
            
            // Renderizar a color de ataque para transparencias (depth peeling simple)
#if !PLATFORM_NATIVE_TERRAIN_PIPELINE
            renderColorMask(false, false, false, false);
#endif
#if PLATFORM_NATIVE_TERRAIN_PIPELINE
            const bool nativeTransparentDepthPass = renderTerrainBeginPass(terrainTexture, RenderTerrainPass::TranslucentDepth);
            if (!nativeTransparentDepthPass)
            {
                nativeTransparentUsedFallback = true;
                renderEnable(RenderCapability::Fog);
                renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
                renderEnable(RenderCapability::Blend);
                renderDisable(RenderCapability::CullFace);
                renderColorMask(false, false, false, false);
            }
#endif
            int transparentCount = renderglobal->sortAndRender(entityliving, 1, partialTicks);
#if PLATFORM_NATIVE_TERRAIN_PIPELINE
            if (nativeTransparentDepthPass)
                renderTerrainEndPass(RenderTerrainPass::Translucent);
#endif
            
#if !PLATFORM_NATIVE_TERRAIN_PIPELINE
            if (mc->gameSettings->anaglyph)
            {
                if (anaglyphField == 0)
                {
                    renderColorMask(false, true, true, true);
                }
                else
                {
                    renderColorMask(true, false, false, true);
                }
            }
            else
            {
                renderColorMask(true, true, true, true);
            }
#endif
            
            if (transparentCount > 0)
            {
#if PLATFORM_NATIVE_TERRAIN_PIPELINE
                const bool nativeTransparentColorPass = renderTerrainBeginPass(terrainTexture, RenderTerrainPass::TranslucentColor);
                if (!nativeTransparentColorPass)
                {
                    nativeTransparentUsedFallback = true;
                    renderEnable(RenderCapability::Fog);
                    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
                    renderEnable(RenderCapability::Blend);
                    renderDisable(RenderCapability::CullFace);
                    renderColorMask(true, true, true, true);
                }
#endif
#if !PLATFORM_PS2
                renderglobal->renderAllRenderLists(1, partialTicks);
#endif
#if PLATFORM_NATIVE_TERRAIN_PIPELINE
                if (nativeTransparentColorPass)
                    renderTerrainEndPass(RenderTerrainPass::Translucent);
#endif
            }
            
            renderShadeModel(RenderShadeModel::Flat);
        }
        else
        {
#if PLATFORM_NATIVE_TERRAIN_PIPELINE
            const bool nativeTransparentPass = renderTerrainBeginPass(terrainTexture, RenderTerrainPass::TranslucentColor);
            if (!nativeTransparentPass)
            {
                nativeTransparentUsedFallback = true;
                renderEnable(RenderCapability::Fog);
                renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
                renderEnable(RenderCapability::Blend);
                renderDisable(RenderCapability::CullFace);
            }
#endif
            renderglobal->sortAndRender(entityliving, 1, partialTicks);
#if PLATFORM_NATIVE_TERRAIN_PIPELINE
            if (nativeTransparentPass)
                renderTerrainEndPass(RenderTerrainPass::Translucent);
#endif
        }
        
#if !PLATFORM_NATIVE_TERRAIN_PIPELINE
        renderDepthMask(true);
        renderEnable(RenderCapability::CullFace);
        renderDisable(RenderCapability::Blend);
#elif PLATFORM_NATIVE_TERRAIN_PIPELINE
        if (nativeTransparentUsedFallback)
        {
            renderDepthMask(true);
            renderEnable(RenderCapability::CullFace);
            renderDisable(RenderCapability::Blend);
            renderColorMask(true, true, true, true);
        }
#endif
#else
        // PS2: simple transparent pass (water, ice, glass, portals). No depth
        // peeling or color-mask tricks like the PC path above -- just alpha-blend
        // the captured pass-1 geometry over the opaque terrain. Gated by the
        // PLATFORM_SKIP_TRANSPARENT_WORLD_PASS tuning flag.
#if !PLATFORM_SKIP_TRANSPARENT_WORLD_PASS
        setupFog(0, partialTicks);
        // Texture binding, fixed-factor alpha blend, depth-write disable and
        // cull disable are PS2 terrain-pipeline state now. Keeping them out of
        // the public GL API makes pass 1 use the same native setup as pass 0 and
        // avoids translating a known fixed state every frame.
		renderTerrainBeginPass(terrainTexture, RenderTerrainPass::Translucent);
		const std::uint32_t cycPass1 = platformProfileRenderPhaseBegin();
		renderglobal->sortAndRender(entityliving, 1, partialTicks);
		renderTerrainEndPass(RenderTerrainPass::Translucent);
		platformProfileRenderPhaseEnd(cycPass1, PlatformRenderPhase::Translucent);
#endif
#endif
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
        platformProfileDrawCategory(PlatformDrawCategory::Terrain, translucentDrawStart);
#endif

        // Seleccion de bloque (fuera de agua)
#if !PLATFORM_SKIP_BLOCK_SELECTION_BOX || PLATFORM_ENABLE_BLOCK_BREAK_OVERLAY
        if (cameraZoom == 1.0 && 
            entityliving->isPlayer() &&
            mc->objectMouseOver != nullptr &&
            !entityliving->isInsideOfMaterial(Material::water))
        {
            EntityPlayer* entityplayer1 = (EntityPlayer*)entityliving;
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
            const PlatformDrawSnapshot selectionDrawStart2 = platformProfileDrawSnapshot();
#endif
            
            renderDisable(RenderCapability::AlphaTest);
            renderglobal->drawBlockBreaking(entityplayer1, mc->objectMouseOver, 0,
                                           entityplayer1->inventory->getCurrentItem(), partialTicks);
#if !PLATFORM_SKIP_BLOCK_SELECTION_BOX
            renderglobal->drawSelectionBox(entityplayer1, mc->objectMouseOver, 0,
                                          entityplayer1->inventory->getCurrentItem(), partialTicks);
#endif
            renderEnable(RenderCapability::AlphaTest);
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
            platformProfileDrawCategory(PlatformDrawCategory::Selection, selectionDrawStart2);
#endif
        }
#endif
        
        // Lluvia y nieve
#if !PLATFORM_SKIP_RAIN_SNOW
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
        const PlatformDrawSnapshot weatherDrawStart = platformProfileDrawSnapshot();
#endif
        renderRainSnow(partialTicks);
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
        platformProfileDrawCategory(PlatformDrawCategory::Weather, weatherDrawStart);
#endif
#endif
        
        // Niebla para nubes
        renderDisable(RenderCapability::Fog);
        setupFog(0, partialTicks);
        renderEnable(RenderCapability::Fog);
#if !PLATFORM_SKIP_CLOUDS
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
        const PlatformDrawSnapshot cloudsDrawStart = platformProfileDrawSnapshot();
#endif
        renderglobal->renderClouds(partialTicks);
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
        platformProfileDrawCategory(PlatformDrawCategory::Clouds, cloudsDrawStart);
#endif
#endif
        
        renderDisable(RenderCapability::Fog);
        setupFog(1, partialTicks);
        
        // Renderizar mano y overlays
        if (cameraZoom == 1.0)
        {
            renderClear(RenderClearMask::Depth);  // 256
#if !PLATFORM_SKIP_HAND_RENDER
#if PLATFORM_PROFILE_RENDER_PHASES
            const std::uint32_t cycHand = platformProfileRenderPhaseBegin();
#endif
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
            const PlatformDrawSnapshot handDrawStart = platformProfileDrawSnapshot();
#endif
            renderHand(partialTicks, eye);
#if PLATFORM_PROFILE_RENDER_PHASES
            platformProfileRenderPhaseEnd(cycHand, PlatformRenderPhase::Hand);
#endif
#if PLATFORM_PS2 && MC_LOG_LEVEL > 2
            platformProfileDrawCategory(PlatformDrawCategory::Hand, handDrawStart);
#endif
#endif
        }
        
        if (!mc->gameSettings->anaglyph)
        {
            return;
        }
    }
    
    // Restaurar color mask
    renderColorMask(true, true, true, false);
}

void EntityRenderer::addRainParticles()
{
    float rainStrength = mc->theWorld->getRainStrength(1.0f);
    if (!Config::isRainFancy())
        rainStrength /= 2.0f;

    if (rainStrength == 0.0f)
        return;
    if (!Config::isRainSplash())
        return;

    random.setSeed(static_cast<long_t>(rendererUpdateCount) * 312987231LL);

    EntityLiving* entity = mc->renderViewEntity;
    World* world = mc->theWorld;
    const int_t centerX = MathHelper::floor_double(entity->posX);
    const int_t centerY = MathHelper::floor_double(entity->posY);
    const int_t centerZ = MathHelper::floor_double(entity->posZ);
    const int_t range = 10;
    double soundX = 0.0;
    double soundY = 0.0;
    double soundZ = 0.0;
    int_t rainParticleCount = 0;
    int_t particleCount = static_cast<int_t>(100.0f * rainStrength * rainStrength);
#if PLATFORM_PS2
    // PS2 does not draw the full weather curtains, but this splash path still
    // ran the vanilla 100-attempt burst every tick. Bound it before applying the
    // user's particle setting so "Decreased" still halves the console budget.
    if (particleCount > PS2_RAIN_SPLASH_PARTICLES_PER_TICK)
        particleCount = PS2_RAIN_SPLASH_PARTICLES_PER_TICK;
#endif

    if (mc->gameSettings->particleSetting == 1)
        particleCount >>= 1;
    else if (mc->gameSettings->particleSetting == 2)
        particleCount = 0;

    for (int_t i = 0; i < particleCount; ++i)
    {
        const int_t x = random.nextIntOffset(centerX, range);
        const int_t z = random.nextIntOffset(centerZ, range);
        const int_t precipitationY = world->getPrecipitationHeight(x, z);
        const int_t blockIdBelow = world->getBlockId(x, precipitationY - 1, z);
        BiomeGenBase* biome = world->getBiomeGenForCoords(x, z);

        if (precipitationY > centerY + range || precipitationY < centerY - range ||
            biome == nullptr || !biome->canSpawnLightningBolt() || biome->getFloatTemperature() <= 0.2f)
        {
            continue;
        }

        const float offsetX = random.nextFloat();
        const float offsetZ = random.nextFloat();
        if (blockIdBelow <= 0 || blockIdBelow >= Block::BLOCK_REGISTRY_SIZE)
            continue;

        Block* blockBelow = Block::blocksList[blockIdBelow];
        if (blockBelow == nullptr)
            continue;

        const double particleY = static_cast<double>(static_cast<float>(precipitationY) + 0.1f) - blockBelow->minY;
        if (blockBelow->blockMaterial == Material::lava)
        {
            mc->effectRenderer->addEffect(new EntitySmokeFX(
                world, static_cast<double>(static_cast<float>(x) + offsetX), particleY,
                static_cast<double>(static_cast<float>(z) + offsetZ), 0.0, 0.0, 0.0));
        }
        else
        {
            ++rainParticleCount;
            if (random.nextInt(rainParticleCount) == 0)
            {
                soundX = static_cast<double>(static_cast<float>(x) + offsetX);
                soundY = particleY;
                soundZ = static_cast<double>(static_cast<float>(z) + offsetZ);
            }

            mc->effectRenderer->addEffect(new EntityRainFX(
                world, static_cast<double>(static_cast<float>(x) + offsetX), particleY,
                static_cast<double>(static_cast<float>(z) + offsetZ)));
        }
    }

    if (rainParticleCount > 0 && random.nextInt(3) < rainSoundCounter++)
    {
        rainSoundCounter = 0;
        if (soundY > entity->posY + 1.0 &&
            world->getPrecipitationHeight(MathHelper::floor_double(entity->posX),
                                          MathHelper::floor_double(entity->posZ)) >
                MathHelper::floor_double(entity->posY))
        {
            world->playSoundEffect(soundX, soundY, soundZ, "ambient.weather.rain", 0.1f, 0.5f);
        }
        else
        {
            world->playSoundEffect(soundX, soundY, soundZ, "ambient.weather.rain", 0.2f, 1.0f);
        }
    }
}

void EntityRenderer::renderRainSnow(float partialTicks)
{
    const float rainStrength = mc->theWorld->getRainStrength(partialTicks);
    if (rainStrength <= 0.0f)
        return;

    enableLightmap(static_cast<double>(partialTicks));

    if (!rainCoordsInitialized)
    {
        for (int_t z = 0; z < 32; ++z)
        {
            for (int_t x = 0; x < 32; ++x)
            {
                const float dx = static_cast<float>(x - 16);
                const float dz = static_cast<float>(z - 16);
                const float length = MathHelper::sqrt_float(dx * dx + dz * dz);
                const int_t index = z << 5 | x;
                rainXCoords[index] = -dz / length;
                rainYCoords[index] = dx / length;
            }
        }
        rainCoordsInitialized = true;
    }

    if (Config::isRainOff())
        return;

    EntityLiving* entity = mc->renderViewEntity;
    World* world = mc->theWorld;
    const int_t centerX = MathHelper::floor_double(entity->posX);
    const int_t centerY = MathHelper::floor_double(entity->posY);
    const int_t centerZ = MathHelper::floor_double(entity->posZ);
    Tessellator* tessellator = &Tessellator::instance;

    renderDisable(RenderCapability::CullFace);
    renderNormal3f(0.0f, 1.0f, 0.0f);
    renderEnable(RenderCapability::Blend);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
    renderAlphaFunc(RenderCompare::Greater, 0.01f);
    renderBindTexture(mc->renderEngine->getTexture("/environment/snow.png"));

#if PLATFORM_FLOAT_VERTEX_MATH
    const float renderPosX = static_cast<float>(entity->lastTickPosX) +
        (static_cast<float>(entity->posX) - static_cast<float>(entity->lastTickPosX)) * partialTicks;
    const float renderPosY = static_cast<float>(entity->lastTickPosY) +
        (static_cast<float>(entity->posY) - static_cast<float>(entity->lastTickPosY)) * partialTicks;
    const float renderPosZ = static_cast<float>(entity->lastTickPosZ) +
        (static_cast<float>(entity->posZ) - static_cast<float>(entity->lastTickPosZ)) * partialTicks;
#else
    const double renderPosX = entity->lastTickPosX + (entity->posX - entity->lastTickPosX) * static_cast<double>(partialTicks);
    const double renderPosY = entity->lastTickPosY + (entity->posY - entity->lastTickPosY) * static_cast<double>(partialTicks);
    const double renderPosZ = entity->lastTickPosZ + (entity->posZ - entity->lastTickPosZ) * static_cast<double>(partialTicks);
#endif
    const int_t interpolatedY = MathHelper::floor_double(renderPosY);
    const int_t range = Config::isRainFancy() ? 10 : 5;
    int_t activeWeatherTexture = -1;
    const float weatherTime = static_cast<float>(rendererUpdateCount) + partialTicks;

    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    for (int_t z = centerZ - range; z <= centerZ + range; ++z)
    {
        for (int_t x = centerX - range; x <= centerX + range; ++x)
        {
            const int_t coordIndex = (z - centerZ + 16) * 32 + x - centerX + 16;
            const float offsetX = rainXCoords[coordIndex] * 0.5f;
            const float offsetZ = rainYCoords[coordIndex] * 0.5f;
            BiomeGenBase* biome = world->getBiomeGenForCoords(x, z);
            if (biome == nullptr || (!biome->canSpawnLightningBolt() && !biome->getEnableSnow()))
                continue;

            const int_t precipitationY = world->getPrecipitationHeight(x, z);
            int_t minY = centerY - range;
            int_t maxY = centerY + range;
            if (minY < precipitationY)
                minY = precipitationY;
            if (maxY < precipitationY)
                maxY = precipitationY;

            int_t brightnessY = precipitationY;
            if (precipitationY < interpolatedY)
                brightnessY = interpolatedY;

            if (minY == maxY)
                continue;

            const int_t xSquared = JavaArithmetic::intMul(x, x);
            const int_t zSquared = JavaArithmetic::intMul(z, z);
            const int_t xSeed = JavaArithmetic::intAdd(JavaArithmetic::intMul(xSquared, 3121),
                                                       JavaArithmetic::intMul(x, 45238971));
            const int_t zSeed = JavaArithmetic::intAdd(JavaArithmetic::intMul(zSquared, 418711),
                                                       JavaArithmetic::intMul(z, 13761));
            random.setSeed(static_cast<long_t>(xSeed ^ zSeed));

            const float temperature = world->getWorldChunkManager()->getTemperatureAtHeight(
                biome->getFloatTemperature(), precipitationY);

            if (temperature >= 0.15f)
            {
                if (activeWeatherTexture != 0)
                {
                    if (activeWeatherTexture >= 0)
                        tessellator->draw();
                    activeWeatherTexture = 0;
                    renderBindTexture(mc->renderEngine->getTexture("/environment/rain.png"));
                    tessellator->startDrawingQuads();
                }

                const int_t animationSeed = JavaArithmetic::intAdd(
                    rendererUpdateCount,
                    JavaArithmetic::intAdd(xSeed, zSeed));
                const float textureOffset =
                    ((static_cast<float>(animationSeed & 31) + partialTicks) / 32.0f) *
                    (3.0f + random.nextFloat());
#if PLATFORM_FLOAT_VERTEX_MATH
                const float dx = static_cast<float>(
                    static_cast<double>(static_cast<float>(x) + 0.5f) - entity->posX);
                const float dz = static_cast<float>(
                    static_cast<double>(static_cast<float>(z) + 0.5f) - entity->posZ);
                const float distance = MathHelper::sqrt_float(dx * dx + dz * dz) / static_cast<float>(range);
#else
                const double dx = static_cast<double>(static_cast<float>(x) + 0.5f) - entity->posX;
                const double dz = static_cast<double>(static_cast<float>(z) + 0.5f) - entity->posZ;
                const float distance = MathHelper::sqrt_double(dx * dx + dz * dz) / static_cast<float>(range);
#endif
                const tess_coord_t minX = static_cast<tess_coord_t>(static_cast<float>(x) - offsetX) + static_cast<tess_coord_t>(0.5);
                const tess_coord_t maxX = static_cast<tess_coord_t>(static_cast<float>(x) + offsetX) + static_cast<tess_coord_t>(0.5);
                const tess_coord_t minZ = static_cast<tess_coord_t>(static_cast<float>(z) - offsetZ) + static_cast<tess_coord_t>(0.5);
                const tess_coord_t maxZ = static_cast<tess_coord_t>(static_cast<float>(z) + offsetZ) + static_cast<tess_coord_t>(0.5);
                const float minV = static_cast<float>(minY) / 4.0f + textureOffset;
                const float maxV = static_cast<float>(maxY) / 4.0f + textureOffset;

                tessellator->setBrightness(world->getLightBrightnessForSkyBlocks(x, brightnessY, z, 0));
                tessellator->setColorRGBA_F(1.0f, 1.0f, 1.0f,
                    ((1.0f - distance * distance) * 0.5f + 0.5f) * rainStrength);
                tessellator->setTranslationD(-renderPosX, -renderPosY, -renderPosZ);
                tessellator->addVertexWithUV(minX, minY, minZ, 0.0f, minV);
                tessellator->addVertexWithUV(maxX, minY, maxZ, 1.0f, minV);
                tessellator->addVertexWithUV(maxX, maxY, maxZ, 1.0f, maxV);
                tessellator->addVertexWithUV(minX, maxY, minZ, 0.0f, maxV);
                tessellator->setTranslationD(0.0, 0.0, 0.0);
            }
            else
            {
                if (activeWeatherTexture != 1)
                {
                    if (activeWeatherTexture >= 0)
                        tessellator->draw();
                    activeWeatherTexture = 1;
                    renderBindTexture(mc->renderEngine->getTexture("/environment/snow.png"));
                    tessellator->startDrawingQuads();
                }

                const float textureV = (static_cast<float>(rendererUpdateCount & 511) + partialTicks) / 512.0f;
                const float textureRandom = random.nextFloat();
                const float textureGaussian = static_cast<float>(random.nextGaussian());
                const float textureU = textureRandom + weatherTime * 0.01f * textureGaussian;
                const float jitterRandom = random.nextFloat();
                const float jitterGaussian = static_cast<float>(random.nextGaussian());
                const float textureJitter = jitterRandom + weatherTime * jitterGaussian * 0.001f;
#if PLATFORM_FLOAT_VERTEX_MATH
                const float dx = static_cast<float>(
                    static_cast<double>(static_cast<float>(x) + 0.5f) - entity->posX);
                const float dz = static_cast<float>(
                    static_cast<double>(static_cast<float>(z) + 0.5f) - entity->posZ);
                const float distance = MathHelper::sqrt_float(dx * dx + dz * dz) / static_cast<float>(range);
#else
                const double dx = static_cast<double>(static_cast<float>(x) + 0.5f) - entity->posX;
                const double dz = static_cast<double>(static_cast<float>(z) + 0.5f) - entity->posZ;
                const float distance = MathHelper::sqrt_double(dx * dx + dz * dz) / static_cast<float>(range);
#endif
                const tess_coord_t minX = static_cast<tess_coord_t>(static_cast<float>(x) - offsetX) + static_cast<tess_coord_t>(0.5);
                const tess_coord_t maxX = static_cast<tess_coord_t>(static_cast<float>(x) + offsetX) + static_cast<tess_coord_t>(0.5);
                const tess_coord_t minZ = static_cast<tess_coord_t>(static_cast<float>(z) - offsetZ) + static_cast<tess_coord_t>(0.5);
                const tess_coord_t maxZ = static_cast<tess_coord_t>(static_cast<float>(z) + offsetZ) + static_cast<tess_coord_t>(0.5);
                const float minV = static_cast<float>(minY) / 4.0f + textureV + textureJitter;
                const float maxV = static_cast<float>(maxY) / 4.0f + textureV + textureJitter;
                const int_t packedLight = world->getLightBrightnessForSkyBlocks(x, brightnessY, z, 0);

                tessellator->setBrightness((packedLight * 3 + 15728880) / 4);
                tessellator->setColorRGBA_F(1.0f, 1.0f, 1.0f,
                    ((1.0f - distance * distance) * 0.3f + 0.5f) * rainStrength);
                tessellator->setTranslationD(-renderPosX, -renderPosY, -renderPosZ);
                tessellator->addVertexWithUV(minX, minY, minZ, textureU, minV);
                tessellator->addVertexWithUV(maxX, minY, maxZ, 1.0f + textureU, minV);
                tessellator->addVertexWithUV(maxX, maxY, maxZ, 1.0f + textureU, maxV);
                tessellator->addVertexWithUV(minX, maxY, minZ, textureU, maxV);
                tessellator->setTranslationD(0.0, 0.0, 0.0);
            }
        }
    }

    if (activeWeatherTexture >= 0)
        tessellator->draw();

    renderEnable(RenderCapability::CullFace);
    renderDisable(RenderCapability::Blend);
    renderAlphaFunc(RenderCompare::Greater, 0.1f);
    disableLightmap(static_cast<double>(partialTicks));
}

void EntityRenderer::setupOverlayRendering()
{
    ScaledResolution scaledResolution(mc->gameSettings, mc->displayWidth, mc->displayHeight);
	renderViewport(0, viewportOffsetY, mc->displayWidth, mc->displayHeight);

    // Note for the Wii port: disabling GL_LIGHTING and GL_FOG here was tried, on
    // the theory that the native console lit pipeline (two colour channels, no normals in
    // GUI quads) was what turned the widgets white. It is not, and the control
    // flow rules it out on its own: GuiScreen::drawBackground disables both and
    // never restores them, so every GUI draw *after* the dirt background already
    // runs unlit -- and those are exactly the draws that fail. FontRenderer
    // disables lighting too, and its text is missing all the same.
    renderClear(RenderClearMask::Depth);  // 256
    renderMatrixMode(RenderMatrixMode::Projection);
    renderLoadIdentity();

    const float guiWidth = static_cast<float>(scaledResolution.field_25121_a);
    const float guiHeight = static_cast<float>(scaledResolution.field_25120_b);
    renderOrtho(0.0, guiWidth, guiHeight, 0.0, 1000.0, 3000.0);

    renderMatrixMode(RenderMatrixMode::ModelView);
    renderLoadIdentity();
    renderTranslate(0.0f, 0.0f, -2000.0f);
}

void EntityRenderer::updateFogColor(float partialTicks)
{
    World *world = mc->theWorld;
    EntityLiving *entityliving = mc->renderViewEntity;

    float fogDistanceFactor = 1.0f / (float)(4 - mc->gameSettings->renderDistance);
#if PLATFORM_FLOAT_VERTEX_MATH
    fogDistanceFactor = 1.0f - std::pow(fogDistanceFactor, 0.25f);
#else
    fogDistanceFactor = 1.0f - (float)pow(fogDistanceFactor, 0.25);
#endif

    Vec3D *skyColor = world->getSkyColor(mc->renderViewEntity, partialTicks);
    const float skyR = (float)skyColor->xCoord;
    const float skyG = (float)skyColor->yCoord;
    const float skyB = (float)skyColor->zCoord;

    Vec3D *fogColor = world->getFogColor(partialTicks);
    fogColorRed = (float)fogColor->xCoord;
    fogColorGreen = (float)fogColor->yCoord;
    fogColorBlue = (float)fogColor->zCoord;

    if (mc->gameSettings->renderDistance < 2)
    {
        Vec3D *sunDirection = MathHelper::sin(world->getCelestialAngleRadians(partialTicks)) > 0.0f
            ? Vec3D::createVector(-1.0, 0.0, 0.0)
            : Vec3D::createVector(1.0, 0.0, 0.0);
        float sunriseBlend = (float)entityliving->getLook(partialTicks)->dotProduct(sunDirection);
        if (sunriseBlend < 0.0f)
            sunriseBlend = 0.0f;
        if (sunriseBlend > 0.0f)
        {
            float *sunriseColors = world->worldProvider->calcSunriseSunsetColors(
                world->getCelestialAngle(partialTicks), partialTicks);
            if (sunriseColors != nullptr)
            {
                sunriseBlend *= sunriseColors[3];
                fogColorRed = fogColorRed * (1.0f - sunriseBlend) + sunriseColors[0] * sunriseBlend;
                fogColorGreen = fogColorGreen * (1.0f - sunriseBlend) + sunriseColors[1] * sunriseBlend;
                fogColorBlue = fogColorBlue * (1.0f - sunriseBlend) + sunriseColors[2] * sunriseBlend;
            }
        }
    }

    fogColorRed += (skyR - fogColorRed) * fogDistanceFactor;
    fogColorGreen += (skyG - fogColorGreen) * fogDistanceFactor;
    fogColorBlue += (skyB - fogColorBlue) * fogDistanceFactor;

    const float rainStrength = world->getRainStrength(partialTicks);
    if (rainStrength > 0.0f)
    {
        const float rainDarkness = 1.0f - rainStrength * 0.5f;
        const float rainBlueDarkness = 1.0f - rainStrength * 0.4f;
        fogColorRed *= rainDarkness;
        fogColorGreen *= rainDarkness;
        fogColorBlue *= rainBlueDarkness;
    }

    const float thunderStrength = world->getWeightedThunderStrength(partialTicks);
    if (thunderStrength > 0.0f)
    {
        const float thunderDarkness = 1.0f - thunderStrength * 0.5f;
        fogColorRed *= thunderDarkness;
        fogColorGreen *= thunderDarkness;
        fogColorBlue *= thunderDarkness;
    }

    const int_t viewpointBlockId = ActiveRenderInfo::getBlockIdAtEntityViewpoint(
        mc->theWorld, entityliving, partialTicks);
    Material *viewpointMaterial = nullptr;
    if (viewpointBlockId > 0 && viewpointBlockId < Block::BLOCK_REGISTRY_SIZE &&
        Block::blocksList[viewpointBlockId] != nullptr)
    {
        viewpointMaterial = Block::blocksList[viewpointBlockId]->blockMaterial;
    }

    if (cloudFog)
    {
        Vec3D *cloudColor = world->drawClouds(partialTicks);
        fogColorRed = (float)cloudColor->xCoord;
        fogColorGreen = (float)cloudColor->yCoord;
        fogColorBlue = (float)cloudColor->zCoord;
    }
    else if (viewpointMaterial == Material::water)
    {
        fogColorRed = 0.02f;
        fogColorGreen = 0.02f;
        fogColorBlue = 0.2f;
    }
    else if (viewpointMaterial == Material::lava)
    {
        fogColorRed = 0.6f;
        fogColorGreen = 0.1f;
        fogColorBlue = 0.0f;
    }

    const float fogBrightness = fogColor2 + (fogColor1 - fogColor2) * partialTicks;
    fogColorRed *= fogBrightness;
    fogColorGreen *= fogBrightness;
    fogColorBlue *= fogBrightness;

#if PLATFORM_FLOAT_VERTEX_MATH
    const float interpolatedFogY = static_cast<float>(entityliving->lastTickPosY) +
        (static_cast<float>(entityliving->posY) - static_cast<float>(entityliving->lastTickPosY)) * partialTicks;
    float voidFog = interpolatedFogY * static_cast<float>(world->worldProvider->getVoidFogYFactor());
#else
    const double interpolatedFogY = entityliving->lastTickPosY +
        (entityliving->posY - entityliving->lastTickPosY) * (double)partialTicks;
    double voidFog = interpolatedFogY * world->worldProvider->getVoidFogYFactor();
#endif
    if (entityliving->isPotionActive(Potion::blindness))
    {
        PotionEffect *effect = entityliving->getActivePotionEffect(Potion::blindness);
        const int_t blindnessDuration = effect != nullptr ? effect->getDuration() : 0;
        if (blindnessDuration < 20)
            voidFog *= 1.0f - (float)blindnessDuration / 20.0f;
        else
            voidFog = 0.0f;
    }

    if (voidFog < 1.0f)
    {
        if (voidFog < 0.0f)
            voidFog = 0.0f;
        voidFog *= voidFog;
        fogColorRed = static_cast<float>(fogColorRed * voidFog);
        fogColorGreen = static_cast<float>(fogColorGreen * voidFog);
        fogColorBlue = static_cast<float>(fogColorBlue * voidFog);
    }

#if PLATFORM_PS2
    if (mc->gameSettings != nullptr && mc->gameSettings->legacyLook)
        legacyLookRgb(fogColorRed, fogColorGreen, fogColorBlue);
#endif

    if (mc->gameSettings->anaglyph)
    {
        const float grayR = (fogColorRed * 30.0f + fogColorGreen * 59.0f + fogColorBlue * 11.0f) / 100.0f;
        const float grayG = (fogColorRed * 30.0f + fogColorGreen * 70.0f) / 100.0f;
        const float grayB = (fogColorRed * 30.0f + fogColorBlue * 70.0f) / 100.0f;
        fogColorRed = grayR;
        fogColorGreen = grayG;
        fogColorBlue = grayB;
    }

    renderClearColor(fogColorRed, fogColorGreen, fogColorBlue, 0.0f);
}

void EntityRenderer::setupFog(int fogMode, float partialTicks)
{
    EntityLiving *entityliving = mc->renderViewEntity;
    EntityPlayer *player = dynamic_cast<EntityPlayer *>(entityliving);
    const bool creativeMode = player != nullptr && player->capabilities.isCreativeMode;

    Potion::initPotions();

    if (fogMode == 999)
    {
        setupFogColorBuffer(0.0f, 0.0f, 0.0f, 1.0f);
        renderFogColor(fogColorBuffer);
        renderFogi(RenderFogParameter::Mode, RenderFogMode::Linear);
        renderFogf(RenderFogParameter::Start, 0.0f);
        renderFogf(RenderFogParameter::End, 8.0f);
        renderFogi(RenderFogParameter::DistanceMode, RenderFogMode::EyeRadial);
        renderTerrainSetFog(RenderFogMode::Linear, 1.0f, 0.0f, 8.0f,
                            0.0f, 0.0f, 0.0f, 1.0f);
        renderEnable(RenderCapability::ColorMaterial);
        renderColorMaterial(RenderFace::Front, RenderColorMaterialMode::Ambient);
        return;
    }

    setupFogColorBuffer(fogColorRed, fogColorGreen, fogColorBlue, 1.0f);
    renderFogColor(fogColorBuffer);
    renderNormal3f(0.0f, -1.0f, 0.0f);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    const int_t viewpointBlockId = ActiveRenderInfo::getBlockIdAtEntityViewpoint(mc->theWorld, entityliving, partialTicks);
    Material *viewpointMaterial = nullptr;
    if (viewpointBlockId > 0 && viewpointBlockId < Block::BLOCK_REGISTRY_SIZE && Block::blocksList[viewpointBlockId] != nullptr)
        viewpointMaterial = Block::blocksList[viewpointBlockId]->blockMaterial;

    if (entityliving->isPotionActive(Potion::blindness))
    {
        float fogDistance = 5.0f;
        PotionEffect *effect = entityliving->getActivePotionEffect(Potion::blindness);
        const int_t duration = effect != nullptr ? effect->getDuration() : 0;
        if (duration < 20)
            fogDistance = 5.0f + (farPlaneDistance - 5.0f) * (1.0f - (float)duration / 20.0f);

        float fogStart = fogMode < 0 ? 0.0f : fogDistance * 0.25f;
        float fogEnd = fogMode < 0 ? fogDistance * 0.8f : fogDistance;
        renderFogi(RenderFogParameter::Mode, RenderFogMode::Linear);
        renderFogf(RenderFogParameter::Start, fogStart);
        renderFogf(RenderFogParameter::End, fogEnd);
        renderFogi(RenderFogParameter::DistanceMode, RenderFogMode::EyeRadial);
        renderTerrainSetFog(RenderFogMode::Linear, 1.0f, fogStart, fogEnd,
                            fogColorRed, fogColorGreen, fogColorBlue, 1.0f);
    }
    else if (cloudFog)
    {
        renderFogi(RenderFogParameter::Mode, RenderFogMode::Exp);
        renderFogf(RenderFogParameter::Density, 0.1f);
        renderTerrainSetFog(RenderFogMode::Exp, 0.1f, 0.0f, farPlaneDistance,
                            fogColorRed, fogColorGreen, fogColorBlue, 1.0f);
    }
    else if (viewpointMaterial == Material::water)
    {
        float density = entityliving->isPotionActive(Potion::waterBreathing) ? 0.05f : 0.1f;
        if (Config::isClearWater())
            density /= 5.0f;
        renderFogi(RenderFogParameter::Mode, RenderFogMode::Exp);
        renderFogf(RenderFogParameter::Density, density);
        renderTerrainSetFog(RenderFogMode::Exp, density, 0.0f, farPlaneDistance,
                            fogColorRed, fogColorGreen, fogColorBlue, 1.0f);
    }
    else if (viewpointMaterial == Material::lava)
    {
        renderFogi(RenderFogParameter::Mode, RenderFogMode::Exp);
        renderFogf(RenderFogParameter::Density, 2.0f);
        renderTerrainSetFog(RenderFogMode::Exp, 2.0f, 0.0f, farPlaneDistance,
                            fogColorRed, fogColorGreen, fogColorBlue, 1.0f);
    }
    else
    {
        float effectiveFar = farPlaneDistance;
        if (Config::isDepthFog() && mc->theWorld->worldProvider->getWorldHasNoSky() && !creativeMode)
        {
            const int_t packedBrightness = entityliving->getBrightnessForRender(partialTicks);
#if PLATFORM_FLOAT_VERTEX_MATH
            const float interpolatedFogY = static_cast<float>(entityliving->lastTickPosY) +
                (static_cast<float>(entityliving->posY) - static_cast<float>(entityliving->lastTickPosY)) * partialTicks;
            float fogFactor = static_cast<float>((packedBrightness & 15728640) >> 20) / 16.0f;
            fogFactor += (interpolatedFogY + 4.0f) / 32.0f;
#else
            const double interpolatedFogY = entityliving->lastTickPosY +
                (entityliving->posY - entityliving->lastTickPosY) * (double)partialTicks;
            double fogFactor = (double)((packedBrightness & 15728640) >> 20) / 16.0
                             + (interpolatedFogY + 4.0) / 32.0;
#endif
            if (fogFactor < 1.0f)
            {
                if (fogFactor < 0.0f)
                    fogFactor = 0.0f;
                fogFactor *= fogFactor;
                float heightFog = 100.0f * static_cast<float>(fogFactor);
                if (heightFog < 5.0f)
                    heightFog = 5.0f;
                if (effectiveFar > heightFog)
                    effectiveFar = heightFog;
            }
        }

        renderFogi(RenderFogParameter::Mode, RenderFogMode::Linear);
        if (Config::isFogFancy())
        {
            renderFogHint(RenderHintMode::Nicest);
            renderFogi(RenderFogParameter::DistanceMode, RenderFogMode::EyeRadial);
        }
        else
        {
            renderFogHint(RenderHintMode::Fastest);
        }

        float fogStart = fogMode < 0 ? 0.0f : effectiveFar * Config::getFogStart();
        float fogEnd = fogMode < 0 ? effectiveFar * 0.8f : effectiveFar;

        if (mc->theWorld->worldProvider->func_48218_b((int_t)entityliving->posX, (int_t)entityliving->posZ))
        {
            fogStart = effectiveFar * 0.05f;
            fogEnd = std::min(effectiveFar, 192.0f) * 0.5f;
        }

        if (Config::isFogOff() && fogMode >= 0 && !mc->theWorld->worldProvider->isNether)
        {
            fogStart = effectiveFar * 4.0f;
            fogEnd = effectiveFar * 8.0f;
        }

#if PLATFORM_CHUNK_EDGE_FOG
        if (fogMode >= 0 && !mc->theWorld->worldProvider->isNether && !mc->theWorld->worldProvider->func_48218_b((int_t)entityliving->posX, (int_t)entityliving->posZ))
        {
            const float loadedEdge = (float)(PLATFORM_VISIBLE_CHUNK_RADIUS * 16);
            const float chunkFogEnd = loadedEdge > 8.0f ? loadedEdge - 8.0f : loadedEdge;
            const float effectiveFogEnd = effectiveFar < chunkFogEnd ? effectiveFar : chunkFogEnd;
            fogEnd = effectiveFogEnd;
            fogStart = effectiveFogEnd * Config::getFogStart();
        }
#elif PLATFORM_CONSOLE_LOW
        if (fogMode >= 0 && !mc->theWorld->worldProvider->isNether && !mc->theWorld->worldProvider->func_48218_b((int_t)entityliving->posX, (int_t)entityliving->posZ))
        {
            const float loadedEdge = (float)(PLATFORM_VISIBLE_CHUNK_RADIUS * 16);
            fogEnd = loadedEdge < effectiveFar ? loadedEdge : effectiveFar;
            fogStart = fogEnd * 0.55f;
        }
#endif

        renderFogf(RenderFogParameter::Start, fogStart);
        renderFogf(RenderFogParameter::End, fogEnd);
        renderTerrainSetFog(RenderFogMode::Linear, 1.0f, fogStart, fogEnd,
                            fogColorRed, fogColorGreen, fogColorBlue, 1.0f);
    }

    renderEnable(RenderCapability::ColorMaterial);
    renderColorMaterial(RenderFace::Front, RenderColorMaterialMode::Ambient);
}

void EntityRenderer::setupFogColorBuffer(float r, float g, float b, float a)
{
    fogColorBuffer[0] = r;
    fogColorBuffer[1] = g;
    fogColorBuffer[2] = b;
    fogColorBuffer[3] = a;
}
