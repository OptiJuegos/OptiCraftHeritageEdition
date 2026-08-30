// ============================================================================
// FILE: Render.cpp
// ============================================================================
#include "Render.h"
#include "Block.h"
#include "BlockFire.h"
#include "EntityLiving.h"
#include "EntityAnimal.h"
#include "platform/PlatformTuning.h"
#include "platform/RenderAPI.h"



Render::Render() {
    renderBlocks = new RenderBlocks();
    shadowSize = 0.0f;
    shadowOpaque = 1.0f;
    renderManager = nullptr;
}

Render::~Render() {
    delete renderBlocks;
}

void Render::loadTexture(const std::string& path) {
    RenderEngine* renderengine = renderManager->renderEngine;
    renderengine->bindTexture(renderengine->getTexture(path));
}

bool Render::loadDownloadableImageTexture(const std::string& url, const std::string& fallback) {
    if (renderManager == nullptr || renderManager->renderEngine == nullptr)
        return false;
    RenderEngine* renderengine = renderManager->renderEngine;
    int i = renderengine->getTextureForDownloadableImage(url, fallback);
    if (i >= 0) {
        renderengine->bindTexture(i);
        return true;
    }
    return false;
}

void Render::renderEntityOnFire(Entity* entity, double x, double y, double z, float partialTicks) {
    renderDisable(RenderCapability::Lighting);
    int i = Block::fire->blockIndexInTexture;
    int j = (i & 0xf) << 4;
    int k = i & 0xf0;
    float f1 = (float)j / 256.0f;
    float f3 = ((float)j + 15.99f) / 256.0f;
    float f5 = (float)k / 256.0f;
    float f7 = ((float)k + 15.99f) / 256.0f;
    renderPushMatrix();
    renderTranslate((float)x, (float)y, (float)z);
    float f9 = entity->width * 1.4f;
    renderScale(f9, f9, f9);
    loadTexture("/terrain.png");
    Tessellator* tessellator = &Tessellator::instance;
    float f10 = 0.5f;
    float f11 = 0.0f;
    float f12 = entity->height / f9;
    float f13 = (float)(entity->posY - entity->boundingBox->minY);
    renderRotate(-renderManager->playerViewY, 0.0f, 1.0f, 0.0f);
    renderTranslate(0.0f, 0.0f, -0.3f + (float)(int)f12 * 0.02f);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    float f14 = 0.0f;
    int l = 0;
    tessellator->startDrawingQuads();
    while (f12 > 0.0f) {
        float f2, f4, f6, f8;
        if (l % 2 == 0) {
            f2 = (float)j / 256.0f;
            f4 = ((float)j + 15.99f) / 256.0f;
            f6 = (float)k / 256.0f;
            f8 = ((float)k + 15.99f) / 256.0f;
        } else {
            f2 = (float)j / 256.0f;
            f4 = ((float)j + 15.99f) / 256.0f;
            f6 = (float)(k + 16) / 256.0f;
            f8 = ((float)(k + 16) + 15.99f) / 256.0f;
        }
        if ((l / 2) % 2 == 0) {
            float f15 = f4;
            f4 = f2;
            f2 = f15;
        }
        tessellator->addVertexWithUV(f10 - f11, 0.0f - f13, f14, f4, f8);
        tessellator->addVertexWithUV(-f10 - f11, 0.0f - f13, f14, f2, f8);
        tessellator->addVertexWithUV(-f10 - f11, 1.4f - f13, f14, f2, f6);
        tessellator->addVertexWithUV(f10 - f11, 1.4f - f13, f14, f4, f6);
        f12 -= 0.45f;
        f13 -= 0.45f;
        f10 *= 0.9f;
        f14 += 0.03f;
        l++;
    }
    tessellator->draw();
    renderPopMatrix();
    renderEnable(RenderCapability::Lighting);
}

