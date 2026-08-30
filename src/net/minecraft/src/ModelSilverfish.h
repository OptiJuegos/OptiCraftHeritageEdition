#pragma once

#include "ModelBase.h"

class ModelRenderer;

// net.minecraft.src.ModelSilverfish
class ModelSilverfish : public ModelBase
{
public:
    ModelSilverfish();
    ~ModelSilverfish() override;

    void render(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale) override;
    void setRotationAngles(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale) override;

private:
    ModelRenderer* silverfishBodyParts[7];
    ModelRenderer* silverfishWings[3];
    float field_35399_c[7];
};
