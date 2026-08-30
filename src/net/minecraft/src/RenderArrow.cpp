#include "RenderArrow.h"
#include "EntityArrow.h"
#include "Tessellator.h"
#include "MathHelper.h"
#include "Entity.h"
#include "platform/RenderAPI.h"

RenderArrow::RenderArrow() {
}

void RenderArrow::renderArrow(EntityArrow* entityArrow, double d, double d1, double d2, float f, float f1) {
    loadTexture("/item/arrows.png");

    renderPushMatrix();
    renderTranslate((float)d, (float)d1, (float)d2);
    renderRotate((entityArrow->prevRotationYaw + (entityArrow->rotationYaw - entityArrow->prevRotationYaw) * f1) - 90.0f, 0.0f, 1.0f, 0.0f);
    renderRotate(entityArrow->prevRotationPitch + (entityArrow->rotationPitch - entityArrow->prevRotationPitch) * f1, 0.0f, 0.0f, 1.0f);

    Tessellator* tessellator = &Tessellator::instance;

    int i = 0;
    float f2 = 0.0f;
    float f3 = 0.5f;
    float f4 = (float)(0 + i * 10) / 32.0f;
    float f5 = (float)(5 + i * 10) / 32.0f;
    float f6 = 0.0f;
    float f7 = 0.15625f;
    float f8 = (float)(5 + i * 10) / 32.0f;
    float f9 = (float)(10 + i * 10) / 32.0f;
    float f10 = 0.05625f;

    renderEnable(RenderCapability::RescaleNormal);

    float f11 = (float)entityArrow->arrowShake - f1;
    if (f11 > 0.0f) {
        float f12 = -MathHelper::sin(f11 * 3.0f) * f11;
        renderRotate(f12, 0.0f, 0.0f, 1.0f);
    }

    renderRotate(45.0f, 1.0f, 0.0f, 0.0f);
    renderScale(f10, f10, f10);
    renderTranslate(-4.0f, 0.0f, 0.0f);
    renderNormal3f(f10, 0.0f, 0.0f);
    tessellator->startDrawingQuads();
    tessellator->addVertexWithUV(-7.0f, -2.0f, -2.0f, f6, f8);
    tessellator->addVertexWithUV(-7.0f, -2.0f,  2.0f, f7, f8);
    tessellator->addVertexWithUV(-7.0f,  2.0f,  2.0f, f7, f9);
    tessellator->addVertexWithUV(-7.0f,  2.0f, -2.0f, f6, f9);
    tessellator->draw();
    renderNormal3f(-f10, 0.0f, 0.0f);
    tessellator->startDrawingQuads();
    tessellator->addVertexWithUV(-7.0f,  2.0f, -2.0f, f6, f8);
    tessellator->addVertexWithUV(-7.0f,  2.0f,  2.0f, f7, f8);
    tessellator->addVertexWithUV(-7.0f, -2.0f,  2.0f, f7, f9);
    tessellator->addVertexWithUV(-7.0f, -2.0f, -2.0f, f6, f9);
    tessellator->draw();

    for (int_t j = 0; j < 4; ++j) {
        renderRotate(90.0f, 1.0f, 0.0f, 0.0f);
        renderNormal3f(0.0f, 0.0f, f10);
        tessellator->startDrawingQuads();
        tessellator->addVertexWithUV(-8.0f, -2.0f, 0.0f, f2, f4);
        tessellator->addVertexWithUV( 8.0f, -2.0f, 0.0f, f3, f4);
        tessellator->addVertexWithUV( 8.0f,  2.0f, 0.0f, f3, f5);
        tessellator->addVertexWithUV(-8.0f,  2.0f, 0.0f, f2, f5);
        tessellator->draw();
    }

    renderDisable(RenderCapability::RescaleNormal);
    renderPopMatrix();
}

void RenderArrow::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    renderArrow(static_cast<EntityArrow*>(entity), d, d1, d2, f, f1);
}
