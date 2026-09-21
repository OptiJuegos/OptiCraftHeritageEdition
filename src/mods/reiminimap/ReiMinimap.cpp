#include "ReiMinimap.h"
#include "Minecraft.h"
#include "GuiIngame.h"
#include "FontRenderer.h"
#include "RenderEngine.h"
#include "EntityPlayerSP.h"
#include "World.h"
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

#include <cmath>
#include <algorithm>
#include <cstdio>
#include <cstdlib>

#if PLATFORM_PS2
#include "ps2/input/Ps2PadState.h"
#endif

static const int_t MAP_RES = 64;

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
    , m_mapTextureId(-1)
    , m_lastPlayerX(-999999)
    , m_lastPlayerZ(-999999)
    , m_updateTicks(0)
    , m_initialized(false)
    , m_waypointComboWasPressed(false)
    , m_toastTimer(0)
{
}

ReiMinimap::~ReiMinimap()
{
    if (m_mapTextureId >= 0 && m_mc != nullptr && m_mc->renderEngine != nullptr)
    {
        m_mc->renderEngine->deleteTexture(m_mapTextureId);
        m_mapTextureId = -1;
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
        m_mapTextureId = m_mc->renderEngine->allocateAndSetupTexture(m_mapImage.get());
    }

    loadWaypoints();
    m_initialized = true;
    MC_LOG_INFO("mods", "Rei's Minimap initialized with texture ID %d\n", m_mapTextureId);
}

