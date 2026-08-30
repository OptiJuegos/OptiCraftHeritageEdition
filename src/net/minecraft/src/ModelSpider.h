#pragma once

#include "ModelBase.h"

class ModelRenderer;

// net.minecraft.src.ModelSpider
class ModelSpider : public ModelBase
{
public:
	ModelSpider();
	~ModelSpider() override;

	void render(float f, float f1, float f2, float f3, float f4, float f5) override;
	void setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5) override;

	ModelRenderer *spiderHead;
	ModelRenderer *spiderNeck;
	ModelRenderer *spiderBody;
	ModelRenderer *spiderLeg1;
	ModelRenderer *spiderLeg2;
	ModelRenderer *spiderLeg3;
	ModelRenderer *spiderLeg4;
	ModelRenderer *spiderLeg5;
	ModelRenderer *spiderLeg6;
	ModelRenderer *spiderLeg7;
	ModelRenderer *spiderLeg8;
};
