#include "RenderDragon.h"
#include "java/Math.h"

#include <cmath>

#include "EntityDragon.h"
#include "EntityEnderCrystal.h"
#include "MathHelper.h"
#include "ModelDragon.h"
#include "OpenGlHelper.h"
#include "RenderHelper.h"
#include "Tessellator.h"
#include "java/Random.h"
#include "platform/RenderAPI.h"

namespace
{
constexpr float kPi = 3.14159265358979323846f;
}


EntityDragon* RenderDragon::entityDragon = nullptr;

RenderDragon::RenderDragon() : RenderLiving(new ModelDragon(0.0f), 0.5f)
{
    modelDragon = static_cast<ModelDragon*>(mainModel);
    setRenderPassModel(mainModel);
}

void RenderDragon::rotateDragonBody(EntityDragon* dragon, float age, float yaw, float partialTick)
{
    (void)age;
    (void)yaw;
    const float historyYaw = static_cast<float>(dragon->func_40160_a(7, partialTick)[0]);
    const float historyPitch = static_cast<float>(dragon->func_40160_a(5, partialTick)[1] - dragon->func_40160_a(10, partialTick)[1]);
    renderRotate(-historyYaw, 0.0f, 1.0f, 0.0f);
    renderRotate(historyPitch * 10.0f, 1.0f, 0.0f, 0.0f);
    renderTranslate(0.0f, 0.0f, 1.0f);
    if (dragon->deathTime > 0)
    {
        float rotation = (static_cast<float>(dragon->deathTime) + partialTick - 1.0f) / 20.0f * 1.6f;
        rotation = MathHelper::sqrt_float(rotation);
        if (rotation > 1.0f)
            rotation = 1.0f;
        renderRotate(rotation * getDeathMaxRotation(dragon), 0.0f, 0.0f, 1.0f);
    }
}

void RenderDragon::renderModel(EntityLiving* entity, float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale)
{
    EntityDragon* dragon = static_cast<EntityDragon*>(entity);
    if (dragon->field_40178_aA > 0)
    {
        const float alpha = static_cast<float>(dragon->field_40178_aA) / 200.0f;
        renderDepthFunc(RenderCompare::LessEqual);
        renderEnable(RenderCapability::AlphaTest);
        renderAlphaFunc(RenderCompare::Greater, alpha);
        loadDownloadableImageTexture(dragon->skinUrl, "/mob/enderdragon/shuffle.png");
        mainModel->render(dragon, limbSwing, limbAmount, age, headYaw, headPitch, scale);
        renderAlphaFunc(RenderCompare::Greater, 0.1f);
        renderDepthFunc(RenderCompare::Equal);
    }

    loadDownloadableImageTexture(dragon->skinUrl, dragon->getEntityTexture());
    mainModel->render(dragon, limbSwing, limbAmount, age, headYaw, headPitch, scale);
    if (dragon->hurtTime > 0)
    {
        renderDepthFunc(RenderCompare::Equal);
        renderDisable(RenderCapability::Texture2D);
        renderEnable(RenderCapability::Blend);
        renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
        renderColor4f(1.0f, 0.0f, 0.0f, 0.5f);
        mainModel->render(dragon, limbSwing, limbAmount, age, headYaw, headPitch, scale);
        renderEnable(RenderCapability::Texture2D);
        renderDisable(RenderCapability::Blend);
        renderDepthFunc(RenderCompare::LessEqual);
    }
}

