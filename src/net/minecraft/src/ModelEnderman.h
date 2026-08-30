#pragma once

#include "ModelBiped.h"

// net.minecraft.src.ModelEnderman
class ModelEnderman : public ModelBiped
{
public:
    ModelEnderman();
    void setRotationAngles(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale) override;

    bool isCarrying;
    bool isAttacking;
};
