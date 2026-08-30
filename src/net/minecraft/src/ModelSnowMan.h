#pragma once

#include "ModelBase.h"

class ModelRenderer;

// net.minecraft.src.ModelSnowMan
class ModelSnowMan : public ModelBase
{
public:
    ModelSnowMan();
    ~ModelSnowMan() override;

    void render(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale) override;
    void setRotationAngles(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale) override;

    ModelRenderer* field_40306_a;
    ModelRenderer* field_40304_b;
    ModelRenderer* field_40305_c;
    ModelRenderer* field_40302_d;
    ModelRenderer* field_40303_e;
};
