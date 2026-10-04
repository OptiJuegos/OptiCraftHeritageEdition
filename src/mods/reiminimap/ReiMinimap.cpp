#include "ReiMinimap.h"
#include "GuiWaypointManager.h"
#include "Minecraft.h"
#include "GuiIngame.h"
#include "FontRenderer.h"
#include "RenderEngine.h"
#include "EntityPlayerSP.h"
#include "World.h"
#include "WorldInfo.h"
#include "WorldProvider.h"
#include "WorldHeight.h"
#include "Chunk.h"
#include "Block.h"
#include "Material.h"
#include "MapColor.h"
#include "Tessellator.h"
#include "platform/RenderAPI.h"
#include "platform/Storage.h"
#include "platform/Log.h"
#include "java/File.h"
#include "java/BufferedImage.h"
#include "SoundManager.h"
#include "ISaveHandler.h"

#include <cmath>
#include <algorithm>
#include <cstdio>
#include <cstdlib>

#if PLATFORM_PS2
#include "ps2/input/Ps2PadState.h"
#endif

static const int_t MAP_RES = 64;

static const int_t WAYPOINT_PALETTE[] = {
    0x00FF88, // Mint / Emerald (Default Spawn)
    0xFF2222, // Vibrant Red
    0x2288FF, // Bright Blue
    0xFFFF00, // Vibrant Yellow
    0xFF22FF, // Magenta / Pink
    0x00FFFF, // Cyan / Aqua
    0xFF8800, // Orange
    0xAA00FF, // Deep Purple
    0x77FF00, // Lime Green
    0xFF4488, // Coral Rose
    0xFFFFFF, // Pure White
    0x00A8FF  // Sky Blue
};
static const size_t NUM_PALETTE_COLORS = sizeof(WAYPOINT_PALETTE) / sizeof(WAYPOINT_PALETTE[0]);

namespace
{

// First block with a visible map color at or below scanTop in this chunk
// column (the same predicate the minimap has always used). Returns the
// height above it -- blockY + 1, the exact convention the old inline scan
// ended with -- and copies the block id out. A column with no visible
// block bottoms out at y == 1 with whatever block sits there.
int_t minimapColumnSurface(Chunk *chunk, int_t lx, int_t lz, int_t scanTop, int_t *blockIdOut)
{
    int_t y = scanTop;
    int_t blockId = 0;
    while (y > 1)
    {
        blockId = chunk->getBlockID(lx, y - 1, lz);
        if (blockId != 0 && Block::blocksList[blockId] != nullptr)
        {
            Material *mat = Block::blocksList[blockId]->blockMaterial;
            if (mat != nullptr && mat->materialMapColor != nullptr && mat->materialMapColor != MapColor::airColor)
                break;
        }
        y--;
    }
    if (blockIdOut != nullptr)
        *blockIdOut = blockId;
    return y;
}

} // namespace

static void drawColoredRect(int_t x1, int_t y1, int_t x2, int_t y2, int_t color)
{
    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);
    float_t a = static_cast<float_t>((color >> 24) & 255) / 255.0f;
    float_t r = static_cast<float_t>((color >> 16) & 255) / 255.0f;
    float_t g = static_cast<float_t>((color >> 8) & 255) / 255.0f;
    float_t b = static_cast<float_t>(color & 255) / 255.0f;
    renderEnable(RenderCapability::Blend);
    renderDisable(RenderCapability::Texture2D);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
    renderColor4f(r, g, b, a);
    Tessellator *tess = &Tessellator::instance;
    tess->startDrawingQuads();
    tess->setColorRGBA_F(r, g, b, a);
    tess->addVertex(x1, y2, 0.0);
    tess->addVertex(x2, y2, 0.0);
    tess->addVertex(x2, y1, 0.0);
    tess->addVertex(x1, y1, 0.0);
    tess->draw();
    renderEnable(RenderCapability::Texture2D);
}

ReiMinimap &ReiMinimap::getInstance()
{
    static ReiMinimap instance;
    return instance;
}

