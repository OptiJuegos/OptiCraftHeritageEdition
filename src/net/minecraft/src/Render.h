// ============================================================================
// FILE: Render.h
// ============================================================================
#ifndef RENDER_H
#define RENDER_H

#include <string>
#include "Entity.h"
#include "ModelBiped.h"
#include "RenderBlocks.h"
#include "RenderManager.h"
#include "RenderEngine.h"
#include "Block.h"
#include "BlockFire.h"
#include "Tessellator.h"
#include "AxisAlignedBB.h"
#include "MathHelper.h"
#include "World.h"
#include "GameSettings.h"
#include "ModelBase.h"
#include "FontRenderer.h"
#include "platform/RenderAPI.h"

class RenderManager;

class Render {
public:
    RenderBlocks* renderBlocks;
    float shadowSize;
    float shadowOpaque;
    RenderManager* renderManager;

    Render();
    virtual ~Render();

    virtual void doRender(Entity* entity, double x, double y, double z, float yaw, float partialTicks) = 0;

    void setRenderManager(RenderManager* manager) { renderManager = manager; }

protected:
    void loadTexture(const std::string& path);
    bool loadDownloadableImageTexture(const std::string& url, const std::string& fallback);

    void renderEntityOnFire(Entity* entity, double x, double y, double z, float partialTicks);
    void renderShadow(Entity* entity, double x, double y, double z, float opacity, float partialTicks);
    World* getWorldFromRenderManager();
    FontRenderer* getFontRendererFromRenderManager();
    void renderShadowOnBlock(Block* block, double x, double y, double z, int blockX, int blockY, int blockZ, float opacity, float shadowSize, double offsetX, double offsetY, double offsetZ);

public:
    static void renderOffsetAABB(AxisAlignedBB* aabb, double x, double y, double z);
    static void renderAABB(AxisAlignedBB* aabb);
    void doRenderShadowAndFire(Entity* entity, double x, double y, double z, float yaw, float partialTicks);
};

#endif
