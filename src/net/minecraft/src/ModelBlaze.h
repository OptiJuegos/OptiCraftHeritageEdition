#pragma once

#include "ModelBase.h"

class ModelRenderer;

// net.minecraft.src.ModelBlaze
class ModelBlaze : public ModelBase
{
public:
    ModelBlaze();
    ~ModelBlaze() override;

    int func_40321_a() const;
    void render(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale) override;
    void setRotationAngles(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale) override;

private:
    ModelRenderer* field_40323_a[12];
    ModelRenderer* field_40322_b;
};
