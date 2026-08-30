#include "RenderPainting.h"
#include "EntityPainting.h"
#include "EnumArt.h"
#include "Tessellator.h"
#include "OpenGlHelper.h"
#include "platform/RenderAPI.h"

RenderPainting::RenderPainting() {
}

void RenderPainting::renderPainting(EntityPainting* entityPainting, double d, double d1, double d2, float f, float f1) {
    renderPushMatrix();
    renderTranslate((float)d, (float)d1, (float)d2);
    renderRotate(f, 0.0f, 1.0f, 0.0f);
    renderEnable(RenderCapability::RescaleNormal);
    loadTexture("/art/kz.png");

    EnumArt* enumart = entityPainting->art;
    if (enumart != nullptr) {
        const float scale = 0.0625f;
        renderScale(scale, scale, scale);

        const int width = enumart->sizeX;
        const int height = enumart->sizeY;
        const int offsetX = enumart->offsetX;
        const int offsetY = enumart->offsetY;
        const float minX = (float)(-width) / 2.0f;
        const float minY = (float)(-height) / 2.0f;
        const float frontZ = -0.5f;
        const float backZ = 0.5f;
        Tessellator* tessellator = &Tessellator::instance;

        for (int segmentX = 0; segmentX < width / 16; ++segmentX) {
            for (int segmentY = 0; segmentY < height / 16; ++segmentY) {
                const float maxSegmentX = minX + (float)((segmentX + 1) * 16);
                const float minSegmentX = minX + (float)(segmentX * 16);
                const float maxSegmentY = minY + (float)((segmentY + 1) * 16);
                const float minSegmentY = minY + (float)(segmentY * 16);
                setPaintingBrightness(entityPainting,
                                      (maxSegmentX + minSegmentX) / 2.0f,
                                      (maxSegmentY + minSegmentY) / 2.0f);

                const float frontMaxU = (float)((offsetX + width) - segmentX * 16) / 256.0f;
                const float frontMinU = (float)((offsetX + width) - (segmentX + 1) * 16) / 256.0f;
                const float frontMaxV = (float)((offsetY + height) - segmentY * 16) / 256.0f;
                const float frontMinV = (float)((offsetY + height) - (segmentY + 1) * 16) / 256.0f;
                const float backMinU = 0.75f;
                const float backMaxU = 0.8125f;
                const float backMinV = 0.0f;
                const float backMaxV = 0.0625f;
                const float edgeMinU = 0.75f;
                const float edgeMaxU = 0.8125f;
                const float edgeMinV = 0.001953125f;
                const float edgeMaxV = 0.001953125f;
                const float sideMinU = 0.7519531f;
                const float sideMaxU = 0.7519531f;
                const float sideMinV = 0.0f;
                const float sideMaxV = 0.0625f;

                tessellator->startDrawingQuads();
                tessellator->setNormal(0.0f, 0.0f, -1.0f);
                tessellator->addVertexWithUV(maxSegmentX, minSegmentY, frontZ, frontMinU, frontMaxV);
                tessellator->addVertexWithUV(minSegmentX, minSegmentY, frontZ, frontMaxU, frontMaxV);
                tessellator->addVertexWithUV(minSegmentX, maxSegmentY, frontZ, frontMaxU, frontMinV);
                tessellator->addVertexWithUV(maxSegmentX, maxSegmentY, frontZ, frontMinU, frontMinV);

                tessellator->setNormal(0.0f, 0.0f, 1.0f);
                tessellator->addVertexWithUV(maxSegmentX, maxSegmentY, backZ, backMinU, backMinV);
                tessellator->addVertexWithUV(minSegmentX, maxSegmentY, backZ, backMaxU, backMinV);
                tessellator->addVertexWithUV(minSegmentX, minSegmentY, backZ, backMaxU, backMaxV);
                tessellator->addVertexWithUV(maxSegmentX, minSegmentY, backZ, backMinU, backMaxV);

                tessellator->setNormal(0.0f, -1.0f, 0.0f);
                tessellator->addVertexWithUV(maxSegmentX, maxSegmentY, frontZ, edgeMinU, edgeMinV);
                tessellator->addVertexWithUV(minSegmentX, maxSegmentY, frontZ, edgeMaxU, edgeMinV);
                tessellator->addVertexWithUV(minSegmentX, maxSegmentY, backZ, edgeMaxU, edgeMaxV);
                tessellator->addVertexWithUV(maxSegmentX, maxSegmentY, backZ, edgeMinU, edgeMaxV);

                tessellator->setNormal(0.0f, 1.0f, 0.0f);
                tessellator->addVertexWithUV(maxSegmentX, minSegmentY, backZ, edgeMinU, edgeMinV);
                tessellator->addVertexWithUV(minSegmentX, minSegmentY, backZ, edgeMaxU, edgeMinV);
                tessellator->addVertexWithUV(minSegmentX, minSegmentY, frontZ, edgeMaxU, edgeMaxV);
                tessellator->addVertexWithUV(maxSegmentX, minSegmentY, frontZ, edgeMinU, edgeMaxV);

                tessellator->setNormal(-1.0f, 0.0f, 0.0f);
                tessellator->addVertexWithUV(maxSegmentX, maxSegmentY, backZ, sideMaxU, sideMinV);
                tessellator->addVertexWithUV(maxSegmentX, minSegmentY, backZ, sideMaxU, sideMaxV);
                tessellator->addVertexWithUV(maxSegmentX, minSegmentY, frontZ, sideMinU, sideMaxV);
                tessellator->addVertexWithUV(maxSegmentX, maxSegmentY, frontZ, sideMinU, sideMinV);

                tessellator->setNormal(1.0f, 0.0f, 0.0f);
                tessellator->addVertexWithUV(minSegmentX, maxSegmentY, frontZ, sideMaxU, sideMinV);
                tessellator->addVertexWithUV(minSegmentX, minSegmentY, frontZ, sideMaxU, sideMaxV);
                tessellator->addVertexWithUV(minSegmentX, minSegmentY, backZ, sideMinU, sideMaxV);
                tessellator->addVertexWithUV(minSegmentX, maxSegmentY, backZ, sideMinU, sideMinV);
                tessellator->draw();
            }
        }
    }

    renderDisable(RenderCapability::RescaleNormal);
    renderPopMatrix();
}

