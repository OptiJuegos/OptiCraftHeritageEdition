#pragma once

#include "ModelBase.h"

class ModelRenderer;

// net.minecraft.src.ModelIronGolem
class ModelIronGolem : public ModelBase
{
public:
    ModelIronGolem();
    explicit ModelIronGolem(float scale);
    ModelIronGolem(float scale, float yOffset);
    ~ModelIronGolem() override;

    void render(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale) override;
    void setRotationAngles(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale) override;
    void setLivingAnimations(EntityLiving* entity, float limbSwing, float limbAmount, float partialTick) override;

    ModelRenderer* field_48234_a;
    ModelRenderer* field_48232_b;
    ModelRenderer* field_48233_c;
    ModelRenderer* field_48230_d;
    ModelRenderer* field_48231_e;
    ModelRenderer* field_48229_f;

private:
    static float triangleWave(float value, float period);
};
