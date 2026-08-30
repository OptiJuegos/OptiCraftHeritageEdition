#pragma once

#include "ModelBase.h"

class ModelRenderer;

// net.minecraft.src.ModelChicken
class ModelChicken : public ModelBase
{
public:
	ModelChicken();
	~ModelChicken() override;

	void render(float f, float f1, float f2, float f3, float f4, float f5) override;
	void setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5) override;

	ModelRenderer *head;
	ModelRenderer *body;
	ModelRenderer *rightLeg;
	ModelRenderer *leftLeg;
	ModelRenderer *rightWing;
	ModelRenderer *leftWing;
	ModelRenderer *bill;
	ModelRenderer *chin;
};
