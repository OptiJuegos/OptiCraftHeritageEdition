#include "RenderPig.h"
#include "EntityPig.h"
#include "ModelBase.h"
#include "EntityLiving.h"
#include "Entity.h"

RenderPig::RenderPig(ModelBase* modelBase, ModelBase* modelBase2, float shadowSize) : RenderLiving(modelBase, shadowSize) {
    setRenderPassModel(modelBase2);
}

RenderPig::~RenderPig()
{
    delete renderPassModel;
    renderPassModel = nullptr;
}

void RenderPig::renderPig(EntityPig* entityPig, double d, double d1, double d2, float f, float f1) {
    RenderLiving::doRenderLiving(entityPig, d, d1, d2, f, f1);
}

void RenderPig::doRenderLiving(EntityLiving* entityLiving, double d, double d1, double d2, float f, float f1) {
    renderPig((EntityPig*)entityLiving, d, d1, d2, f, f1);
}

void RenderPig::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    renderPig((EntityPig*)entity, d, d1, d2, f, f1);
}


int RenderPig::renderSaddledPig(EntityPig* entityPig, int i, float)
{
    loadTexture("/mob/saddle.png");
    return i == 0 && entityPig != nullptr && entityPig->getSaddled() ? 1 : -1;
}

int RenderPig::shouldRenderPass(EntityLiving* entityLiving, int i, float f)
{
    return renderSaddledPig(static_cast<EntityPig*>(entityLiving), i, f);
}
