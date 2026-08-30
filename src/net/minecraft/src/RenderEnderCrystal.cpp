#include "RenderEnderCrystal.h"

#include "EntityEnderCrystal.h"
#include "MathHelper.h"
#include "ModelEnderCrystal.h"
#include "platform/RenderAPI.h"

RenderEnderCrystal::RenderEnderCrystal() : crystalModel(new ModelEnderCrystal(0.0f))
{
    shadowSize = 0.5f;
}

RenderEnderCrystal::~RenderEnderCrystal()
{
    delete crystalModel;
}

void RenderEnderCrystal::doRender(Entity* entity, double x, double y, double z, float yaw, float partialTick)
{
    EntityEnderCrystal* crystal = static_cast<EntityEnderCrystal*>(entity);
    const float rotation = static_cast<float>(crystal->innerRotation) + partialTick;
    renderPushMatrix();
    renderTranslate(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
    loadTexture("/mob/enderdragon/crystal.png");
    float bob = MathHelper::sin(rotation * 0.2f) / 2.0f + 0.5f;
    bob += bob * bob;
    crystalModel->renderCrystal(rotation * 3.0f, bob * 0.2f, 1.0f / 16.0f);
    renderPopMatrix();
}