ReiMinimap::ReiMinimap()
    : m_mc(nullptr)
    , m_enabled(false)
    , m_updateTicks(0)
    , m_initialized(false)
{
    m_mapTextureId[0] = -1;
    m_mapTextureId[1] = -1;
    m_lastPlayerX[0] = -999999;
    m_lastPlayerZ[0] = -999999;
    m_lastPlayerX[1] = -999999;
    m_lastPlayerZ[1] = -999999;
    m_waypointComboWasPressed[0] = false;
    m_waypointComboWasPressed[1] = false;
    m_waypointMenuComboWasPressed[0] = false;
    m_waypointMenuComboWasPressed[1] = false;
    m_toastTimer[0] = 0;
    m_toastTimer[1] = 0;
}

ReiMinimap::~ReiMinimap()
{
    if (m_mc != nullptr && m_mc->renderEngine != nullptr)
    {
        for (int i = 0; i < 2; ++i)
        {
            if (m_mapTextureId[i] >= 0)
            {
                m_mc->renderEngine->deleteTexture(m_mapTextureId[i]);
                m_mapTextureId[i] = -1;
            }
        }
    }
}

void ReiMinimap::init(Minecraft *mc)
{
    if (m_initialized)
        return;

    m_mc = mc;
    m_mapImage = std::make_unique<BufferedImage>(MAP_RES, MAP_RES);
    m_pixelData.assign(MAP_RES * MAP_RES * 4, 0);

    if (m_mc != nullptr && m_mc->renderEngine != nullptr)
    {
        m_mapTextureId[0] = m_mc->renderEngine->allocateAndSetupTexture(m_mapImage.get());
        m_mapTextureId[1] = m_mc->renderEngine->allocateAndSetupTexture(m_mapImage.get());
    }

    loadWaypoints();
    m_initialized = true;
    MC_LOG_INFO("mods", "Rei's Minimap initialized with texture IDs %d, %d\n", m_mapTextureId[0], m_mapTextureId[1]);
}