void RenderDragon::renderHealingBeam(EntityDragon* dragon, double x, double y, double z, float partialTick)
{
    EntityEnderCrystal* crystal = dragon->getHealingEnderCrystal();
    if (crystal == nullptr)
        return;

    const float crystalRotation = static_cast<float>(crystal->innerRotation) + partialTick;
    float bob = MathHelper::sin(crystalRotation * 0.2f) / 2.0f + 0.5f;
    bob = (bob * bob + bob) * 0.2f;
    const float dx = static_cast<float>(crystal->posX - dragon->posX - (dragon->prevPosX - dragon->posX) * static_cast<double>(1.0f - partialTick));
    const float dy = static_cast<float>(static_cast<double>(bob) + crystal->posY - 1.0 - dragon->posY - (dragon->prevPosY - dragon->posY) * static_cast<double>(1.0f - partialTick));
    const float dz = static_cast<float>(crystal->posZ - dragon->posZ - (dragon->prevPosZ - dragon->posZ) * static_cast<double>(1.0f - partialTick));
    const float horizontal = MathHelper::sqrt_float(dx * dx + dz * dz);
    const float distance = MathHelper::sqrt_float(dx * dx + dy * dy + dz * dz);

#if PLATFORM_FLOAT_VERTEX_MATH
    const float beamYaw = -std::atan2(dz, dx);
    const float beamPitch = -std::atan2(horizontal, dy);
#else
    const float beamYaw = static_cast<float>(-JavaMath::atan2(static_cast<double>(dz), static_cast<double>(dx)));
    const float beamPitch = static_cast<float>(-JavaMath::atan2(static_cast<double>(horizontal), static_cast<double>(dy)));
#endif

    renderPushMatrix();
    renderTranslate(static_cast<float>(x), static_cast<float>(y) + 2.0f, static_cast<float>(z));
    renderRotate(beamYaw * 180.0f / kPi - 90.0f, 0.0f, 1.0f, 0.0f);
    renderRotate(beamPitch * 180.0f / kPi - 90.0f, 1.0f, 0.0f, 0.0f);

    Tessellator* tessellator = &Tessellator::instance;
    RenderHelper::disableStandardItemLighting();
    renderDisable(RenderCapability::CullFace);
    loadTexture("/mob/enderdragon/beam.png");
    renderShadeModel(RenderShadeModel::Smooth);
    const float vStart = -((static_cast<float>(dragon->ticksExisted) + partialTick) * 0.01f);
    const float vEnd = distance / 32.0f - (static_cast<float>(dragon->ticksExisted) + partialTick) * 0.01f;
    tessellator->startDrawing(static_cast<int>(RenderPrimitive::TriangleStrip));
    constexpr int sides = 8;
    for (int i = 0; i <= sides; ++i)
    {
        const float radialX = MathHelper::sin(static_cast<float>(i % sides) * kPi * 2.0f / static_cast<float>(sides)) * 0.75f;
        const float radialY = MathHelper::cos(static_cast<float>(i % sides) * kPi * 2.0f / static_cast<float>(sides)) * 0.75f;
        const float u = static_cast<float>(i % sides) / static_cast<float>(sides);
        tessellator->setColorOpaque_I(0);
        tessellator->addVertexWithUV(radialX * 0.2f, radialY * 0.2f, 0.0f, u, vEnd);
        tessellator->setColorOpaque_I(0xFFFFFF);
        tessellator->addVertexWithUV(radialX, radialY, distance, u, vStart);
    }
    tessellator->draw();
    renderEnable(RenderCapability::CullFace);
    renderShadeModel(RenderShadeModel::Flat);
    RenderHelper::enableStandardItemLighting();
    renderPopMatrix();
}

