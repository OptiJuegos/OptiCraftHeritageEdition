#include "ModelSnowMan.h"

#include "MathHelper.h"
#include "ModelRenderer.h"

ModelSnowMan::ModelSnowMan()
{
    const float yOffset = 4.0f;
    const float scale = 0.0f;
    field_40305_c = new ModelRenderer(this, 0, 0);
    field_40305_c->setTextureSize(64, 64)->addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, scale - 0.5f);
    field_40305_c->setRotationPoint(0.0f, yOffset, 0.0f);

    field_40302_d = new ModelRenderer(this, 32, 0);
    field_40302_d->setTextureSize(64, 64)->addBox(-1.0f, 0.0f, -1.0f, 12, 2, 2, scale - 0.5f);
    field_40302_d->setRotationPoint(0.0f, yOffset + 2.0f, 0.0f);

    field_40303_e = new ModelRenderer(this, 32, 0);
    field_40303_e->setTextureSize(64, 64)->addBox(-1.0f, 0.0f, -1.0f, 12, 2, 2, scale - 0.5f);
    field_40303_e->setRotationPoint(0.0f, yOffset + 2.0f, 0.0f);

    field_40306_a = new ModelRenderer(this, 0, 16);
    field_40306_a->setTextureSize(64, 64)->addBox(-5.0f, -10.0f, -5.0f, 10, 10, 10, scale - 0.5f);
    field_40306_a->setRotationPoint(0.0f, yOffset + 9.0f, 0.0f);

    field_40304_b = new ModelRenderer(this, 0, 36);
    field_40304_b->setTextureSize(64, 64)->addBox(-6.0f, -12.0f, -6.0f, 12, 12, 12, scale - 0.5f);
    field_40304_b->setRotationPoint(0.0f, yOffset + 20.0f, 0.0f);
}

ModelSnowMan::~ModelSnowMan()
{
    delete field_40306_a;
    delete field_40304_b;
    delete field_40305_c;
    delete field_40302_d;
    delete field_40303_e;
}

void ModelSnowMan::render(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale)
{
    setRotationAngles(limbSwing, limbAmount, age, headYaw, headPitch, scale);
    field_40306_a->render(scale);
    field_40304_b->render(scale);
    field_40305_c->render(scale);
    field_40302_d->render(scale);
    field_40303_e->render(scale);
}

void ModelSnowMan::setRotationAngles(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale)
{
    field_40305_c->rotateAngleY = headYaw / 57.295776f;
    field_40305_c->rotateAngleX = headPitch / 57.295776f;
    field_40306_a->rotateAngleY = headYaw / 57.295776f * 0.25f;
    const float sinYaw = MathHelper::sin(field_40306_a->rotateAngleY);
    const float cosYaw = MathHelper::cos(field_40306_a->rotateAngleY);
    field_40302_d->rotateAngleZ = 1.0f;
    field_40303_e->rotateAngleZ = -1.0f;
    field_40302_d->rotateAngleY = field_40306_a->rotateAngleY;
    field_40303_e->rotateAngleY = 3.1415927f + field_40306_a->rotateAngleY;
    field_40302_d->rotationPointX = cosYaw * 5.0f;
    field_40302_d->rotationPointZ = -sinYaw * 5.0f;
    field_40303_e->rotationPointX = -cosYaw * 5.0f;
    field_40303_e->rotationPointZ = sinYaw * 5.0f;
}
