#pragma once

#include "ModelBase.h"

class ModelRenderer;

// net.minecraft.src.ModelSquid
class ModelSquid : public ModelBase
{
public:
	ModelSquid();
	~ModelSquid() override;

	void setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5) override;
	void render(float f, float f1, float f2, float f3, float f4, float f5) override;

	ModelRenderer *squidBody;
	ModelRenderer *squidTentacles[8];
};
