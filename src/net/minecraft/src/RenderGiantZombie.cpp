#include "RenderGiantZombie.h"
#include "EntityGiantZombie.h"
#include "ModelBase.h"
#include "platform/RenderAPI.h"

RenderGiantZombie::RenderGiantZombie(ModelBase* modelbase, float f, float f1) : RenderLiving(modelbase, f * f1) {
    scale = f1;
}

void RenderGiantZombie::preRenderScale(EntityGiantZombie* entitygiantzombie, float f) {
    renderScale(scale, scale, scale);
}

void RenderGiantZombie::preRenderCallback(EntityLiving* entityliving, float f) {
    preRenderScale((EntityGiantZombie*)entityliving, f);
}