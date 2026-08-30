#pragma once

#include "ModelBase.h"

class EntityLiving;
class ModelRenderer;

// net.minecraft.src.ModelOcelot
class ModelOcelot : public ModelBase
{
public:
	ModelOcelot();
	~ModelOcelot() override;

	void render(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale) override;
	void setRotationAngles(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale) override;
	void setLivingAnimations(EntityLiving *entity, float limbSwing, float limbSwingAmount, float partialTicks) override;

private:
	ModelRenderer *backLeftLeg;
	ModelRenderer *backRightLeg;
	ModelRenderer *frontLeftLeg;
	ModelRenderer *frontRightLeg;
	ModelRenderer *tailBase;
	ModelRenderer *tailTip;
	ModelRenderer *head;
	ModelRenderer *nose;
	ModelRenderer *earLeft;
	ModelRenderer *earRight;
	ModelRenderer *body;
	int_t animationState;
};
