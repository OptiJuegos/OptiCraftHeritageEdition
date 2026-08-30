#include "ModelVillager.h"

#include "MathHelper.h"
#include "ModelRenderer.h"

ModelVillager::ModelVillager(float scale) : ModelVillager(scale, 0.0f)
{
}

ModelVillager::ModelVillager(float scale, float yOffset)
{
    field_40340_a = new ModelRenderer(this);
    field_40340_a->setTextureSize(64, 64)->setTextureOffset(0, 0)->addBox(-4.0f, -10.0f, -4.0f, 8, 10, 8, scale);
    field_40340_a->setTextureOffset(24, 0)->addBox(-1.0f, -3.0f, -6.0f, 2, 4, 2, scale);
    field_40340_a->setRotationPoint(0.0f, yOffset, 0.0f);

    field_40338_b = new ModelRenderer(this);
    field_40338_b->setTextureSize(64, 64)->setTextureOffset(16, 20)->addBox(-4.0f, 0.0f, -3.0f, 8, 12, 6, scale);
    field_40338_b->setTextureOffset(0, 38)->addBox(-4.0f, 0.0f, -3.0f, 8, 18, 6, scale + 0.5f);
    field_40338_b->setRotationPoint(0.0f, yOffset, 0.0f);

    field_40339_c = new ModelRenderer(this);
    field_40339_c->setTextureSize(64, 64)->setTextureOffset(44, 22)->addBox(-8.0f, -2.0f, -2.0f, 4, 8, 4, scale);
    field_40339_c->setTextureOffset(44, 22)->addBox(4.0f, -2.0f, -2.0f, 4, 8, 4, scale);
    field_40339_c->setTextureOffset(40, 38)->addBox(-4.0f, 2.0f, -2.0f, 8, 4, 4, scale);
    field_40339_c->setRotationPoint(0.0f, yOffset + 2.0f, 0.0f);

    field_40336_d = new ModelRenderer(this, 0, 22);
    field_40336_d->setTextureSize(64, 64)->addBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, scale);
    field_40336_d->setRotationPoint(-2.0f, 12.0f + yOffset, 0.0f);

    field_40337_e = new ModelRenderer(this, 0, 22);
    field_40337_e->setTextureSize(64, 64);
    field_40337_e->mirror = true;
    field_40337_e->addBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, scale);
    field_40337_e->setRotationPoint(2.0f, 12.0f + yOffset, 0.0f);
}

ModelVillager::~ModelVillager()
{
    delete field_40340_a;
    delete field_40338_b;
    delete field_40339_c;
    delete field_40336_d;
    delete field_40337_e;
}

void ModelVillager::render(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale)
{
    setRotationAngles(limbSwing, limbAmount, age, headYaw, headPitch, scale);
    field_40340_a->render(scale);
    field_40338_b->render(scale);
    field_40336_d->render(scale);
    field_40337_e->render(scale);
    field_40339_c->render(scale);
}

void ModelVillager::setRotationAngles(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale)
{
    field_40340_a->rotateAngleY = headYaw / 57.295776f;
    field_40340_a->rotateAngleX = headPitch / 57.295776f;
    field_40339_c->rotationPointY = 3.0f;
    field_40339_c->rotationPointZ = -1.0f;
    field_40339_c->rotateAngleX = -0.75f;
    field_40336_d->rotateAngleX = MathHelper::cos(limbSwing * 0.6662f) * 1.4f * limbAmount * 0.5f;
    field_40337_e->rotateAngleX = MathHelper::cos(limbSwing * 0.6662f + 3.1415927f) * 1.4f * limbAmount * 0.5f;
    field_40336_d->rotateAngleY = 0.0f;
    field_40337_e->rotateAngleY = 0.0f;
}
