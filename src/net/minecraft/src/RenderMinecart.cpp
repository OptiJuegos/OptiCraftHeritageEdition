#include "RenderMinecart.h"
#include "java/Math.h"

#include <cmath>
#include <cstdint>
#include "EntityMinecart.h"
#include "ModelMinecart.h"
#include "MathHelper.h"
#include "Vec3D.h"
#include "RenderBlocks.h"
#include "Block.h"
#include "platform/RenderAPI.h"

RenderMinecart::RenderMinecart() {
    shadowSize = 0.5f;
    modelMinecart = new ModelMinecart();
}

RenderMinecart::~RenderMinecart()
{
    delete modelMinecart;
    modelMinecart = nullptr;
}

void RenderMinecart::renderMinecart(EntityMinecart* entityMinecart, double d, double d1, double d2, float f, float f1) {
    renderPushMatrix();
    std::uint64_t jitterSeed = static_cast<std::uint64_t>(static_cast<std::int64_t>(entityMinecart->entityId)) * 493286711ULL;
    jitterSeed = jitterSeed * jitterSeed * 4392167121ULL + jitterSeed * 98761ULL;
    const float jitterX = ((((float)((jitterSeed >> 16) & 7ULL) + 0.5f) / 8.0f) - 0.5f) * 0.004f;
    const float jitterY = ((((float)((jitterSeed >> 20) & 7ULL) + 0.5f) / 8.0f) - 0.5f) * 0.004f;
    const float jitterZ = ((((float)((jitterSeed >> 24) & 7ULL) + 0.5f) / 8.0f) - 0.5f) * 0.004f;
    renderTranslate(jitterX, jitterY, jitterZ);
#if PLATFORM_FLOAT_VERTEX_MATH
    const float d3f = static_cast<float>(entityMinecart->lastTickPosX) +
        (static_cast<float>(entityMinecart->posX) - static_cast<float>(entityMinecart->lastTickPosX)) * f1;
    const float d4f = static_cast<float>(entityMinecart->lastTickPosY) +
        (static_cast<float>(entityMinecart->posY) - static_cast<float>(entityMinecart->lastTickPosY)) * f1;
    const float d5f = static_cast<float>(entityMinecart->lastTickPosZ) +
        (static_cast<float>(entityMinecart->posZ) - static_cast<float>(entityMinecart->lastTickPosZ)) * f1;
    const double d3 = static_cast<double>(d3f);
    const double d4 = static_cast<double>(d4f);
    const double d5 = static_cast<double>(d5f);
#else
    const double d3 = entityMinecart->lastTickPosX + (entityMinecart->posX - entityMinecart->lastTickPosX) * (double)f1;
    const double d4 = entityMinecart->lastTickPosY + (entityMinecart->posY - entityMinecart->lastTickPosY) * (double)f1;
    const double d5 = entityMinecart->lastTickPosZ + (entityMinecart->posZ - entityMinecart->lastTickPosZ) * (double)f1;
#endif
    const float d6 = 0.3f;
    Vec3D* vec3d = entityMinecart->getPos(d3, d4, d5);
    float f5 = entityMinecart->prevRotationPitch + (entityMinecart->rotationPitch - entityMinecart->prevRotationPitch) * f1;
    if (vec3d != nullptr) {
        Vec3D* vec3d1 = entityMinecart->getPosOffset(d3, d4, d5, d6);
        Vec3D* vec3d2 = entityMinecart->getPosOffset(d3, d4, d5, -d6);
        if (vec3d1 == nullptr) vec3d1 = vec3d;
        if (vec3d2 == nullptr) vec3d2 = vec3d;
        d += vec3d->xCoord - d3;
        d1 += (vec3d1->yCoord + vec3d2->yCoord) / 2.0 - d4;
        d2 += vec3d->zCoord - d5;
        Vec3D* vec3d3 = vec3d2->addVector(-vec3d1->xCoord, -vec3d1->yCoord, -vec3d1->zCoord);
        if (vec3d3->lengthVector() != 0.0) {
            vec3d3 = vec3d3->normalize();
#if PLATFORM_FLOAT_VERTEX_MATH
            const float directionX = (float)vec3d3->xCoord;
            const float directionY = (float)vec3d3->yCoord;
            const float directionZ = (float)vec3d3->zCoord;
            f = std::atan2(directionZ, directionX) * 180.0f / 3.1415926535897931f;
            f5 = std::atan(directionY) * 73.0f;
#else
            f = (float)(JavaMath::atan2(vec3d3->zCoord, vec3d3->xCoord) * 180.0 / 3.1415926535897931);
            f5 = (float)(std::atan(vec3d3->yCoord) * 73.0);
#endif
        }
    }
    renderTranslate((float)d, (float)d1, (float)d2);
    renderRotate(180.0f - f, 0.0f, 1.0f, 0.0f);
    renderRotate(-f5, 0.0f, 0.0f, 1.0f);
    float f6 = (float)entityMinecart->getTimeSinceHit() - f1;
    float f7 = (float)entityMinecart->getDamageTaken() - f1;
    if (f7 < 0.0f) f7 = 0.0f;
    if (f6 > 0.0f) {
        renderRotate(MathHelper::sin(f6) * f6 * f7 / 10.0f * (float)entityMinecart->getForwardDirection(), 1.0f, 0.0f, 0.0f);
    }
    if (entityMinecart->getMinecartType() != 0) {
        loadTexture("/terrain.png");
        const float blockScale = 12.0f / 16.0f;
        renderScale(blockScale, blockScale, blockScale);
        if (entityMinecart->getMinecartType() == 1) {
            renderTranslate(-0.5f, 0.0f, 0.5f);
            renderRotate(90.0f, 0.0f, 1.0f, 0.0f);
            renderBlocks->renderBlockAsItem(Block::chest, 0, entityMinecart->getEntityBrightness(f1));
            renderRotate(-90.0f, 0.0f, 1.0f, 0.0f);
            renderTranslate(0.5f, 0.0f, -0.5f);
            renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        } else if (entityMinecart->getMinecartType() == 2) {
            renderTranslate(0.0f, 5.0f / 16.0f, 0.0f);
            renderRotate(90.0f, 0.0f, 1.0f, 0.0f);
            renderBlocks->renderBlockAsItem(Block::stoneOvenIdle, 0, entityMinecart->getEntityBrightness(f1));
            renderRotate(-90.0f, 0.0f, 1.0f, 0.0f);
            renderTranslate(0.0f, -(5.0f / 16.0f), 0.0f);
            renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        }
        renderScale(1.0f / blockScale, 1.0f / blockScale, 1.0f / blockScale);
    }
    loadTexture("/item/cart.png");
    renderScale(-1.0f, -1.0f, 1.0f);
    modelMinecart->render(0.0f, 0.0f, -0.1f, 0.0f, 0.0f, 0.0625f);
    renderPopMatrix();
}

void RenderMinecart::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    renderMinecart((EntityMinecart*)entity, d, d1, d2, f, f1);
}