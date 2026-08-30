#include "RenderCow.h"
#include "EntityCow.h"
#include "ModelBase.h"
#include "EntityLiving.h"
#include "Entity.h"

RenderCow::RenderCow(ModelBase* modelBase, float f) : RenderLiving(modelBase, f) {
}

void RenderCow::renderCow(EntityCow* entityCow, double d, double d1, double d2, float f, float f1) {
    RenderLiving::doRenderLiving(entityCow, d, d1, d2, f, f1);
}

void RenderCow::doRenderLiving(EntityLiving* entityLiving, double d, double d1, double d2, float f, float f1) {
    renderCow((EntityCow*)entityLiving, d, d1, d2, f, f1);
}

void RenderCow::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    renderCow((EntityCow*)entity, d, d1, d2, f, f1);
}