void ReiMinimap::update()
{
    if (!m_enabled || m_mc == nullptr || m_mc->theWorld == nullptr)
        return;

    // Detect world switch / entering a world
    std::string curDir;
    if (m_mc->theWorld->getSaveHandler() != nullptr)
        curDir = m_mc->theWorld->getSaveHandler()->getSaveDirectory();
    if (curDir != m_currentWorldDir)
    {
        m_currentWorldDir = curDir;
        loadWaypoints();
        m_lastPlayerX[0] = -999999;
        m_lastPlayerZ[0] = -999999;
        m_lastPlayerX[1] = -999999;
        m_lastPlayerZ[1] = -999999;
    }

    m_updateTicks++;
    for (int i = 0; i < 2; ++i)
    {
        if (m_toastTimer[i] > 0)
            m_toastTimer[i]--;
    }

    EntityPlayerSP *p1 = m_mc->thePlayerOne ? m_mc->thePlayerOne : m_mc->thePlayer;
    EntityPlayerSP *p2 = m_mc->thePlayer2;
    const bool isSplit = (m_mc->isSplitScreenActive() && p2 != nullptr);

#if PLATFORM_PS2
    // Check Pad 0 (Player 1) combo (Triangle + D-Pad Up)
    if (p1 != nullptr)
    {
        const Ps2PadSnapshot &pad0 = ps2PadGetSnapshot(0);
        bool combo0 = (pad0.held & PS2_PAD_TRIANGLE) != 0 && (pad0.held & PS2_PAD_UP) != 0;
        if (combo0 && !m_waypointComboWasPressed[0])
        {
            int_t px = static_cast<int_t>(std::floor(p1->posX));
            int_t py = static_cast<int_t>(std::floor(p1->posY));
            int_t pz = static_cast<int_t>(std::floor(p1->posZ));
            char nameBuf[32];
            std::snprintf(nameBuf, sizeof(nameBuf), isSplit ? "P1 Waypoint %u" : "Waypoint %u", static_cast<unsigned>(m_waypoints.size() + 1));
            addWaypoint(nameBuf, px, py, pz);

            m_toastMessage[0] = "Waypoint saved!";
            m_toastTimer[0] = 60;
            if (m_mc->sndManager != nullptr)
                m_mc->sndManager->playSoundFX("random.orb", 1.0f, 1.0f);
        }
        m_waypointComboWasPressed[0] = combo0;

        // Check Pad 0 (Player 1) combo (Triangle + D-Pad Down) to open Waypoint Manager
        bool menuCombo0 = (pad0.held & PS2_PAD_TRIANGLE) != 0 && (pad0.held & PS2_PAD_DOWN) != 0;
        if (menuCombo0 && !m_waypointMenuComboWasPressed[0] && m_mc->currentScreen == nullptr)
        {
            if (isSplit)
            {
                if (m_mc->isPlayerScreenActive(0))
                    m_mc->closePlayerScreen(0);
                else
                    m_mc->displayPlayerScreen(0, new GuiWaypointManager(0));
            }
            else
            {
                ps2SetMenuPad(0);
                ps2SetMenuOwnerPad(0);
                m_mc->displayGuiScreen(new GuiWaypointManager(0));
            }
        }
        m_waypointMenuComboWasPressed[0] = menuCombo0;
    }

    // Check Pad 1 (Player 2) combo (Triangle + D-Pad Up) if split screen is active
    if (isSplit && p2 != nullptr)
    {
        const Ps2PadSnapshot &pad1 = ps2PadGetSnapshot(1);
        bool combo1 = (pad1.held & PS2_PAD_TRIANGLE) != 0 && (pad1.held & PS2_PAD_UP) != 0;
        if (combo1 && !m_waypointComboWasPressed[1])
        {
            int_t px = static_cast<int_t>(std::floor(p2->posX));
            int_t py = static_cast<int_t>(std::floor(p2->posY));
            int_t pz = static_cast<int_t>(std::floor(p2->posZ));
            char nameBuf[32];
            std::snprintf(nameBuf, sizeof(nameBuf), "P2 Waypoint %u", static_cast<unsigned>(m_waypoints.size() + 1));
            addWaypoint(nameBuf, px, py, pz);

            m_toastMessage[1] = "Waypoint saved!";
            m_toastTimer[1] = 60;
            if (m_mc->sndManager != nullptr)
                m_mc->sndManager->playSoundFX("random.orb", 1.0f, 1.0f);
        }
        m_waypointComboWasPressed[1] = combo1;

        // Check Pad 1 (Player 2) combo (Triangle + D-Pad Down) to open Waypoint Manager
        bool menuCombo1 = (pad1.held & PS2_PAD_TRIANGLE) != 0 && (pad1.held & PS2_PAD_DOWN) != 0;
        if (menuCombo1 && !m_waypointMenuComboWasPressed[1] && m_mc->currentScreen == nullptr)
        {
            if (m_mc->isPlayerScreenActive(1))
                m_mc->closePlayerScreen(1);
            else
                m_mc->displayPlayerScreen(1, new GuiWaypointManager(1));
        }
        m_waypointMenuComboWasPressed[1] = menuCombo1;
    }
#endif

    // Update Player 1 texture
    if (p1 != nullptr)
    {
        int_t px = static_cast<int_t>(std::floor(p1->posX));
        int_t pz = static_cast<int_t>(std::floor(p1->posZ));
        if (px != m_lastPlayerX[0] || pz != m_lastPlayerZ[0] || m_updateTicks >= 10)
        {
            m_lastPlayerX[0] = px;
            m_lastPlayerZ[0] = pz;
            updateMapTexture(0, p1);
        }
    }

    // Update Player 2 texture if split-screen is active
    if (isSplit && p2 != nullptr)
    {
        int_t px2 = static_cast<int_t>(std::floor(p2->posX));
        int_t pz2 = static_cast<int_t>(std::floor(p2->posZ));
        if (px2 != m_lastPlayerX[1] || pz2 != m_lastPlayerZ[1] || m_updateTicks >= 10)
        {
            m_lastPlayerX[1] = px2;
            m_lastPlayerZ[1] = pz2;
            updateMapTexture(1, p2);
        }
    }

    if (m_updateTicks >= 10)
        m_updateTicks = 0;
}