void Render::renderShadow(Entity* entity, double x, double y, double z, float opacity, float partialTicks) {
    renderEnable(RenderCapability::Blend);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
    RenderEngine* renderengine = renderManager->renderEngine;
    renderengine->bindTexture(renderengine->getTexture("%clamp%/misc/shadow.png"));
    World* world = getWorldFromRenderManager();
    renderDepthMask(false);
    float f2 = shadowSize;
    EntityLiving* living = dynamic_cast<EntityLiving*>(entity);
    if (living != nullptr) {
        f2 *= living->getRenderSizeModifier();
        EntityAnimal* animal = dynamic_cast<EntityAnimal*>(living);
        if (animal != nullptr && animal->isChild())
            f2 *= 0.5f;
    }
    double d3 = entity->lastTickPosX + (entity->posX - entity->lastTickPosX) * (double)partialTicks;
    double d4 = entity->lastTickPosY + (entity->posY - entity->lastTickPosY) * (double)partialTicks + (double)entity->getShadowSize();
    double d5 = entity->lastTickPosZ + (entity->posZ - entity->lastTickPosZ) * (double)partialTicks;
    int i = MathHelper::floor_double(d3 - (double)f2);
    int j = MathHelper::floor_double(d3 + (double)f2);
    int k = MathHelper::floor_double(d4 - (double)f2);
    int l = MathHelper::floor_double(d4);
    int i1 = MathHelper::floor_double(d5 - (double)f2);
    int j1 = MathHelper::floor_double(d5 + (double)f2);
    double d6 = x - d3;
    double d7 = y - d4;
    double d8 = z - d5;
    Tessellator* tessellator = &Tessellator::instance;
    tessellator->startDrawingQuads();
    for (int k1 = i; k1 <= j; k1++) {
        for (int l1 = k; l1 <= l; l1++) {
            for (int i2 = i1; i2 <= j1; i2++) {
                int j2 = world->getBlockId(k1, l1 - 1, i2);
                if (j2 > 0 && world->getBlockLightValue(k1, l1, i2) > 3) {
                    renderShadowOnBlock(Block::blocksList[j2], x, y + (double)entity->getShadowSize(), z, k1, l1, i2, opacity, f2, d6, d7 + (double)entity->getShadowSize(), d8);
                }
            }
        }
    }
    tessellator->draw();
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    renderDisable(RenderCapability::Blend);
    renderDepthMask(true);
}

World* Render::getWorldFromRenderManager() {
    return renderManager->worldObj;
}