void RenderPainting::setPaintingBrightness(EntityPainting* entityPainting, float x, float y) {
    int blockX = MathHelper::floor_double(entityPainting->posX);
    const int blockY = MathHelper::floor_double(entityPainting->posY + (double)(y / 16.0f));
    int blockZ = MathHelper::floor_double(entityPainting->posZ);

    if (entityPainting->direction == 0) {
        blockX = MathHelper::floor_double(entityPainting->posX + (double)(x / 16.0f));
    }
    if (entityPainting->direction == 1) {
        blockZ = MathHelper::floor_double(entityPainting->posZ - (double)(x / 16.0f));
    }
    if (entityPainting->direction == 2) {
        blockX = MathHelper::floor_double(entityPainting->posX - (double)(x / 16.0f));
    }
    if (entityPainting->direction == 3) {
        blockZ = MathHelper::floor_double(entityPainting->posZ + (double)(x / 16.0f));
    }

    const int_t brightness = renderManager->worldObj->getLightBrightnessForSkyBlocks(blockX, blockY, blockZ, 0);
    const int_t lightU = brightness % 65536;
    const int_t lightV = brightness / 65536;
    OpenGlHelper::setLightmapTextureCoords(OpenGlHelper::lightmapTexUnit, (float)lightU, (float)lightV);
    renderColor3f(1.0f, 1.0f, 1.0f);
}

void RenderPainting::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    renderPainting((EntityPainting*)entity, d, d1, d2, f, f1);
}
