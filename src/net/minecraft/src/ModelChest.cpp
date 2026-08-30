#include "ModelChest.h"

#include "ModelRenderer.h"

ModelChest::ModelChest()
{
    chestLid = new ModelRenderer(this, 0, 0);
    chestLid->setTextureSize(64, 64)->addBox(0.0f, -5.0f, -14.0f, 14, 5, 14, 0.0f);
    chestLid->rotationPointX = 1.0f;
    chestLid->rotationPointY = 7.0f;
    chestLid->rotationPointZ = 15.0f;

    chestKnob = new ModelRenderer(this, 0, 0);
    chestKnob->setTextureSize(64, 64)->addBox(-1.0f, -2.0f, -15.0f, 2, 4, 1, 0.0f);
    chestKnob->rotationPointX = 8.0f;
    chestKnob->rotationPointY = 7.0f;
    chestKnob->rotationPointZ = 15.0f;

    chestBelow = new ModelRenderer(this, 0, 19);
    chestBelow->setTextureSize(64, 64)->addBox(0.0f, 0.0f, 0.0f, 14, 10, 14, 0.0f);
    chestBelow->rotationPointX = 1.0f;
    chestBelow->rotationPointY = 6.0f;
    chestBelow->rotationPointZ = 1.0f;
}

ModelChest::~ModelChest()
{
    delete chestLid;
    delete chestBelow;
    delete chestKnob;
}

void ModelChest::renderAll()
{
    chestKnob->rotateAngleX = chestLid->rotateAngleX;
    chestLid->render(1.0f / 16.0f);
    chestKnob->render(1.0f / 16.0f);
    chestBelow->render(1.0f / 16.0f);
}