int_t ReiMinimap::getBlockColor(int_t blockId, int_t height, int_t northHeight)
{
    if (blockId <= 0)
        return 0x00000000;

    int_t r = 120, g = 120, b = 120;

    switch (blockId)
    {
    case 8: case 9: // Water
        r = 46; g = 100; b = 254; break;
    case 10: case 11: // Lava
        r = 255; g = 85; b = 0; break;
    case 2: // Grass
        r = 85; g = 160; b = 48; break;
    case 3: case 60: // Dirt / Farmland
        r = 134; g = 96; b = 67; break;
    case 1: case 4: // Stone / Cobblestone
        r = 128; g = 128; b = 128; break;
    case 12: case 24: // Sand / Sandstone
        r = 216; g = 200; b = 136; break;
    case 13: // Gravel
        r = 136; g = 130; b = 130; break;
    case 17: case 5: // Wood / Planks
        r = 138; g = 108; b = 69; break;
    case 18: // Leaves
        r = 42; g = 112; b = 24; break;
    case 20: // Glass
        r = 192; g = 224; b = 240; break;
    case 78: case 80: // Snow
        r = 240; g = 240; b = 240; break;
    case 79: // Ice
        r = 160; g = 200; b = 240; break;
    case 81: // Cactus
        r = 24; g = 112; b = 24; break;
    case 82: // Clay
        r = 158; g = 159; b = 169; break;
    case 87: // Netherrack
        r = 112; g = 32; b = 32; break;
    case 88: // Soul sand
        r = 84; g = 64; b = 51; break;
    case 89: // Glowstone
        r = 228; g = 216; b = 140; break;
    case 49: // Obsidian
        r = 21; g = 18; b = 31; break;
    default:
        if (blockId < 256 && Block::blocksList[blockId] != nullptr)
        {
            Block *blk = Block::blocksList[blockId];
            if (blk->blockMaterial != nullptr && blk->blockMaterial->materialMapColor != nullptr)
            {
                int_t c = blk->blockMaterial->materialMapColor->colorValue;
                if (c != 0)
                {
                    r = (c >> 16) & 0xFF;
                    g = (c >> 8) & 0xFF;
                    b = c & 0xFF;
                }
            }
        }
        break;
    }

    // Hill / slope shading
    if (height > northHeight)
    {
        r = std::min(255, r * 118 / 100);
        g = std::min(255, g * 118 / 100);
        b = std::min(255, b * 118 / 100);
    }
    else if (height < northHeight)
    {
        r = r * 82 / 100;
        g = g * 82 / 100;
        b = b * 82 / 100;
    }

    return (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16) | 0xFF000000;
}

