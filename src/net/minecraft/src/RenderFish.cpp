#include "RenderFish.h"
#include "EntityFish.h"
#include "EntityPlayer.h"
#include "Tessellator.h"
#include "MathHelper.h"
#include "Vec3D.h"
#include "GameSettings.h"
#include "platform/RenderAPI.h"
#include "platform/PlatformTuning.h"

RenderFish::RenderFish() {
}

void RenderFish::doRenderFish(EntityFish* entityfish, double d, double d1, double d2, float f, float f1) {
    renderPushMatrix();
    renderTranslate((float)d, (float)d1, (float)d2);
    renderEnable(RenderCapability::RescaleNormal);
    renderScale(0.5f, 0.5f, 0.5f);
    int i = 1;
    int byte0 = 2;
    loadTexture("/particles.png");
    Tessellator* tessellator = &Tessellator::instance;
    float f2 = (float)(i * 8 + 0) / 128.0f;
    float f3 = (float)(i * 8 + 8) / 128.0f;
    float f4 = (float)(byte0 * 8 + 0) / 128.0f;
    float f5 = (float)(byte0 * 8 + 8) / 128.0f;
    float f6 = 1.0f;
    float f7 = 0.5f;
    float f8 = 0.5f;
    renderRotate(180.0f - renderManager->playerViewY, 0.0f, 1.0f, 0.0f);
    renderRotate(-renderManager->playerViewX, 1.0f, 0.0f, 0.0f);
    tessellator->startDrawingQuads();
    tessellator->setNormal(0.0f, 1.0f, 0.0f);
    tessellator->addVertexWithUV(0.0f - f7, 0.0f - f8, 0.0f, f2, f5);
    tessellator->addVertexWithUV(f6 - f7, 0.0f - f8, 0.0f, f3, f5);
    tessellator->addVertexWithUV(f6 - f7, 1.0f - f8, 0.0f, f3, f4);
    tessellator->addVertexWithUV(0.0f - f7, 1.0f - f8, 0.0f, f2, f4);
    tessellator->draw();
    renderDisable(RenderCapability::RescaleNormal);
    renderPopMatrix();

    EntityPlayer *angler = entityfish->getAngler();
    if (angler != nullptr) {
        float f9 = ((angler->prevRotationYaw + (angler->rotationYaw - angler->prevRotationYaw) * f1) * 3.1415927f) / 180.0f;
        float d3 = MathHelper::sin(f9);
        float d5 = MathHelper::cos(f9);
        float f11 = angler->getSwingProgress(f1);
        float f12 = MathHelper::sin(MathHelper::sqrt_float(f11) * 3.1415927f);
        Vec3D* vec3d = Vec3D::createVector(-0.5, 0.03, 0.8);
        vec3d->rotateAroundX((-(angler->prevRotationPitch + (angler->rotationPitch - angler->prevRotationPitch) * f1) * 3.1415927f) / 180.0f);
        vec3d->rotateAroundY((-(angler->prevRotationYaw + (angler->rotationYaw - angler->prevRotationYaw) * f1) * 3.1415927f) / 180.0f);
        vec3d->rotateAroundY(f12 * 0.5f);
        vec3d->rotateAroundX(-f12 * 0.7f);
#if PLATFORM_FLOAT_VERTEX_MATH
        float d7 = static_cast<float>(angler->prevPosX) + (static_cast<float>(angler->posX) - static_cast<float>(angler->prevPosX)) * f1 + static_cast<float>(vec3d->xCoord);
        float d8 = static_cast<float>(angler->prevPosY) + (static_cast<float>(angler->posY) - static_cast<float>(angler->prevPosY)) * f1 + static_cast<float>(vec3d->yCoord);
        float d9 = static_cast<float>(angler->prevPosZ) + (static_cast<float>(angler->posZ) - static_cast<float>(angler->prevPosZ)) * f1 + static_cast<float>(vec3d->zCoord);
#else
        double d7 = angler->prevPosX + (angler->posX - angler->prevPosX) * (double)f1 + vec3d->xCoord;
        double d8 = angler->prevPosY + (angler->posY - angler->prevPosY) * (double)f1 + vec3d->yCoord;
        double d9 = angler->prevPosZ + (angler->posZ - angler->prevPosZ) * (double)f1 + vec3d->zCoord;
#endif
        if (renderManager->options->thirdPersonView) {
            float f10 = ((angler->prevRenderYawOffset + (angler->renderYawOffset - angler->prevRenderYawOffset) * f1) * 3.1415927f) / 180.0f;
            float d4 = MathHelper::sin(f10);
            float d6 = MathHelper::cos(f10);
#if PLATFORM_FLOAT_VERTEX_MATH
            d7 = static_cast<float>(angler->prevPosX) + (static_cast<float>(angler->posX) - static_cast<float>(angler->prevPosX)) * f1 - d6 * 0.35f - d4 * 0.85f;
            d8 = static_cast<float>(angler->prevPosY) + (static_cast<float>(angler->posY) - static_cast<float>(angler->prevPosY)) * f1 - 0.45f;
            d9 = static_cast<float>(angler->prevPosZ) + (static_cast<float>(angler->posZ) - static_cast<float>(angler->prevPosZ)) * f1 - d4 * 0.35f + d6 * 0.85f;
#else
            d7 = (angler->prevPosX + (angler->posX - angler->prevPosX) * (double)f1) - d6 * 0.35 - d4 * 0.85;
            d8 = (angler->prevPosY + (angler->posY - angler->prevPosY) * (double)f1) - 0.45;
            d9 = ((angler->prevPosZ + (angler->posZ - angler->prevPosZ) * (double)f1) - d4 * 0.35) + d6 * 0.85;
#endif
        }
#if PLATFORM_FLOAT_VERTEX_MATH
        const float d10 = static_cast<float>(entityfish->prevPosX) + (static_cast<float>(entityfish->posX) - static_cast<float>(entityfish->prevPosX)) * f1;
        const float d11 = static_cast<float>(entityfish->prevPosY) + (static_cast<float>(entityfish->posY) - static_cast<float>(entityfish->prevPosY)) * f1 + 0.25f;
        const float d12 = static_cast<float>(entityfish->prevPosZ) + (static_cast<float>(entityfish->posZ) - static_cast<float>(entityfish->prevPosZ)) * f1;
#else
        double d10 = entityfish->prevPosX + (entityfish->posX - entityfish->prevPosX) * (double)f1;
        double d11 = entityfish->prevPosY + (entityfish->posY - entityfish->prevPosY) * (double)f1 + 0.25;
        double d12 = entityfish->prevPosZ + (entityfish->posZ - entityfish->prevPosZ) * (double)f1;
#endif
        tess_coord_t d13 = static_cast<tess_coord_t>(static_cast<float>(d7 - d10));
        tess_coord_t d14 = static_cast<tess_coord_t>(static_cast<float>(d8 - d11));
        tess_coord_t d15 = static_cast<tess_coord_t>(static_cast<float>(d9 - d12));
        const tess_coord_t lineX = static_cast<tess_coord_t>(d);
        const tess_coord_t lineY = static_cast<tess_coord_t>(d1);
        const tess_coord_t lineZ = static_cast<tess_coord_t>(d2);
        renderDisable(RenderCapability::Texture2D);
        renderDisable(RenderCapability::Lighting);
        tessellator->startDrawing(3);
        tessellator->setColorOpaque_I(0);
        int j = 16;
        for (int k = 0; k <= j; k++) {
            float f13 = (float)k / (float)j;
            const tess_coord_t lineProgress = static_cast<tess_coord_t>(f13);
            const tess_coord_t lineCurve = static_cast<tess_coord_t>(f13 * f13 + f13);
            tessellator->addVertex(
                lineX + d13 * lineProgress,
                lineY + d14 * lineCurve * static_cast<tess_coord_t>(0.5) + static_cast<tess_coord_t>(0.25),
                lineZ + d15 * lineProgress);
        }
        tessellator->draw();
        renderEnable(RenderCapability::Lighting);
        renderEnable(RenderCapability::Texture2D);
    }
}

void RenderFish::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    doRenderFish((EntityFish*)entity, d, d1, d2, f, f1);
}
