#include "ModelMagmaCube.h"

#include "EntityMagmaCube.h"
#include "ModelRenderer.h"

ModelMagmaCube::ModelMagmaCube()
{
    for (int i = 0; i < 8; ++i)
    {
        int textureX = 0;
        int textureY = i;
        if (i == 2)
        {
            textureX = 24;
            textureY = 10;
        }
        else if (i == 3)
        {
            textureX = 24;
            textureY = 19;
        }
        field_40345_a[i] = new ModelRenderer(this, textureX, textureY);
        field_40345_a[i]->addBox(-4.0f, static_cast<float>(16 + i), -4.0f, 8, 1, 8);
    }
    field_40344_b = new ModelRenderer(this, 0, 16);
    field_40344_b->addBox(-2.0f, 18.0f, -2.0f, 4, 4, 4);
}

ModelMagmaCube::~ModelMagmaCube()
{
    for (ModelRenderer*& part : field_40345_a)
    {
        delete part;
        part = nullptr;
    }
    delete field_40344_b;
}

int ModelMagmaCube::func_40343_a() const
{
    return 5;
}

void ModelMagmaCube::setLivingAnimations(EntityLiving* entity, float limbSwing, float limbAmount, float partialTick)
{
    EntityMagmaCube* magmaCube = static_cast<EntityMagmaCube*>(entity);
    float squish = magmaCube->field_767_b + (magmaCube->field_768_a - magmaCube->field_767_b) * partialTick;
    if (squish < 0.0f)
        squish = 0.0f;
    for (int i = 0; i < 8; ++i)
        field_40345_a[i]->rotationPointY = static_cast<float>(-(4 - i)) * squish * 1.7f;
}

void ModelMagmaCube::render(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale)
{
    field_40344_b->render(scale);
    for (ModelRenderer* part : field_40345_a)
        part->render(scale);
}