void ReiMinimap::updateMapTexture(int_t playerIndex, EntityPlayer *player)
{
    if (m_mc == nullptr || player == nullptr || m_mc->theWorld == nullptr || !m_mapImage)
        return;
    if (playerIndex < 0 || playerIndex >= 2 || m_mapTextureId[playerIndex] < 0)
        return;

    int_t playerX = static_cast<int_t>(std::floor(player->posX));
    int_t playerZ = static_cast<int_t>(std::floor(player->posZ));
    World *world = m_mc->theWorld;

    // Hell worlds (the Nether) are capped by a bedrock ceiling, so the chunk
    // heightmap tops out at the ceiling and a scan from the top samples
    // bedrock for every pixel -- the map rendered flat gray. There the scan
    // starts at the player's feet instead (cave-style), mapping the terrain
    // the player walks on; the shading neighbour is scanned the same way.
    const bool isHellWorld = world->worldProvider != nullptr && world->worldProvider->isHellWorld;
    // The blocks at the player's feet and head are passable by definition, so
    // starting at feetY + 1 never reads the ceiling right above the player.
    // Clamped to the world height so a bogus server teleport can't turn the
    // scan into a near-unbounded loop (the overworld start is the heightmap,
    // which is already bounded).
    const int_t hellScanTop = std::min(static_cast<int_t>(std::floor(player->posY)) + 1, WorldHeight::HEIGHT);

    Chunk *cachedChunk = nullptr;
    int_t cachedChunkX = 0x7FFFFFFF;
    int_t cachedChunkZ = 0x7FFFFFFF;

    // Hell-mode shading state: the north neighbour of row dy is the column
    // row dy-1 just scanned (same scan top), so each row's surface height is
    // kept for the next row instead of scanning every column twice. Only the
    // first row needs fresh north lookups. Zero means "no visible block or
    // missing chunk", the same convention as the overworld's heightValue
    // fallback.
    int_t prevRowY[MAP_RES];
    for (int_t i = 0; i < MAP_RES; ++i)
        prevRowY[i] = 0;

    for (int_t dy = 0; dy < MAP_RES; ++dy)
    {
        int_t wz = playerZ + (dy - MAP_RES / 2);
        int_t chunkZ = wz >> 4;

        for (int_t dx = 0; dx < MAP_RES; ++dx)
        {
            int_t wx = playerX + (dx - MAP_RES / 2);
            int_t chunkX = wx >> 4;

            if (cachedChunk == nullptr || chunkX != cachedChunkX || chunkZ != cachedChunkZ)
            {
                cachedChunk = world->getChunkFromBlockCoords(wx, wz);
                cachedChunkX = chunkX;
                cachedChunkZ = chunkZ;
            }

            Chunk *chunk = cachedChunk;
            int_t r = 24, g = 24, b = 24, a = 255;

            if (chunk != nullptr && !chunk->isEmpty())
            {
                int_t lx = wx & 15;
                int_t lz = wz & 15;
                const int_t scanTop = isHellWorld
                    ? hellScanTop
                    : chunk->getHeightValue(lx, lz) + 1;

                int_t blockId = 0;
                int_t y = minimapColumnSurface(chunk, lx, lz, scanTop, &blockId);

                int_t northHeight;
                if (!isHellWorld)
                {
                    northHeight = (lz > 0) ? chunk->getHeightValue(lx, lz - 1) : world->getHeightValue(wx, wz - 1);
                }
                else if (dy > 0)
                {
                    northHeight = prevRowY[dx];
                }
                else if (lz > 0)
                {
                    northHeight = minimapColumnSurface(chunk, lx, lz - 1, hellScanTop, nullptr);
                }
                else
                {
                    // First row, north neighbour across the chunk border:
                    // same guard the pixel loop itself uses.
                    Chunk *northChunk = world->getChunkFromBlockCoords(wx, wz - 1);
                    northHeight = (northChunk != nullptr && !northChunk->isEmpty())
                        ? minimapColumnSurface(northChunk, lx, 15, hellScanTop, nullptr)
                        : 0;
                }

                if (isHellWorld)
                    prevRowY[dx] = y;

                int_t col = getBlockColor(blockId, y, northHeight);

                r = col & 0xFF;
                g = (col >> 8) & 0xFF;
                b = (col >> 16) & 0xFF;
                a = 255;
            }
            else if (isHellWorld)
            {
                prevRowY[dx] = 0;
            }

            // El sampler 3DS lee V=0 en la ULTIMA fila almacenada (convencion
            // MC-3DS, ver DsShader.v.pica / DsRender), mientras que GL lee V=0
            // en la primera. Guardamos la fila del norte (dy=0) al reves para
            // que termine en V=0, arriba en pantalla -- sin esto el mapa se
            // dibujaba con el sur arriba y quedaba invertido respecto de las
            // etiquetas N/S y de los waypoints, que si van en coordenadas de
            // pantalla correctas.
            size_t row = static_cast<size_t>(dy);
#if PLATFORM_3DS
            row = static_cast<size_t>(MAP_RES - 1 - dy);
#endif
            size_t idx = (row * static_cast<size_t>(MAP_RES) + static_cast<size_t>(dx)) * 4;
            m_pixelData[idx + 0] = static_cast<unsigned char>(r);
            m_pixelData[idx + 1] = static_cast<unsigned char>(g);
            m_pixelData[idx + 2] = static_cast<unsigned char>(b);
            m_pixelData[idx + 3] = static_cast<unsigned char>(a);
        }
    }

    m_mapImage->setRGB(0, 0, MAP_RES, MAP_RES, m_pixelData.data());

    if (m_mapTextureId[playerIndex] >= 0 && m_mc->renderEngine != nullptr)
    {
        m_mc->renderEngine->setupTexture(m_mapImage.get(), m_mapTextureId[playerIndex]);
    }
}

