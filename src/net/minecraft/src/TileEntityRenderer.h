#pragma once

#include <map>
#include <string>

class TileEntity;
class TileEntitySpecialRenderer;
class FontRenderer;
class RenderEngine;
class World;
class EntityLiving;

class TileEntityRenderer {
public:
    TileEntityRenderer();
    ~TileEntityRenderer();

    bool hasSpecialRenderer(TileEntity *tileentity);
    TileEntitySpecialRenderer *getSpecialRendererForEntity(TileEntity *tileentity);
    void renderTileEntity(TileEntity* tileentity, float f);
    void renderTileEntityAt(TileEntity* tileentity, double d, double d1, double d2, float f);
    // func_217_a — caches camera/render context before a render pass
    void cacheActiveRenderInfo(World* world, RenderEngine* renderengine, FontRenderer* fontrenderer, EntityLiving* entityliving, float f);
    void getPistonExtensionRender(World* world);
    void getPistonExtensionFacing();
    void getPistonExtensionBounds(FontRenderer* fontrenderer);
    void getPistonExtensionTexture(RenderEngine* renderengine);

    static TileEntityRenderer instance;

    // Camera position for this render pass (set by cacheActiveRenderInfo)
    static double staticPlayerX;
    static double staticPlayerY;
    static double staticPlayerZ;

    void setWorld(World* world);   // func_31072_a — updates worldObj on all sub-renderers

    std::map<std::string, TileEntitySpecialRenderer*> specialRendererMap;
    FontRenderer* fontRenderer;
    RenderEngine* renderEngine;
    World* worldObj;
    EntityLiving* entityLivingPlayer;
    float playerYaw;
    float playerPitch;
    double playerX;
    double playerY;
    double playerZ;
};
