#include "ModelDragon.h"
#include "java/Math.h"

#include <cmath>

#include "EntityDragon.h"
#include "ModelRenderer.h"
#include "platform/RenderAPI.h"
#include "platform/PlatformTuning.h"

namespace
{
constexpr float kPi = 3.14159265358979323846f;

#if PLATFORM_FLOAT_VERTEX_MATH
using model_math_t = float;
#else
using model_math_t = double;
#endif

model_math_t modelSin(model_math_t value)
{
#if PLATFORM_FLOAT_VERTEX_MATH
    return std::sin(value);
#else
    return JavaMath::sin(value);
#endif
}

model_math_t modelCos(model_math_t value)
{
#if PLATFORM_FLOAT_VERTEX_MATH
    return std::cos(value);
#else
    return JavaMath::cos(value);
#endif
}
}

ModelDragon::ModelDragon(float scale) :
    head(nullptr),
    neck(nullptr),
    jaw(nullptr),
    body(nullptr),
    rearLeg(nullptr),
    frontLeg(nullptr),
    rearLegTip(nullptr),
    frontLegTip(nullptr),
    rearFoot(nullptr),
    frontFoot(nullptr),
    wing(nullptr),
    wingTip(nullptr),
    partialTick(0.0f)
{
    (void)scale;
    textureWidth = 256;
    textureHeight = 256;
    setTextureOffset("body.body", 0, 0);
    setTextureOffset("wing.skin", -56, 88);
    setTextureOffset("wingtip.skin", -56, 144);
    setTextureOffset("rearleg.main", 0, 0);
    setTextureOffset("rearfoot.main", 112, 0);
    setTextureOffset("rearlegtip.main", 196, 0);
    setTextureOffset("head.upperhead", 112, 30);
    setTextureOffset("wing.bone", 112, 88);
    setTextureOffset("head.upperlip", 176, 44);
    setTextureOffset("jaw.jaw", 176, 65);
    setTextureOffset("frontleg.main", 112, 104);
    setTextureOffset("wingtip.bone", 112, 136);
    setTextureOffset("frontfoot.main", 144, 104);
    setTextureOffset("neck.box", 192, 104);
    setTextureOffset("frontlegtip.main", 226, 138);
    setTextureOffset("body.scale", 220, 53);
    setTextureOffset("head.scale", 0, 0);
    setTextureOffset("neck.scale", 48, 0);
    setTextureOffset("head.nostril", 112, 0);

    const float headOffset = -16.0f;
    head = new ModelRenderer(this, "head");
    head->addBox("upperlip", -6.0f, -1.0f, -8.0f + headOffset, 12, 5, 16);
    head->addBox("upperhead", -8.0f, -8.0f, 6.0f + headOffset, 16, 16, 16);
    head->mirror = true;
    head->addBox("scale", -5.0f, -12.0f, 12.0f + headOffset, 2, 4, 6);
    head->addBox("nostril", -5.0f, -3.0f, -6.0f + headOffset, 2, 2, 4);
    head->mirror = false;
    head->addBox("scale", 3.0f, -12.0f, 12.0f + headOffset, 2, 4, 6);
    head->addBox("nostril", 3.0f, -3.0f, -6.0f + headOffset, 2, 2, 4);

    jaw = new ModelRenderer(this, "jaw");
    jaw->setRotationPoint(0.0f, 4.0f, 8.0f + headOffset);
    jaw->addBox("jaw", -6.0f, 0.0f, -16.0f, 12, 4, 16);
    head->addChild(jaw);

    neck = new ModelRenderer(this, "neck");
    neck->addBox("box", -5.0f, -5.0f, -5.0f, 10, 10, 10);
    neck->addBox("scale", -1.0f, -9.0f, -3.0f, 2, 4, 6);

    body = new ModelRenderer(this, "body");
    body->setRotationPoint(0.0f, 4.0f, 8.0f);
    body->addBox("body", -12.0f, 0.0f, -16.0f, 24, 24, 64);
    body->addBox("scale", -1.0f, -6.0f, -10.0f, 2, 6, 12);
    body->addBox("scale", -1.0f, -6.0f, 10.0f, 2, 6, 12);
    body->addBox("scale", -1.0f, -6.0f, 30.0f, 2, 6, 12);

    wing = new ModelRenderer(this, "wing");
    wing->setRotationPoint(-12.0f, 5.0f, 2.0f);
    wing->addBox("bone", -56.0f, -4.0f, -4.0f, 56, 8, 8);
    wing->addBox("skin", -56.0f, 0.0f, 2.0f, 56, 0, 56);

    wingTip = new ModelRenderer(this, "wingtip");
    wingTip->setRotationPoint(-56.0f, 0.0f, 0.0f);
    wingTip->addBox("bone", -56.0f, -2.0f, -2.0f, 56, 4, 4);
    wingTip->addBox("skin", -56.0f, 0.0f, 2.0f, 56, 0, 56);
    wing->addChild(wingTip);

    frontLeg = new ModelRenderer(this, "frontleg");
    frontLeg->setRotationPoint(-12.0f, 20.0f, 2.0f);
    frontLeg->addBox("main", -4.0f, -4.0f, -4.0f, 8, 24, 8);

    frontLegTip = new ModelRenderer(this, "frontlegtip");
    frontLegTip->setRotationPoint(0.0f, 20.0f, -1.0f);
    frontLegTip->addBox("main", -3.0f, -1.0f, -3.0f, 6, 24, 6);
    frontLeg->addChild(frontLegTip);

    frontFoot = new ModelRenderer(this, "frontfoot");
    frontFoot->setRotationPoint(0.0f, 23.0f, 0.0f);
    frontFoot->addBox("main", -4.0f, 0.0f, -12.0f, 8, 4, 16);
    frontLegTip->addChild(frontFoot);

    rearLeg = new ModelRenderer(this, "rearleg");
    rearLeg->setRotationPoint(-16.0f, 16.0f, 42.0f);
    rearLeg->addBox("main", -8.0f, -4.0f, -8.0f, 16, 32, 16);

    rearLegTip = new ModelRenderer(this, "rearlegtip");
    rearLegTip->setRotationPoint(0.0f, 32.0f, -4.0f);
    rearLegTip->addBox("main", -6.0f, -2.0f, 0.0f, 12, 32, 12);
    rearLeg->addChild(rearLegTip);

    rearFoot = new ModelRenderer(this, "rearfoot");
    rearFoot->setRotationPoint(0.0f, 31.0f, 4.0f);
    rearFoot->addBox("main", -9.0f, 0.0f, -20.0f, 18, 6, 24);
    rearLegTip->addChild(rearFoot);
}