void ReiMinimap::render(GuiIngame *, int_t screenWidth, int_t, float_t)
{
    if (!m_enabled || m_mc == nullptr || m_mc->thePlayer == nullptr)
        return;

    const bool isSplit = (m_mc->isSplitScreenActive() && m_mc->thePlayer2 != nullptr);
    int_t playerIndex = 0;
    EntityPlayer *curPlayer = m_mc->thePlayer;
    if (isSplit && m_mc->thePlayer == m_mc->thePlayer2)
    {
        playerIndex = 1;
        curPlayer = m_mc->thePlayer2;
    }
    else if (m_mc->thePlayerOne != nullptr)
    {
        curPlayer = m_mc->thePlayerOne;
    }
    if (curPlayer == nullptr)
        curPlayer = m_mc->thePlayer;

    if (m_mapTextureId[playerIndex] < 0)
        return;

    renderDisable(RenderCapability::DepthTest);

    // Initial update if needed
    if (m_lastPlayerX[playerIndex] == -999999)
    {
        m_lastPlayerX[playerIndex] = static_cast<int_t>(std::floor(curPlayer->posX));
        m_lastPlayerZ[playerIndex] = static_cast<int_t>(std::floor(curPlayer->posZ));
        updateMapTexture(playerIndex, curPlayer);
    }

    // Adaptive map sizing: 64px for single player, 48px for split-screen half-viewports
    const int_t mapSize = isSplit ? 48 : 64;
    const int_t posX = isSplit ? (screenWidth - mapSize - 4) : (screenWidth - mapSize - 6);
    const int_t posY = isSplit ? 4 : 6;

    // Background panel & border
    drawColoredRect(posX - 1, posY - 1, posX + mapSize + 1, posY + mapSize + 1, 0xA0000000);
    drawColoredRect(posX - 2, posY - 2, posX + mapSize + 2, posY - 1, 0xFF555555);
    drawColoredRect(posX - 2, posY + mapSize + 1, posX + mapSize + 2, posY + mapSize + 2, 0xFF555555);
    drawColoredRect(posX - 2, posY - 2, posX - 1, posY + mapSize + 2, 0xFF555555);
    drawColoredRect(posX + mapSize + 1, posY - 2, posX + mapSize + 2, posY + mapSize + 2, 0xFF555555);

    // Draw the dynamic map texture for this player
    renderEnable(RenderCapability::Texture2D);
    renderEnable(RenderCapability::Blend);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    renderBindTexture(m_mapTextureId[playerIndex]);

    Tessellator *tess = &Tessellator::instance;
    tess->startDrawingQuads();
    tess->setColorOpaque_I(0xFFFFFF);
    tess->addVertexWithUV(posX,           posY + mapSize, 0.0, 0.0, 1.0);
    tess->addVertexWithUV(posX + mapSize, posY + mapSize, 0.0, 1.0, 1.0);
    tess->addVertexWithUV(posX + mapSize, posY,           0.0, 1.0, 0.0);
    tess->addVertexWithUV(posX,           posY,           0.0, 0.0, 0.0);
    tess->draw();

    // Cardinal direction markers (N, S, W, E)
    FontRenderer *fr = m_mc->fontRenderer;
    if (fr != nullptr)
    {
        fr->drawStringWithShadow("N", posX + mapSize / 2 - 2, posY + 2, 0xFF5555); // Red N
        fr->drawStringWithShadow("S", posX + mapSize / 2 - 2, posY + mapSize - 9, 0xAAAAAA);
        fr->drawStringWithShadow("W", posX + 2, posY + mapSize / 2 - 4, 0xAAAAAA);
        fr->drawStringWithShadow("E", posX + mapSize - 7, posY + mapSize / 2 - 4, 0xAAAAAA);
    }

    // Center and scaling
    const float_t cx = posX + mapSize / 2.0f;
    const float_t cy = posY + mapSize / 2.0f;
    int_t px = static_cast<int_t>(std::floor(curPlayer->posX));
    int_t pz = static_cast<int_t>(std::floor(curPlayer->posZ));

    const float_t visibleBlockRadius = MAP_RES / 2.0f; // 32 blocks
    const float_t mapPixelRadius = mapSize / 2.0f;
    const float_t scale = mapPixelRadius / visibleBlockRadius;
    const float_t maxBlockDist = visibleBlockRadius - 3.0f;
    const float_t borderR = mapPixelRadius - 3.5f;

    // Render Waypoints
    for (const auto &wp : m_waypoints)
    {
        if (!wp.enabled) continue;
        float_t dx = static_cast<float_t>(wp.x - px);
        float_t dz = static_cast<float_t>(wp.z - pz);

        if (std::abs(dx) <= maxBlockDist && std::abs(dz) <= maxBlockDist)
        {
            // Waypoint is inside minimap bounds: draw standard dot
            int_t wxScreen = static_cast<int_t>(cx + dx * scale);
            int_t wzScreen = static_cast<int_t>(cy + dz * scale);
            drawColoredRect(wxScreen - 2, wzScreen - 2, wxScreen + 2, wzScreen + 2, 0xFF000000);
            drawColoredRect(wxScreen - 1, wzScreen - 1, wxScreen + 1, wzScreen + 1, 0xFF000000 | wp.color);
        }
        else
        {
            // Waypoint is OUTSIDE the minimap: stay on border with directional arrow pointing towards it!
            float_t maxAbs = std::max(std::abs(dx), std::abs(dz));
            float_t bx = cx + (dx / maxAbs) * borderR;
            float_t by = cy + (dz / maxAbs) * borderR;

            float_t angleRad = std::atan2(dz, dx);
            float_t angleDeg = angleRad * (180.0f / 3.14159265f);

            // Draw directional pointer on the border
            renderPushMatrix();
            renderTranslate(bx, by, 0.0f);
            renderRotate(angleDeg + 90.0f, 0.0f, 0.0f, 1.0f);
            renderDisable(RenderCapability::Texture2D);

            // Outer dark outline arrow
            tess->startDrawingQuads();
            tess->setColorOpaque_I(0x000000);
            tess->addVertex(-3.5,  2.5, 0.0);
            tess->addVertex( 3.5,  2.5, 0.0);
            tess->addVertex( 0.0, -4.5, 0.0);
            tess->addVertex( 0.0, -4.5, 0.0);
            tess->draw();

            // Inner colored arrow
            tess->startDrawingQuads();
            tess->setColorOpaque_I(0xFF000000 | wp.color);
            tess->addVertex(-2.5,  1.5, 0.0);
            tess->addVertex( 2.5,  1.5, 0.0);
            tess->addVertex( 0.0, -3.5, 0.0);
            tess->addVertex( 0.0, -3.5, 0.0);
            tess->draw();

            renderEnable(RenderCapability::Texture2D);
            renderPopMatrix();
        }
    }

    // Player arrow in center
    renderPushMatrix();
    renderTranslate(cx, cy, 0.0f);
    renderRotate(curPlayer->rotationYaw + 180.0f, 0.0f, 0.0f, 1.0f);
    renderDisable(RenderCapability::Texture2D);

    tess->startDrawingQuads();
    tess->setColorOpaque_I(0xFF2222); // Red arrow tip
    tess->addVertex(-2.0,  3.0, 0.0);
    tess->addVertex( 2.0,  3.0, 0.0);
    tess->addVertex( 0.0, -4.0, 0.0);
    tess->addVertex( 0.0, -4.0, 0.0);
    tess->draw();

    renderEnable(RenderCapability::Texture2D);
    renderPopMatrix();

    // Coordinates display below minimap
    if (fr != nullptr)
    {
        int_t py = static_cast<int_t>(std::floor(curPlayer->posY));
        char coordBuf[48];
        std::snprintf(coordBuf, sizeof(coordBuf), "X:%d Y:%d Z:%d", px, py, pz);
        int_t strW = fr->getStringWidth(coordBuf);
        fr->drawStringWithShadow(coordBuf, posX + (mapSize - strW) / 2, posY + mapSize + (isSplit ? 2 : 4), 0xFFFFFF);

        // On-screen notification toast (e.g. when adding a waypoint)
        if (m_toastTimer[playerIndex] > 0 && !m_toastMessage[playerIndex].empty())
        {
            int_t toastW = fr->getStringWidth(m_toastMessage[playerIndex]);
            int_t toastX = posX + (mapSize - toastW) / 2;
            int_t toastY = posY + mapSize + (isSplit ? 12 : 16);
            drawColoredRect(toastX - 3, toastY - 2, toastX + toastW + 3, toastY + 10, 0xC0000000);
            fr->drawStringWithShadow(m_toastMessage[playerIndex], toastX, toastY, 0x55FF55);
        }
    }

    renderEnable(RenderCapability::DepthTest);
}

