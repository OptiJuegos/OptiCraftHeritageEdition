#pragma once

#include "ModelBase.h"

class ModelRenderer;

// net.minecraft.src.ModelCreeper
class ModelCreeper : public ModelBase
{
public:
	ModelCreeper();
	~ModelCreeper() override;
	ModelCreeper(float f);

	void render(float f, float f1, float f2, float f3, float f4, float f5) override;
	void setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5) override;

	ModelRenderer *head;
	ModelRenderer *creeperAura;
	ModelRenderer *body;
	ModelRenderer *leg1;
	ModelRenderer *leg2;
	ModelRenderer *leg3;
	ModelRenderer *leg4;
};
