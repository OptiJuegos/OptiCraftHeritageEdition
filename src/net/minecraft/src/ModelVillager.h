#pragma once

#include "ModelBase.h"

class ModelRenderer;

// net.minecraft.src.ModelVillager
class ModelVillager : public ModelBase
{
public:
    explicit ModelVillager(float scale = 0.0f);
    ModelVillager(float scale, float yOffset);
    ~ModelVillager() override;

    void render(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale) override;
    void setRotationAngles(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale) override;

    ModelRenderer* field_40340_a;
    ModelRenderer* field_40338_b;
    ModelRenderer* field_40339_c;
    ModelRenderer* field_40336_d;
    ModelRenderer* field_40337_e;
};
