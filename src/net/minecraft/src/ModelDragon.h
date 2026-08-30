#pragma once

#include "ModelBase.h"

class Entity;
class EntityLiving;
class ModelRenderer;

// net.minecraft.src.ModelDragon
class ModelDragon : public ModelBase
{
public:
    explicit ModelDragon(float scale);
    ~ModelDragon() override;

    void setLivingAnimations(EntityLiving* entity, float limbSwing, float limbAmount, float partialTick) override;
    void render(Entity* entity, float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale) override;
    void setRotationAngles(float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale) override;

private:
    float updateRotations(double angle) const;

    ModelRenderer* head;
    ModelRenderer* neck;
    ModelRenderer* jaw;
    ModelRenderer* body;
    ModelRenderer* rearLeg;
    ModelRenderer* frontLeg;
    ModelRenderer* rearLegTip;
    ModelRenderer* frontLegTip;
    ModelRenderer* rearFoot;
    ModelRenderer* frontFoot;
    ModelRenderer* wing;
    ModelRenderer* wingTip;
    float partialTick;
};