void ReiMinimap::update()
{
    if (!m_enabled || m_mc == nullptr || m_mc->thePlayer == nullptr || m_mc->theWorld == nullptr)
        return;

    m_updateTicks++;
    if (m_toastTimer > 0)
        m_toastTimer--;

#if PLATFORM_PS2
    const Ps2PadSnapshot &pad = ps2PadGetSnapshot(0);
    bool comboDown = (pad.held & PS2_PAD_TRIANGLE) != 0 && (pad.held & PS2_PAD_UP) != 0;
    if (comboDown && !m_waypointComboWasPressed)
    {
        int_t px = static_cast<int_t>(std::floor(m_mc->thePlayer->posX));
        int_t py = static_cast<int_t>(std::floor(m_mc->thePlayer->posY));
        int_t pz = static_cast<int_t>(std::floor(m_mc->thePlayer->posZ));
        char nameBuf[32];
        std::snprintf(nameBuf, sizeof(nameBuf), "Waypoint %u", static_cast<unsigned>(m_waypoints.size() + 1));
        addWaypoint(nameBuf, px, py, pz, 0xFFFF00);

        m_toastMessage = "Waypoint saved!";
        m_toastTimer = 60; // 3 seconds on screen
        if (m_mc->sndManager != nullptr)
            m_mc->sndManager->playSoundFX("random.orb", 1.0f, 1.0f);
    }
    m_waypointComboWasPressed = comboDown;
#endif

    int_t px = static_cast<int_t>(std::floor(m_mc->thePlayer->posX));
    int_t pz = static_cast<int_t>(std::floor(m_mc->thePlayer->posZ));

    // Update map texture if moved or every 10 ticks
    if (px != m_lastPlayerX || pz != m_lastPlayerZ || m_updateTicks >= 10)
    {
        m_updateTicks = 0;
        m_lastPlayerX = px;
        m_lastPlayerZ = pz;
        updateMapTexture();
    }
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

void ReiMinimap::updateMapTexture()
{
    if (m_mc == nullptr || m_mc->thePlayer == nullptr || m_mc->theWorld == nullptr || !m_mapImage)
        return;

    int_t playerX = static_cast<int_t>(std::floor(m_mc->thePlayer->posX));
    int_t playerZ = static_cast<int_t>(std::floor(m_mc->thePlayer->posZ));
    World *world = m_mc->theWorld;

    for (int_t dy = 0; dy < MAP_RES; ++dy)
    {
        int_t wz = playerZ + (dy - MAP_RES / 2);
        for (int_t dx = 0; dx < MAP_RES; ++dx)
        {
            int_t wx = playerX + (dx - MAP_RES / 2);
            Chunk *chunk = world->getChunkFromBlockCoords(wx, wz);

            int_t r = 24, g = 24, b = 24, a = 255;

            if (chunk != nullptr && !chunk->isEmpty())
            {
                int_t lx = wx & 15;
                int_t lz = wz & 15;
                int_t y = chunk->getHeightValue(lx, lz) + 1;
                int_t blockId = 0;

                // Walk down to find the highest non-air solid/liquid block
                while (y > 1)
                {
                    blockId = chunk->getBlockID(lx, y - 1, lz);
                    if (blockId != 0 && Block::blocksList[blockId] != nullptr)
                    {
                        Material *mat = Block::blocksList[blockId]->blockMaterial;
                        if (mat != nullptr && mat->materialMapColor != nullptr && mat->materialMapColor != MapColor::airColor)
                        {
                            break;
                        }
                    }
                    y--;
                }

                int_t northHeight = world->getHeightValue(wx, wz - 1);
                int_t col = getBlockColor(blockId, y, northHeight);

                r = col & 0xFF;
                g = (col >> 8) & 0xFF;
                b = (col >> 16) & 0xFF;
                a = 255;
            }

            size_t idx = (static_cast<size_t>(dy) * MAP_RES + static_cast<size_t>(dx)) * 4;
            m_pixelData[idx + 0] = static_cast<unsigned char>(r);
            m_pixelData[idx + 1] = static_cast<unsigned char>(g);
            m_pixelData[idx + 2] = static_cast<unsigned char>(b);
            m_pixelData[idx + 3] = static_cast<unsigned char>(a);
        }
    }

    m_mapImage->setRGB(0, 0, MAP_RES, MAP_RES, m_pixelData.data());

    if (m_mapTextureId >= 0 && m_mc->renderEngine != nullptr)
    {
        m_mc->renderEngine->setupTexture(m_mapImage.get(), m_mapTextureId);
    }
}

void ReiMinimap::render(GuiIngame *, int_t screenWidth, int_t, float_t)
{
    if (!m_enabled || m_mc == nullptr || m_mc->thePlayer == nullptr || m_mapTextureId < 0)
        return;

    renderDisable(RenderCapability::DepthTest);

    // First frame initialization if not yet updated
    if (m_lastPlayerX == -999999)
    {
        m_lastPlayerX = static_cast<int_t>(std::floor(m_mc->thePlayer->posX));
        m_lastPlayerZ = static_cast<int_t>(std::floor(m_mc->thePlayer->posZ));
        updateMapTexture();
    }

    // Position: Top-right corner with 6px margin
    const int_t mapSize = 64;
    const int_t posX = screenWidth - mapSize - 6;
    const int_t posY = 6;

    // Background panel & border
    drawColoredRect(posX - 1, posY - 1, posX + mapSize + 1, posY + mapSize + 1, 0xA0000000);
    drawColoredRect(posX - 2, posY - 2, posX + mapSize + 2, posY - 1, 0xFF555555);
    drawColoredRect(posX - 2, posY + mapSize + 1, posX + mapSize + 2, posY + mapSize + 2, 0xFF555555);
    drawColoredRect(posX - 2, posY - 2, posX - 1, posY + mapSize + 2, 0xFF555555);
    drawColoredRect(posX + mapSize + 1, posY - 2, posX + mapSize + 2, posY + mapSize + 2, 0xFF555555);

    // Draw the 64x64 dynamic map texture
    renderEnable(RenderCapability::Texture2D);
    renderEnable(RenderCapability::Blend);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    renderBindTexture(m_mapTextureId);

    Tessellator *tess = &Tessellator::instance;
    tess->startDrawingQuads();
    tess->setColorOpaque_I(0xFFFFFF);
    tess->addVertexWithUV(posX,           posY + mapSize, 0.0, 0.0, 1.0);
    tess->addVertexWithUV(posX + mapSize, posY + mapSize, 0.0, 1.0, 1.0);
    tess->addVertexWithUV(posX + mapSize, posY,           0.0, 1.0, 0.0);
    tess->addVertexWithUV(posX,           posY,           0.0, 0.0, 0.0);
    tess->draw();

    // Cardinal direction markers
    FontRenderer *fr = m_mc->fontRenderer;
    if (fr != nullptr)
    {
        fr->drawStringWithShadow("N", posX + mapSize / 2 - 2, posY + 2, 0xFF5555); // Red N
        fr->drawStringWithShadow("S", posX + mapSize / 2 - 2, posY + mapSize - 9, 0xAAAAAA);
        fr->drawStringWithShadow("W", posX + 2, posY + mapSize / 2 - 4, 0xAAAAAA);
        fr->drawStringWithShadow("E", posX + mapSize - 7, posY + mapSize / 2 - 4, 0xAAAAAA);
    }

    // Waypoints rendering (only within visible minimap bounds)
    int_t px = static_cast<int_t>(std::floor(m_mc->thePlayer->posX));
    int_t pz = static_cast<int_t>(std::floor(m_mc->thePlayer->posZ));
    const int_t maxDist = 28;

    for (const auto &wp : m_waypoints)
    {
        if (!wp.enabled) continue;
        int_t rdx = wp.x - px;
        int_t rdz = wp.z - pz;
        if (rdx >= -maxDist && rdx <= maxDist && rdz >= -maxDist && rdz <= maxDist)
        {
            int_t wxScreen = posX + mapSize / 2 + rdx;
            int_t wzScreen = posY + mapSize / 2 + rdz;
            // Draw 4x4 waypoint with 2x2 colored center and 1px dark border
            drawColoredRect(wxScreen - 2, wzScreen - 2, wxScreen + 2, wzScreen + 2, 0xFF000000);
            drawColoredRect(wxScreen - 1, wzScreen - 1, wxScreen + 1, wzScreen + 1, 0xFF000000 | wp.color);
        }
    }

    // Player arrow in center
    const int_t cx = posX + mapSize / 2;
    const int_t cy = posY + mapSize / 2;

    renderPushMatrix();
    renderTranslate(static_cast<float_t>(cx), static_cast<float_t>(cy), 0.0f);
    renderRotate(m_mc->thePlayer->rotationYaw + 180.0f, 0.0f, 0.0f, 1.0f);
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
        int_t py = static_cast<int_t>(std::floor(m_mc->thePlayer->posY));
        char coordBuf[48];
        std::snprintf(coordBuf, sizeof(coordBuf), "X:%d Y:%d Z:%d", px, py, pz);
        int_t strW = fr->getStringWidth(coordBuf);
        fr->drawStringWithShadow(coordBuf, posX + (mapSize - strW) / 2, posY + mapSize + 4, 0xFFFFFF);

        // On-screen notification toast (e.g. when adding a waypoint)
        if (m_toastTimer > 0 && !m_toastMessage.empty())
        {
            int_t toastW = fr->getStringWidth(m_toastMessage);
            int_t toastX = posX + (mapSize - toastW) / 2;
            int_t toastY = posY + mapSize + 16;
            drawColoredRect(toastX - 3, toastY - 2, toastX + toastW + 3, toastY + 10, 0xC0000000);
            fr->drawStringWithShadow(m_toastMessage, toastX, toastY, 0x55FF55);
        }
    }

    renderEnable(RenderCapability::DepthTest);
}

void ReiMinimap::addWaypoint(const std::string &name, int_t x, int_t y, int_t z, int_t color)
{
    m_waypoints.push_back({ name, x, y, z, color, true });
    saveWaypoints();
}

std::string ReiMinimap::getWaypointsFilePath() const
{
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
        m_waypoints.push_back({ "Spawn", 0, 64, 0, 0x00FF88, true });
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
        int_t color = (p5 != std::string::npos) ? std::atoi(line.substr(p4 + 1, p5 - p4 - 1).c_str()) : 0x00FF88;
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
