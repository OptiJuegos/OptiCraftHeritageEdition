#include "RenderChicken.h"
#include "EntityChicken.h"
#include "MathHelper.h"
#include "ModelBase.h"
#include "EntityLiving.h"
#include "Entity.h"

RenderChicken::RenderChicken(ModelBase* modelBase, float f) : RenderLiving(modelBase, f) {
}

void RenderChicken::renderChicken(EntityChicken* entityChicken, double d, double d1, double d2, float f, float f1) {
    RenderLiving::doRenderLiving(entityChicken, d, d1, d2, f, f1);
}

float RenderChicken::getWingRotation(EntityChicken* entityChicken, float f) {
    float f1 = entityChicken->field_756_e + (entityChicken->field_752_b - entityChicken->field_756_e) * f;
    float f2 = entityChicken->field_757_d + (entityChicken->destPos - entityChicken->field_757_d) * f;
    return (MathHelper::sin(f1) + 1.0f) * f2;
}

float RenderChicken::getTicksExisted(EntityLiving* entityLiving, float f) {
    return getWingRotation((EntityChicken*)entityLiving, f);
}

void RenderChicken::doRenderLiving(EntityLiving* entityLiving, double d, double d1, double d2, float f, float f1) {
    renderChicken((EntityChicken*)entityLiving, d, d1, d2, f, f1);
}

void RenderChicken::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    renderChicken((EntityChicken*)entity, d, d1, d2, f, f1);
}
