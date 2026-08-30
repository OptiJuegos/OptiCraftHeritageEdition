#include "TileEntityRenderer.h"

#include "platform/RenderAPI.h"
#include "TileEntity.h"
#include "TileEntitySpecialRenderer.h"
#include "TileEntitySign.h"
#include "TileEntitySignRenderer.h"
#include "TileEntityMobSpawner.h"
#include "TileEntityMobSpawnerRenderer.h"
#include "TileEntityPiston.h"
#include "TileEntityChest.h"
#include "TileEntityChestRenderer.h"
#include "TileEntityEnchantmentTable.h"
#include "TileEntityEndPortal.h"
#include "RenderEndPortal.h"
#include "RenderEnchantmentTable.h"
#include "TileEntityRendererPiston.h"
#include "FontRenderer.h"
#include "RenderEngine.h"
#include "OpenGlHelper.h"
#include "World.h"
#include "EntityLiving.h"
#include <typeindex>

TileEntityRenderer TileEntityRenderer::instance;
double TileEntityRenderer::staticPlayerX = 0.0;
double TileEntityRenderer::staticPlayerY = 0.0;
double TileEntityRenderer::staticPlayerZ = 0.0;

TileEntityRenderer::TileEntityRenderer()
    : fontRenderer(nullptr)
    , renderEngine(nullptr)
    , worldObj(nullptr)
    , entityLivingPlayer(nullptr)
    , playerYaw(0.0f)
    , playerPitch(0.0f)
    , playerX(0.0)
    , playerY(0.0)
    , playerZ(0.0)
{
    specialRendererMap[typeid(TileEntitySign).name()]        = new TileEntitySignRenderer();
    specialRendererMap[typeid(TileEntityMobSpawner).name()]  = new TileEntityMobSpawnerRenderer();
    specialRendererMap[typeid(TileEntityPiston).name()]      = new TileEntityRendererPiston();
    specialRendererMap[typeid(TileEntityChest).name()]       = new TileEntityChestRenderer();
    specialRendererMap[typeid(TileEntityEnchantmentTable).name()] = new RenderEnchantmentTable();
    specialRendererMap[typeid(TileEntityEndPortal).name()] = new RenderEndPortal();

    for (auto &pair : specialRendererMap)
        if (pair.second)
            pair.second->setTileEntityRenderer(this);
}

TileEntityRenderer::~TileEntityRenderer()
{
    for (auto &pair : specialRendererMap)
        delete pair.second;
    specialRendererMap.clear();
}

void TileEntityRenderer::setWorld(World *world)
{
    worldObj = world;
    for (auto &pair : specialRendererMap)
        if (pair.second)
            pair.second->onWorldChanged(world);
}

TileEntitySpecialRenderer *TileEntityRenderer::getSpecialRendererForEntity(TileEntity *tileentity)
{
    if (tileentity == nullptr)
        return nullptr;

    auto exact = specialRendererMap.find(typeid(*tileentity).name());
    if (exact != specialRendererMap.end())
        return exact->second;

    // Java walks the superclass chain.  dynamic_cast gives the same semantics
    // for the registered vanilla TileEntity base classes and their subclasses.
    if (dynamic_cast<TileEntitySign *>(tileentity) != nullptr)
        return specialRendererMap[typeid(TileEntitySign).name()];
    if (dynamic_cast<TileEntityMobSpawner *>(tileentity) != nullptr)
        return specialRendererMap[typeid(TileEntityMobSpawner).name()];
    if (dynamic_cast<TileEntityPiston *>(tileentity) != nullptr)
        return specialRendererMap[typeid(TileEntityPiston).name()];
    if (dynamic_cast<TileEntityChest *>(tileentity) != nullptr)
        return specialRendererMap[typeid(TileEntityChest).name()];
    if (dynamic_cast<TileEntityEnchantmentTable *>(tileentity) != nullptr)
        return specialRendererMap[typeid(TileEntityEnchantmentTable).name()];
    if (dynamic_cast<TileEntityEndPortal *>(tileentity) != nullptr)
        return specialRendererMap[typeid(TileEntityEndPortal).name()];
    return nullptr;
}

bool TileEntityRenderer::hasSpecialRenderer(TileEntity *tileentity)
{
    return getSpecialRendererForEntity(tileentity) != nullptr;
}

void TileEntityRenderer::renderTileEntity(TileEntity *tileentity, float f)
{
    if (tileentity == nullptr || tileentity->isInvalid() || worldObj == nullptr)
        return;

    if (tileentity->getDistanceFrom(playerX, playerY, playerZ) < 4096.0)
    {
        const int_t brightness = worldObj->getLightBrightnessForSkyBlocks(
            tileentity->xCoord, tileentity->yCoord, tileentity->zCoord, 0);
        const int_t lightU = brightness % 65536;
        const int_t lightV = brightness / 65536;
        OpenGlHelper::setLightmapTextureCoords(
            OpenGlHelper::lightmapTexUnit, (float)lightU, (float)lightV);
        renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        renderTileEntityAt(tileentity,
            (double)tileentity->xCoord - staticPlayerX,
            (double)tileentity->yCoord - staticPlayerY,
            (double)tileentity->zCoord - staticPlayerZ, f);
    }
}

void TileEntityRenderer::renderTileEntityAt(TileEntity *tileentity, double d, double d1, double d2, float f)
{
    if (tileentity == nullptr || tileentity->isInvalid())
        return;

    TileEntitySpecialRenderer *renderer = getSpecialRendererForEntity(tileentity);
    if (renderer != nullptr)
        renderer->renderTileEntityAt(tileentity, d, d1, d2, f);
}

void TileEntityRenderer::cacheActiveRenderInfo(World *world, RenderEngine *renderengine, FontRenderer *fontrenderer, EntityLiving *entityliving, float f)
{
    if (worldObj != world)
        setWorld(world);
    renderEngine      = renderengine;
    entityLivingPlayer = entityliving;
    fontRenderer      = fontrenderer;
    playerYaw   = entityliving->prevRotationYaw   + (entityliving->rotationYaw   - entityliving->prevRotationYaw)   * f;
    playerPitch = entityliving->prevRotationPitch + (entityliving->rotationPitch - entityliving->prevRotationPitch) * f;
    playerX = entityliving->lastTickPosX + (entityliving->posX - entityliving->lastTickPosX) * (double)f;
    playerY = entityliving->lastTickPosY + (entityliving->posY - entityliving->lastTickPosY) * (double)f;
    playerZ = entityliving->lastTickPosZ + (entityliving->posZ - entityliving->lastTickPosZ) * (double)f;
}

void TileEntityRenderer::getPistonExtensionRender(World *world)
{
    worldObj = world;
}

void TileEntityRenderer::getPistonExtensionFacing()
{
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
    renderEnable(RenderCapability::Blend);
    renderDisable(RenderCapability::CullFace);
}

void TileEntityRenderer::getPistonExtensionBounds(FontRenderer *fontrenderer)
{
    fontRenderer = fontrenderer;
}

void TileEntityRenderer::getPistonExtensionTexture(RenderEngine *renderengine)
{
    renderEngine = renderengine;
}
