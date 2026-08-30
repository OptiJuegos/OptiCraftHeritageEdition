#pragma once

#include "ModelBase.h"

class EntityLiving;
class EntityWolf;
class ModelRenderer;

// net.minecraft.src.ModelWolf
class ModelWolf : public ModelBase
{
public:
	ModelWolf();
	~ModelWolf() override;

	void render(float f, float f1, float f2, float f3, float f4, float f5) override;
	void setLivingAnimations(EntityLiving *entityliving, float f, float f1, float f2) override;
	void setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5) override;

	ModelRenderer *wolfHeadMain;
	ModelRenderer *wolfBody;
	ModelRenderer *wolfLeg1;
	ModelRenderer *wolfLeg2;
	ModelRenderer *wolfLeg3;
	ModelRenderer *wolfLeg4;
	ModelRenderer *wolfRightEar;
	ModelRenderer *wolfLeftEar;
	ModelRenderer *wolfSnout;
	ModelRenderer *wolfTail;
	ModelRenderer *wolfMane;
};
