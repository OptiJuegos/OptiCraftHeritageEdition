#pragma once

#include "ModelBase.h"

class ModelRenderer;

// net.minecraft.src.ModelBiped
class ModelBiped : public ModelBase
{
public:
	ModelBiped();
	~ModelBiped() override;
	ModelBiped(float f);
	ModelBiped(float f, float f1);

	void render(float f, float f1, float f2, float f3, float f4, float f5) override;
	void setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5) override;
	void renderEars(float f);
	void renderCloak(float f);

	ModelRenderer *bipedHead;
	ModelRenderer *bipedHeadwear;
	ModelRenderer *bipedBody;
	ModelRenderer *bipedRightArm;
	ModelRenderer *bipedLeftArm;
	ModelRenderer *bipedRightLeg;
	ModelRenderer *bipedLeftLeg;
	ModelRenderer *bipedEars;
	ModelRenderer *bipedCloak;
	int heldItemLeft;
	int heldItemRight;
	bool isSneak;
	bool aimedBow;
};
