#include "ModelBlaze.h"

#include "MathHelper.h"
#include "ModelRenderer.h"

ModelBlaze::ModelBlaze()
{
    for (int i = 0; i < 12; ++i)
    {
        field_40323_a[i] = new ModelRenderer(this, 0, 16);
        field_40323_a[i]->addBox(0.0f, 0.0f, 0.0f, 2, 8, 2);
    }
    field_40322_b = new ModelRenderer(this, 0, 0);
    field_40322_b->addBox(-4.0f, -4.0f, -4.0f, 8, 8, 8);
}

ModelBlaze::~ModelBlaze()
{
    for (ModelRenderer*& part : field_40323_a)
    {
        delete part;
        part = nullptr;
    }
    delete field_40322_b;
}

int ModelBlaze::func_40321_a() const
{
    return 8;
}

void ModelBlaze::render(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale)
{
    setRotationAngles(limbSwing, limbAmount, age, headYaw, headPitch, scale);
    field_40322_b->render(scale);
    for (ModelRenderer* part : field_40323_a)
        part->render(scale);
}

void ModelBlaze::setRotationAngles(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale)
{
    float angle = age * 3.1415927f * -0.1f;
    int i = 0;
    for (; i < 4; ++i)
    {
        field_40323_a[i]->rotationPointY = -2.0f + MathHelper::cos((static_cast<float>(i * 2) + age) * 0.25f);
        field_40323_a[i]->rotationPointX = MathHelper::cos(angle) * 9.0f;
        field_40323_a[i]->rotationPointZ = MathHelper::sin(angle) * 9.0f;
        angle += 3.1415927f * 0.5f;
    }
    angle = 3.1415927f * 0.25f + age * 3.1415927f * 0.03f;
    for (; i < 8; ++i)
    {
        field_40323_a[i]->rotationPointY = 2.0f + MathHelper::cos((static_cast<float>(i * 2) + age) * 0.25f);
        field_40323_a[i]->rotationPointX = MathHelper::cos(angle) * 7.0f;
        field_40323_a[i]->rotationPointZ = MathHelper::sin(angle) * 7.0f;
        angle += 3.1415927f * 0.5f;
    }
    angle = 3.1415927f * 0.15f + age * 3.1415927f * -0.05f;
    for (; i < 12; ++i)
    {
        field_40323_a[i]->rotationPointY = 11.0f + MathHelper::cos((static_cast<float>(i) * 1.5f + age) * 0.5f);
        field_40323_a[i]->rotationPointX = MathHelper::cos(angle) * 5.0f;
        field_40323_a[i]->rotationPointZ = MathHelper::sin(angle) * 5.0f;
        angle += 3.1415927f * 0.5f;
    }
    field_40322_b->rotateAngleY = headYaw / 57.295776f;
    field_40322_b->rotateAngleX = headPitch / 57.295776f;
}