void Render::renderShadowOnBlock(Block* block, double x, double y, double z, int blockX, int blockY, int blockZ, float opacity, float shadowSize, double offsetX, double offsetY, double offsetZ) {
    Tessellator* tessellator = &Tessellator::instance;
    if (!block->renderAsNormalBlock()) {
        return;
    }
    double d6 = ((double)opacity - (y - ((double)blockY + offsetY)) / 2.0) * 0.5 * (double)getWorldFromRenderManager()->getLightBrightness(blockX, blockY, blockZ);
    if (d6 < 0.0) {
        return;
    }
    if (d6 > 1.0) {
        d6 = 1.0;
    }
    tessellator->setColorRGBA_F(1.0f, 1.0f, 1.0f, (float)d6);
    const double shadowMinXDouble = (double)blockX + block->minX + offsetX;
    const double shadowMaxXDouble = (double)blockX + block->maxX + offsetX;
    const double shadowYDouble = (double)blockY + block->minY + offsetY + 0.015625;
    const double shadowMinZDouble = (double)blockZ + block->minZ + offsetZ;
    const double shadowMaxZDouble = (double)blockZ + block->maxZ + offsetZ;
    const tess_coord_t shadowMinX = static_cast<tess_coord_t>(shadowMinXDouble);
    const tess_coord_t shadowMaxX = static_cast<tess_coord_t>(shadowMaxXDouble);
    const tess_coord_t shadowY = static_cast<tess_coord_t>(shadowYDouble);
    const tess_coord_t shadowMinZ = static_cast<tess_coord_t>(shadowMinZDouble);
    const tess_coord_t shadowMaxZ = static_cast<tess_coord_t>(shadowMaxZDouble);
#if PLATFORM_FLOAT_VERTEX_MATH
    const float shadowX = static_cast<float>(x);
    const float shadowZ = static_cast<float>(z);
    const float f2 = (shadowX - shadowMinX) / 2.0f / shadowSize + 0.5f;
    const float f3 = (shadowX - shadowMaxX) / 2.0f / shadowSize + 0.5f;
    const float f4 = (shadowZ - shadowMinZ) / 2.0f / shadowSize + 0.5f;
    const float f5 = (shadowZ - shadowMaxZ) / 2.0f / shadowSize + 0.5f;
#else
    const float f2 = (float)((x - shadowMinXDouble) / 2.0 / (double)shadowSize + 0.5);
    const float f3 = (float)((x - shadowMaxXDouble) / 2.0 / (double)shadowSize + 0.5);
    const float f4 = (float)((z - shadowMinZDouble) / 2.0 / (double)shadowSize + 0.5);
    const float f5 = (float)((z - shadowMaxZDouble) / 2.0 / (double)shadowSize + 0.5);
#endif
    tessellator->addVertexWithUV(shadowMinX, shadowY, shadowMinZ, f2, f4);
    tessellator->addVertexWithUV(shadowMinX, shadowY, shadowMaxZ, f2, f5);
    tessellator->addVertexWithUV(shadowMaxX, shadowY, shadowMaxZ, f3, f5);
    tessellator->addVertexWithUV(shadowMaxX, shadowY, shadowMinZ, f3, f4);
}

void Render::renderOffsetAABB(AxisAlignedBB* aabb, double x, double y, double z) {
    renderDisable(RenderCapability::Texture2D);
    Tessellator* tessellator = &Tessellator::instance;
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    tessellator->startDrawingQuads();
    tessellator->setTranslationD(x, y, z);
    tessellator->setNormal(0.0f, 0.0f, -1.0f);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->minZ);
    tessellator->setNormal(0.0f, 0.0f, 1.0f);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->maxZ);
    tessellator->setNormal(0.0f, -1.0f, 0.0f);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->maxZ);
    tessellator->setNormal(0.0f, 1.0f, 0.0f);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->minZ);
    tessellator->setNormal(-1.0f, 0.0f, 0.0f);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->minZ);
    tessellator->setNormal(1.0f, 0.0f, 0.0f);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->maxZ);
    tessellator->setTranslationD(0.0, 0.0, 0.0);
    tessellator->draw();
    renderEnable(RenderCapability::Texture2D);
}

void Render::renderAABB(AxisAlignedBB* aabb) {
    Tessellator* tessellator = &Tessellator::instance;
    tessellator->startDrawingQuads();
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->maxZ);
    tessellator->draw();
}

void Render::doRenderShadowAndFire(Entity* entity, double x, double y, double z, float yaw, float partialTicks) {
#if PLATFORM_SKIP_ENTITY_SHADOWS
    // See PS2_SKIP_ENTITY_SHADOWS: a 1-bit-alpha framebuffer turns the soft
    // shadow gradient into hard black squares, and the block scan behind it is
    // not affordable per entity per frame.
    (void)yaw;
#else
    if (renderManager->options->fancyGraphics && shadowSize > 0.0f) {
        double d = renderManager->getDistanceToCamera(entity->posX, entity->posY, entity->posZ);
        float f = (float)((1.0 - d / 256.0) * (double)shadowOpaque);
        if (f > 0.0f) {
            renderShadow(entity, x, y, z, f, partialTicks);
        }
    }
#endif
    if (entity->isBurning()) {
        renderEntityOnFire(entity, x, y, z, partialTicks);
    }
}

FontRenderer* Render::getFontRendererFromRenderManager() {
    return renderManager->getFontRenderer();
}