ModelDragon::~ModelDragon()
{
    delete rearFoot;
    delete rearLegTip;
    delete rearLeg;
    delete frontFoot;
    delete frontLegTip;
    delete frontLeg;
    delete wingTip;
    delete wing;
    delete body;
    delete neck;
    delete jaw;
    delete head;
}

void ModelDragon::setLivingAnimations(EntityLiving* entity, float limbSwing, float limbAmount, float tick)
{
    (void)entity;
    (void)limbSwing;
    (void)limbAmount;
    partialTick = tick;
}

void ModelDragon::render(Entity* entity, float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale)
{
    (void)limbSwing;
    (void)limbAmount;
    (void)age;
    (void)headYaw;
    (void)headPitch;

    EntityDragon* dragon = static_cast<EntityDragon*>(entity);
    renderPushMatrix();

    const float anim = dragon->field_40173_aw + (dragon->field_40172_ax - dragon->field_40173_aw) * partialTick;
    jaw->rotateAngleX = static_cast<float>(
        modelSin(static_cast<model_math_t>(anim * kPi * 2.0f)) + static_cast<model_math_t>(1.0)) * 0.2f;
    float bob = static_cast<float>(
        modelSin(static_cast<model_math_t>(anim * kPi * 2.0f - 1.0f)) + static_cast<model_math_t>(1.0));
    bob = (bob * bob + bob * 2.0f) * 0.05f;
    renderTranslate(0.0f, bob - 2.0f, -3.0f);
    renderRotate(bob * 2.0f, 1.0f, 0.0f, 0.0f);

    float neckY = -30.0f;
    float neckX = 0.0f;
    const float bendScale = 1.5f;
    const std::array<double, 3> baseMovement = dragon->func_40160_a(6, partialTick);
    const float bodyYawDelta = updateRotations(dragon->func_40160_a(5, partialTick)[0] - dragon->func_40160_a(10, partialTick)[0]);
    const float bodyYaw = updateRotations(dragon->func_40160_a(5, partialTick)[0] + static_cast<double>(bodyYawDelta / 2.0f));
    neckY += 2.0f;
    const float animPhase = anim * kPi * 2.0f;
    neckY = 20.0f;
    float neckZ = -12.0f;

    for (int i = 0; i < 5; ++i)
    {
        const std::array<double, 3> movement = dragon->func_40160_a(5 - i, partialTick);
        const float wave = static_cast<float>(
            modelCos(static_cast<model_math_t>(static_cast<float>(i) * 0.45f + animPhase))) * 0.15f;
        neck->rotateAngleY = updateRotations(movement[0] - baseMovement[0]) * kPi / 180.0f * bendScale;
        neck->rotateAngleX = wave + static_cast<float>(movement[1] - baseMovement[1]) * kPi / 180.0f * bendScale * 5.0f;
        neck->rotateAngleZ = -updateRotations(movement[0] - static_cast<double>(bodyYaw)) * kPi / 180.0f * bendScale;
        neck->rotationPointY = neckY;
        neck->rotationPointZ = neckZ;
        neck->rotationPointX = neckX;
        neckY = static_cast<float>(static_cast<model_math_t>(neckY)
            + modelSin(static_cast<model_math_t>(neck->rotateAngleX)) * static_cast<model_math_t>(10.0));
        neckZ = static_cast<float>(static_cast<model_math_t>(neckZ)
            - modelCos(static_cast<model_math_t>(neck->rotateAngleY))
                * modelCos(static_cast<model_math_t>(neck->rotateAngleX))
                * static_cast<model_math_t>(10.0));
        neckX = static_cast<float>(static_cast<model_math_t>(neckX)
            - modelSin(static_cast<model_math_t>(neck->rotateAngleY))
                * modelCos(static_cast<model_math_t>(neck->rotateAngleX))
                * static_cast<model_math_t>(10.0));
        neck->render(scale);
    }

    head->rotationPointY = neckY;
    head->rotationPointZ = neckZ;
    head->rotationPointX = neckX;
    const std::array<double, 3> headMovement = dragon->func_40160_a(0, partialTick);
    head->rotateAngleY = updateRotations(headMovement[0] - baseMovement[0]) * kPi / 180.0f;
    head->rotateAngleZ = -updateRotations(headMovement[0] - static_cast<double>(bodyYaw)) * kPi / 180.0f;
    head->render(scale);

    renderPushMatrix();
    renderTranslate(0.0f, 1.0f, 0.0f);
    renderRotate(-bodyYawDelta * bendScale, 0.0f, 0.0f, 1.0f);
    renderTranslate(0.0f, -1.0f, 0.0f);
    body->rotateAngleZ = 0.0f;
    body->render(scale);

    for (int side = 0; side < 2; ++side)
    {
        renderEnable(RenderCapability::CullFace);
        const float wingPhase = anim * kPi * 2.0f;
        wing->rotateAngleX = 0.125f - static_cast<float>(modelCos(static_cast<model_math_t>(wingPhase))) * 0.2f;
        wing->rotateAngleY = 0.25f;
        wing->rotateAngleZ = static_cast<float>(modelSin(static_cast<model_math_t>(wingPhase)) + static_cast<model_math_t>(0.125)) * 0.8f;
        wingTip->rotateAngleZ = -static_cast<float>(
            modelSin(static_cast<model_math_t>(wingPhase + 2.0f)) + static_cast<model_math_t>(0.5)) * 0.75f;
        rearLeg->rotateAngleX = 1.0f + bob * 0.1f;
        rearLegTip->rotateAngleX = 0.5f + bob * 0.1f;
        rearFoot->rotateAngleX = 0.75f + bob * 0.1f;
        frontLeg->rotateAngleX = 1.3f + bob * 0.1f;
        frontLegTip->rotateAngleX = -0.5f - bob * 0.1f;
        frontFoot->rotateAngleX = 0.75f + bob * 0.1f;
        wing->render(scale);
        frontLeg->render(scale);
        rearLeg->render(scale);
        renderScale(-1.0f, 1.0f, 1.0f);
        if (side == 0)
            renderCullFace(RenderFace::Front);
    }

    renderPopMatrix();
    renderCullFace(RenderFace::Back);
    renderDisable(RenderCapability::CullFace);

    float tailWave = -static_cast<float>(modelSin(static_cast<model_math_t>(anim * kPi * 2.0f))) * 0.0f;
    neckY = 10.0f;
    neckZ = 60.0f;
    neckX = 0.0f;
    const std::array<double, 3> tailBase = dragon->func_40160_a(11, partialTick);

    for (int i = 0; i < 12; ++i)
    {
        const std::array<double, 3> movement = dragon->func_40160_a(12 + i, partialTick);
        tailWave = static_cast<float>(static_cast<model_math_t>(tailWave)
            + modelSin(static_cast<model_math_t>(static_cast<float>(i) * 0.45f + animPhase))
                * static_cast<model_math_t>(0.05f));
        neck->rotateAngleY = (updateRotations(movement[0] - tailBase[0]) * bendScale + 180.0f) * kPi / 180.0f;
        neck->rotateAngleX = tailWave + static_cast<float>(movement[1] - tailBase[1]) * kPi / 180.0f * bendScale * 5.0f;
        neck->rotateAngleZ = updateRotations(movement[0] - static_cast<double>(bodyYaw)) * kPi / 180.0f * bendScale;
        neck->rotationPointY = neckY;
        neck->rotationPointZ = neckZ;
        neck->rotationPointX = neckX;
        neckY = static_cast<float>(static_cast<model_math_t>(neckY)
            + modelSin(static_cast<model_math_t>(neck->rotateAngleX)) * static_cast<model_math_t>(10.0));
        neckZ = static_cast<float>(static_cast<model_math_t>(neckZ)
            - modelCos(static_cast<model_math_t>(neck->rotateAngleY))
                * modelCos(static_cast<model_math_t>(neck->rotateAngleX))
                * static_cast<model_math_t>(10.0));
        neckX = static_cast<float>(static_cast<model_math_t>(neckX)
            - modelSin(static_cast<model_math_t>(neck->rotateAngleY))
                * modelCos(static_cast<model_math_t>(neck->rotateAngleX))
                * static_cast<model_math_t>(10.0));
        neck->render(scale);
    }

    renderPopMatrix();
}

void ModelDragon::setRotationAngles(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale)
{
    ModelBase::setRotationAngles(limbSwing, limbAmount, age, headYaw, headPitch, scale);
}

float ModelDragon::updateRotations(double angle) const
{
    while (angle >= 180.0)
        angle -= 360.0;
    while (angle < -180.0)
        angle += 360.0;
    return static_cast<float>(angle);
}