int_t ReiMinimap::getNextWaypointColor() const
{
    return WAYPOINT_PALETTE[m_waypoints.size() % NUM_PALETTE_COLORS];
}

void ReiMinimap::addWaypoint(const std::string &name, int_t x, int_t y, int_t z, int_t color)
{
    if (color < 0)
        color = getNextWaypointColor();
    m_waypoints.push_back({ name, x, y, z, color, true });
    saveWaypoints();
}

void ReiMinimap::toggleWaypoint(size_t index)
{
    if (index < m_waypoints.size())
    {
        m_waypoints[index].enabled = !m_waypoints[index].enabled;
        saveWaypoints();
    }
}

void ReiMinimap::removeWaypoint(size_t index)
{
    if (index < m_waypoints.size())
    {
        m_waypoints.erase(m_waypoints.begin() + index);
        saveWaypoints();
    }
}

std::string ReiMinimap::getWaypointsFilePath() const
{
    if (m_mc != nullptr && m_mc->theWorld != nullptr && m_mc->theWorld->getSaveHandler() != nullptr)
    {
        std::string saveDir = m_mc->theWorld->getSaveHandler()->getSaveDirectory();
        if (!saveDir.empty())
        {
            return PlatformStorage::join(saveDir, "waypoints.txt");
        }
    }
    File *dir = Minecraft::getMinecraftDir();
    if (dir != nullptr)
        return PlatformStorage::join(dir->toString(), "waypoints.txt");
    return "waypoints.txt";
}

