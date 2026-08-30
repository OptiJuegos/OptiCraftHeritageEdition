#include "ModelIronGolem.h"

#include <cmath>

#include "EntityIronGolem.h"
#include "ModelRenderer.h"

ModelIronGolem::ModelIronGolem() : ModelIronGolem(0.0f)
{
}

ModelIronGolem::ModelIronGolem(float scale) : ModelIronGolem(scale, -7.0f)
{
}

ModelIronGolem::ModelIronGolem(float scale, float yOffset)
{
    field_48234_a = new ModelRenderer(this);
    field_48234_a->setTextureSize(128, 128)->setTextureOffset(0, 0)->addBox(-4.0f, -12.0f, -5.5f, 8, 10, 8, scale);
    field_48234_a->setTextureOffset(24, 0)->addBox(-1.0f, -5.0f, -7.5f, 2, 4, 2, scale);
    field_48234_a->setRotationPoint(0.0f, yOffset, -2.0f);

    field_48232_b = new ModelRenderer(this);
    field_48232_b->setTextureSize(128, 128)->setTextureOffset(0, 40)->addBox(-9.0f, -2.0f, -6.0f, 18, 12, 11, scale);
    field_48232_b->setTextureOffset(0, 70)->addBox(-4.5f, 10.0f, -3.0f, 9, 5, 6, scale + 0.5f);
    field_48232_b->setRotationPoint(0.0f, yOffset, 0.0f);

    field_48233_c = new ModelRenderer(this);
    field_48233_c->setTextureSize(128, 128)->setTextureOffset(60, 21)->addBox(-13.0f, -2.5f, -3.0f, 4, 30, 6, scale);
    field_48233_c->setRotationPoint(0.0f, -7.0f, 0.0f);

    field_48230_d = new ModelRenderer(this);
    field_48230_d->setTextureSize(128, 128)->setTextureOffset(60, 58)->addBox(9.0f, -2.5f, -3.0f, 4, 30, 6, scale);
    field_48230_d->setRotationPoint(0.0f, -7.0f, 0.0f);

    field_48231_e = new ModelRenderer(this, 0, 22);
    field_48231_e->setTextureSize(128, 128)->setTextureOffset(37, 0)->addBox(-3.5f, -3.0f, -3.0f, 6, 16, 5, scale);
    field_48231_e->setRotationPoint(-4.0f, 18.0f + yOffset, 0.0f);

    field_48229_f = new ModelRenderer(this, 0, 22);
    field_48229_f->setTextureSize(128, 128);
    field_48229_f->mirror = true;
    field_48229_f->setTextureOffset(60, 0)->addBox(-3.5f, -3.0f, -3.0f, 6, 16, 5, scale);
    field_48229_f->setRotationPoint(5.0f, 18.0f + yOffset, 0.0f);
}

ModelIronGolem::~ModelIronGolem()
{
    delete field_48234_a;
    delete field_48232_b;
    delete field_48233_c;
    delete field_48230_d;
    delete field_48231_e;
    delete field_48229_f;
}

void ModelIronGolem::render(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale)
{
    setRotationAngles(limbSwing, limbAmount, age, headYaw, headPitch, scale);
    field_48234_a->render(scale);
    field_48232_b->render(scale);
    field_48231_e->render(scale);
    field_48229_f->render(scale);
    field_48233_c->render(scale);
    field_48230_d->render(scale);
}

void ModelIronGolem::setRotationAngles(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale)
{
    field_48234_a->rotateAngleY = headYaw / 57.295776f;
    field_48234_a->rotateAngleX = headPitch / 57.295776f;
    field_48231_e->rotateAngleX = -1.5f * triangleWave(limbSwing, 13.0f) * limbAmount;
    field_48229_f->rotateAngleX = 1.5f * triangleWave(limbSwing, 13.0f) * limbAmount;
    field_48231_e->rotateAngleY = 0.0f;
    field_48229_f->rotateAngleY = 0.0f;
}

void ModelIronGolem::setLivingAnimations(EntityLiving* entity, float limbSwing, float limbAmount, float partialTick)
{
    EntityIronGolem* golem = static_cast<EntityIronGolem*>(entity);
    const int attackTimer = golem->getAttackTimer();
    if (attackTimer > 0)
    {
        field_48233_c->rotateAngleX = -2.0f + 1.5f * triangleWave(static_cast<float>(attackTimer) - partialTick, 10.0f);
        field_48230_d->rotateAngleX = field_48233_c->rotateAngleX;
        return;
    }

    const int roseTick = golem->getHoldRoseTick();
    if (roseTick > 0)
    {
        field_48233_c->rotateAngleX = -0.8f + 0.025f * triangleWave(static_cast<float>(roseTick), 70.0f);
        field_48230_d->rotateAngleX = 0.0f;
        return;
    }

    field_48233_c->rotateAngleX = (-0.2f + 1.5f * triangleWave(limbSwing, 13.0f)) * limbAmount;
    field_48230_d->rotateAngleX = (-0.2f - 1.5f * triangleWave(limbSwing, 13.0f)) * limbAmount;
}

float ModelIronGolem::triangleWave(float value, float period)
{
    return (std::fabs(std::fmod(value, period) - period * 0.5f) - period * 0.25f) / (period * 0.25f);
}
