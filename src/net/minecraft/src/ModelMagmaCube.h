#pragma once

#include "ModelBase.h"

class ModelRenderer;

// net.minecraft.src.ModelMagmaCube
class ModelMagmaCube : public ModelBase
{
public:
    ModelMagmaCube();
    ~ModelMagmaCube() override;

    int func_40343_a() const;
    void render(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale) override;
    void setLivingAnimations(EntityLiving* entity, float limbSwing, float limbAmount, float partialTick) override;

private:
    ModelRenderer* field_40345_a[8];
    ModelRenderer* field_40344_b;
};