void ReiMinimap::loadWaypoints()
{
    m_waypoints.clear();
    std::string path = getWaypointsFilePath();
    std::vector<unsigned char> bytes;
    if (!PlatformStorage::readFile(path, bytes) || bytes.empty())
    {
        if (m_mc != nullptr && m_mc->theWorld != nullptr && m_mc->theWorld->getWorldInfo() != nullptr)
        {
            int_t sx = m_mc->theWorld->getWorldInfo()->getSpawnX();
            int_t sy = m_mc->theWorld->getWorldInfo()->getSpawnY();
            int_t sz = m_mc->theWorld->getWorldInfo()->getSpawnZ();
            m_waypoints.push_back({ "Spawn", sx, sy, sz, WAYPOINT_PALETTE[0], true });
        }
        else
        {
            m_waypoints.push_back({ "Spawn", 0, 64, 0, WAYPOINT_PALETTE[0], true });
        }
        return;
    }

    std::string content(bytes.begin(), bytes.end());
    size_t start = 0;
    while (start < content.size())
    {
        size_t end = content.find('\n', start);
        std::string line;
        if (end == std::string::npos)
        {
            line = content.substr(start);
            start = content.size();
        }
        else
        {
            line = content.substr(start, end - start);
            start = end + 1;
        }

        while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t'))
            line.pop_back();

        if (line.empty() || line[0] == '#')
            continue;

        size_t p1 = line.find(':');
        if (p1 == std::string::npos) continue;
        size_t p2 = line.find(':', p1 + 1);
        if (p2 == std::string::npos) continue;
        size_t p3 = line.find(':', p2 + 1);
        if (p3 == std::string::npos) continue;
        size_t p4 = line.find(':', p3 + 1);
        if (p4 == std::string::npos) continue;
        size_t p5 = line.find(':', p4 + 1);

        std::string name = line.substr(0, p1);
        int_t x = std::atoi(line.substr(p1 + 1, p2 - p1 - 1).c_str());
        int_t y = std::atoi(line.substr(p2 + 1, p3 - p2 - 1).c_str());
        int_t z = std::atoi(line.substr(p3 + 1, p4 - p3 - 1).c_str());
        int_t color = (p4 != std::string::npos && p5 != std::string::npos) ? std::atoi(line.substr(p4 + 1, p5 - p4 - 1).c_str()) : WAYPOINT_PALETTE[m_waypoints.size() % NUM_PALETTE_COLORS];
        bool en = (p5 != std::string::npos) ? (line.substr(p5 + 1) == "1" || line.substr(p5 + 1) == "true") : true;

        m_waypoints.push_back({ name, x, y, z, color, en });
    }
}

void ReiMinimap::saveWaypoints()
{
    std::string path = getWaypointsFilePath();
    std::string content = "# Rei's Minimap Waypoints\n";
    for (const auto &wp : m_waypoints)
    {
        content += wp.name + ":" + std::to_string(wp.x) + ":" + std::to_string(wp.y) + ":" + std::to_string(wp.z) + ":" + std::to_string(wp.color) + ":" + (wp.enabled ? "1" : "0") + "\n";
    }
    PlatformStorage::writeFile(path, content.data(), content.size());
}