void RenderDragon::renderDragonDying(EntityDragon* dragon, float partialTick)
{
    if (dragon->field_40178_aA <= 0)
        return;

    RenderHelper::disableStandardItemLighting();
    const float progress = (static_cast<float>(dragon->field_40178_aA) + partialTick) / 200.0f;
    float fade = 0.0f;
    if (progress > 0.8f)
        fade = (progress - 0.8f) / 0.2f;

    Random random(432LL);
    renderDisable(RenderCapability::Texture2D);
    renderShadeModel(RenderShadeModel::Smooth);
    renderEnable(RenderCapability::Blend);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::One);
    renderDisable(RenderCapability::AlphaTest);
    renderEnable(RenderCapability::CullFace);
    renderDepthMask(false);
    renderPushMatrix();
    renderTranslate(0.0f, -1.0f, -2.0f);

    Tessellator* tessellator = &Tessellator::instance;
    const float rayCount = (progress + progress * progress) * 0.5f * 60.0f;
    for (int i = 0; static_cast<float>(i) < rayCount; ++i)
    {
        renderRotate(random.nextFloat() * 360.0f, 1.0f, 0.0f, 0.0f);
        renderRotate(random.nextFloat() * 360.0f, 0.0f, 1.0f, 0.0f);
        renderRotate(random.nextFloat() * 360.0f, 0.0f, 0.0f, 1.0f);
        renderRotate(random.nextFloat() * 360.0f, 1.0f, 0.0f, 0.0f);
        renderRotate(random.nextFloat() * 360.0f, 0.0f, 1.0f, 0.0f);
        renderRotate(random.nextFloat() * 360.0f + progress * 90.0f, 0.0f, 0.0f, 1.0f);
        tessellator->startDrawing(static_cast<int>(RenderPrimitive::TriangleFan));
        const float length = random.nextFloat() * 20.0f + 5.0f + fade * 10.0f;
        const float radius = random.nextFloat() * 2.0f + 1.0f + fade * 2.0f;
        tessellator->setColorRGBA_I(0xFFFFFF, static_cast<int>(255.0f * (1.0f - fade)));
        tessellator->addVertex(0.0f, 0.0f, 0.0f);
        tessellator->setColorRGBA_I(0xFF00FF, 0);
        tessellator->addVertex(-0.866f * radius, length, -0.5f * radius);
        tessellator->addVertex(0.866f * radius, length, -0.5f * radius);
        tessellator->addVertex(0.0f, length, radius);
        tessellator->addVertex(-0.866f * radius, length, -0.5f * radius);
        tessellator->draw();
    }

    renderPopMatrix();
    renderDepthMask(true);
    renderDisable(RenderCapability::CullFace);
    renderDisable(RenderCapability::Blend);
    renderShadeModel(RenderShadeModel::Flat);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    renderEnable(RenderCapability::Texture2D);
    renderEnable(RenderCapability::AlphaTest);
    RenderHelper::enableStandardItemLighting();
}

int RenderDragon::shouldRenderPass(EntityLiving* entity, int pass, float partialTick)
{
    (void)partialTick;
    EntityDragon* dragon = static_cast<EntityDragon*>(entity);
    if (pass == 1)
        renderDepthFunc(RenderCompare::LessEqual);
    if (pass != 0)
        return -1;

    loadTexture("/mob/enderdragon/ender_eyes.png");
    renderEnable(RenderCapability::Blend);
    renderDisable(RenderCapability::AlphaTest);
    renderBlendFunc(RenderBlendFactor::One, RenderBlendFactor::One);
    renderDisable(RenderCapability::Lighting);
    renderDepthFunc(RenderCompare::Equal);
    OpenGlHelper::setLightmapTextureCoords(OpenGlHelper::lightmapTexUnit, 0xF0, 0xF0);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    renderEnable(RenderCapability::Lighting);
    (void)dragon;
    return 1;
}

void RenderDragon::rotateCorpse(EntityLiving* entity, float age, float yaw, float partialTick)
{
    rotateDragonBody(static_cast<EntityDragon*>(entity), age, yaw, partialTick);
}

void RenderDragon::renderEquippedItems(EntityLiving* entity, float partialTick)
{
    RenderLiving::renderEquippedItems(entity, partialTick);
    renderDragonDying(static_cast<EntityDragon*>(entity), partialTick);
}

void RenderDragon::renderDragon(EntityDragon* dragon, double x, double y, double z, float yaw, float partialTick)
{
    entityDragon = dragon;
    RenderLiving::doRenderLiving(dragon, x, y, z, yaw, partialTick);
    renderHealingBeam(dragon, x, y, z, partialTick);
}

void RenderDragon::doRenderLiving(EntityLiving* entity, double x, double y, double z, float yaw, float partialTick)
{
    renderDragon(static_cast<EntityDragon*>(entity), x, y, z, yaw, partialTick);
}

void RenderDragon::doRender(Entity* entity, double x, double y, double z, float yaw, float partialTick)
{
    renderDragon(static_cast<EntityDragon*>(entity), x, y, z, yaw, partialTick);
}
