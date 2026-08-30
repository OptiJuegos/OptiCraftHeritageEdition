#include "ModelLargeChest.h"

#include "ModelRenderer.h"

ModelLargeChest::ModelLargeChest()
{
    delete chestLid;
    delete chestKnob;
    delete chestBelow;

    chestLid = new ModelRenderer(this, 0, 0);
    chestLid->setTextureSize(128, 64)->addBox(0.0f, -5.0f, -14.0f, 30, 5, 14, 0.0f);
    chestLid->rotationPointX = 1.0f;
    chestLid->rotationPointY = 7.0f;
    chestLid->rotationPointZ = 15.0f;

    chestKnob = new ModelRenderer(this, 0, 0);
    chestKnob->setTextureSize(128, 64)->addBox(-1.0f, -2.0f, -15.0f, 2, 4, 1, 0.0f);
    chestKnob->rotationPointX = 16.0f;
    chestKnob->rotationPointY = 7.0f;
    chestKnob->rotationPointZ = 15.0f;

    chestBelow = new ModelRenderer(this, 0, 19);
    chestBelow->setTextureSize(128, 64)->addBox(0.0f, 0.0f, 0.0f, 30, 10, 14, 0.0f);
    chestBelow->rotationPointX = 1.0f;
    chestBelow->rotationPointY = 6.0f;
    chestBelow->rotationPointZ = 1.0f;
}
