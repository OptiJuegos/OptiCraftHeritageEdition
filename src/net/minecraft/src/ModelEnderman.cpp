#include "ModelEnderman.h"

#include "ModelRenderer.h"

ModelEnderman::ModelEnderman() : ModelBiped(0.0f, -14.0f), isCarrying(false), isAttacking(false)
{
    const float yOffset = -14.0f;

    delete bipedHeadwear;
    bipedHeadwear = new ModelRenderer(this, 0, 16);
    bipedHeadwear->addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, -0.5f);
    bipedHeadwear->setRotationPoint(0.0f, yOffset, 0.0f);

    delete bipedBody;
    bipedBody = new ModelRenderer(this, 32, 16);
    bipedBody->addBox(-4.0f, 0.0f, -2.0f, 8, 12, 4, 0.0f);
    bipedBody->setRotationPoint(0.0f, yOffset, 0.0f);

    delete bipedRightArm;
    bipedRightArm = new ModelRenderer(this, 56, 0);
    bipedRightArm->addBox(-1.0f, -2.0f, -1.0f, 2, 30, 2, 0.0f);
    bipedRightArm->setRotationPoint(-3.0f, 2.0f + yOffset, 0.0f);

    delete bipedLeftArm;
    bipedLeftArm = new ModelRenderer(this, 56, 0);
    bipedLeftArm->mirror = true;
    bipedLeftArm->addBox(-1.0f, -2.0f, -1.0f, 2, 30, 2, 0.0f);
    bipedLeftArm->setRotationPoint(5.0f, 2.0f + yOffset, 0.0f);

    delete bipedRightLeg;
    bipedRightLeg = new ModelRenderer(this, 56, 0);
    bipedRightLeg->addBox(-1.0f, 0.0f, -1.0f, 2, 30, 2, 0.0f);
    bipedRightLeg->setRotationPoint(-2.0f, 12.0f + yOffset, 0.0f);

    delete bipedLeftLeg;
    bipedLeftLeg = new ModelRenderer(this, 56, 0);
    bipedLeftLeg->mirror = true;
    bipedLeftLeg->addBox(-1.0f, 0.0f, -1.0f, 2, 30, 2, 0.0f);
    bipedLeftLeg->setRotationPoint(2.0f, 12.0f + yOffset, 0.0f);
}

void ModelEnderman::setRotationAngles(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale)
{
    ModelBiped::setRotationAngles(limbSwing, limbAmount, age, headYaw, headPitch, scale);
    bipedHead->showModel = true;
    const float yOffset = -14.0f;
    bipedBody->rotateAngleX = 0.0f;
    bipedBody->rotationPointY = yOffset;
    bipedBody->rotationPointZ = 0.0f;
    bipedRightArm->rotateAngleX *= 0.5f;
    bipedLeftArm->rotateAngleX *= 0.5f;
    bipedRightLeg->rotateAngleX *= 0.5f;
    bipedLeftLeg->rotateAngleX *= 0.5f;

    const float limit = 0.4f;
    if (bipedRightArm->rotateAngleX > limit) bipedRightArm->rotateAngleX = limit;
    if (bipedLeftArm->rotateAngleX > limit) bipedLeftArm->rotateAngleX = limit;
    if (bipedRightArm->rotateAngleX < -limit) bipedRightArm->rotateAngleX = -limit;
    if (bipedLeftArm->rotateAngleX < -limit) bipedLeftArm->rotateAngleX = -limit;
    if (bipedRightLeg->rotateAngleX > limit) bipedRightLeg->rotateAngleX = limit;
    if (bipedLeftLeg->rotateAngleX > limit) bipedLeftLeg->rotateAngleX = limit;
    if (bipedRightLeg->rotateAngleX < -limit) bipedRightLeg->rotateAngleX = -limit;
    if (bipedLeftLeg->rotateAngleX < -limit) bipedLeftLeg->rotateAngleX = -limit;

    if (isCarrying)
    {
        bipedRightArm->rotateAngleX = -0.5f;
        bipedLeftArm->rotateAngleX = -0.5f;
        bipedRightArm->rotateAngleZ = 0.05f;
        bipedLeftArm->rotateAngleZ = -0.05f;
    }

    bipedRightArm->rotationPointZ = 0.0f;
    bipedLeftArm->rotationPointZ = 0.0f;
    bipedRightLeg->rotationPointZ = 0.0f;
    bipedLeftLeg->rotationPointZ = 0.0f;
    bipedRightLeg->rotationPointY = 9.0f + yOffset;
    bipedLeftLeg->rotationPointY = 9.0f + yOffset;
    bipedHead->rotationPointZ = 0.0f;
    bipedHead->rotationPointY = yOffset + 1.0f;
    bipedHeadwear->rotationPointX = bipedHead->rotationPointX;
    bipedHeadwear->rotationPointY = bipedHead->rotationPointY;
    bipedHeadwear->rotationPointZ = bipedHead->rotationPointZ;
    bipedHeadwear->rotateAngleX = bipedHead->rotateAngleX;
    bipedHeadwear->rotateAngleY = bipedHead->rotateAngleY;
    bipedHeadwear->rotateAngleZ = bipedHead->rotateAngleZ;
    if (isAttacking)
        bipedHead->rotationPointY -= 5.0f;
}
