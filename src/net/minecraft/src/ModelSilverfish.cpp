#include "ModelSilverfish.h"

#include <cmath>

#include "MathHelper.h"
#include "ModelRenderer.h"

namespace
{
    constexpr int boxLength[7][3] = {
        {3, 2, 2}, {4, 3, 2}, {6, 4, 3}, {3, 3, 3}, {2, 2, 3}, {2, 1, 2}, {1, 1, 2}
    };
    constexpr int texturePositions[7][2] = {
        {0, 0}, {0, 4}, {0, 9}, {0, 16}, {0, 22}, {11, 0}, {13, 4}
    };
}

ModelSilverfish::ModelSilverfish()
{
    float z = -3.5f;
    for (int i = 0; i < 7; ++i)
    {
        silverfishBodyParts[i] = new ModelRenderer(this, texturePositions[i][0], texturePositions[i][1]);
        silverfishBodyParts[i]->addBox(static_cast<float>(boxLength[i][0]) * -0.5f, 0.0f,
            static_cast<float>(boxLength[i][2]) * -0.5f,
            boxLength[i][0], boxLength[i][1], boxLength[i][2]);
        silverfishBodyParts[i]->setRotationPoint(0.0f, static_cast<float>(24 - boxLength[i][1]), z);
        field_35399_c[i] = z;
        if (i < 6)
            z += static_cast<float>(boxLength[i][2] + boxLength[i + 1][2]) * 0.5f;
    }

    silverfishWings[0] = new ModelRenderer(this, 20, 0);
    silverfishWings[0]->addBox(-5.0f, 0.0f, static_cast<float>(boxLength[2][2]) * -0.5f, 10, 8, boxLength[2][2]);
    silverfishWings[0]->setRotationPoint(0.0f, 16.0f, field_35399_c[2]);
    silverfishWings[1] = new ModelRenderer(this, 20, 11);
    silverfishWings[1]->addBox(-3.0f, 0.0f, static_cast<float>(boxLength[4][2]) * -0.5f, 6, 4, boxLength[4][2]);
    silverfishWings[1]->setRotationPoint(0.0f, 20.0f, field_35399_c[4]);
    silverfishWings[2] = new ModelRenderer(this, 20, 18);
    silverfishWings[2]->addBox(-3.0f, 0.0f, static_cast<float>(boxLength[4][2]) * -0.5f, 6, 5, boxLength[1][2]);
    silverfishWings[2]->setRotationPoint(0.0f, 19.0f, field_35399_c[1]);
}

ModelSilverfish::~ModelSilverfish()
{
    for (ModelRenderer*& part : silverfishBodyParts)
    {
        delete part;
        part = nullptr;
    }
    for (ModelRenderer*& wing : silverfishWings)
    {
        delete wing;
        wing = nullptr;
    }
}

void ModelSilverfish::render(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale)
{
    setRotationAngles(limbSwing, limbAmount, age, headYaw, headPitch, scale);
    for (ModelRenderer* part : silverfishBodyParts)
        part->render(scale);
    for (ModelRenderer* wing : silverfishWings)
        wing->render(scale);
}

void ModelSilverfish::setRotationAngles(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale)
{
    for (int i = 0; i < 7; ++i)
    {
        const float distance = static_cast<float>(std::abs(i - 2));
        silverfishBodyParts[i]->rotateAngleY = MathHelper::cos(age * 0.9f + static_cast<float>(i) * 0.15f * 3.1415927f)
            * 3.1415927f * 0.05f * (1.0f + distance);
        silverfishBodyParts[i]->rotationPointX = MathHelper::sin(age * 0.9f + static_cast<float>(i) * 0.15f * 3.1415927f)
            * 3.1415927f * 0.2f * distance;
    }
    silverfishWings[0]->rotateAngleY = silverfishBodyParts[2]->rotateAngleY;
    silverfishWings[1]->rotateAngleY = silverfishBodyParts[4]->rotateAngleY;
    silverfishWings[1]->rotationPointX = silverfishBodyParts[4]->rotationPointX;
    silverfishWings[2]->rotateAngleY = silverfishBodyParts[1]->rotateAngleY;
    silverfishWings[2]->rotationPointX = silverfishBodyParts[1]->rotationPointX;
}
