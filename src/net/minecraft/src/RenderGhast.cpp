#include "RenderGhast.h"
#include "EntityGhast.h"
#include "ModelGhast.h"
#include "platform/RenderAPI.h"

RenderGhast::RenderGhast() : RenderLiving(new ModelGhast(), 0.5f) {
}

void RenderGhast::preRenderGhastScale(EntityGhast* entityghast, float f) {
    EntityGhast* entityghast1 = entityghast;
    float f1 = ((float)entityghast1->prevAttackCounter + (float)(entityghast1->attackCounter - entityghast1->prevAttackCounter) * f) / 20.0f;
    if (f1 < 0.0f) {
        f1 = 0.0f;
    }
    f1 = 1.0f / (f1 * f1 * f1 * f1 * f1 * 2.0f + 1.0f);
    float f2 = (8.0f + f1) / 2.0f;
    float f3 = (8.0f + 1.0f / f1) / 2.0f;
    renderScale(f3, f2, f3);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void RenderGhast::preRenderCallback(EntityLiving* entityliving, float f) {
    preRenderGhastScale((EntityGhast*)entityliving, f);
}