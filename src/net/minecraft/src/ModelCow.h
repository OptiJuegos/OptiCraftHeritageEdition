#pragma once

#include "ModelQuadruped.h"

class ModelRenderer;

// net.minecraft.src.ModelCow
class ModelCow : public ModelQuadruped
{
public:
	ModelCow();
	~ModelCow() override;

	void render(float f, float f1, float f2, float f3, float f4, float f5) override;
	void setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5) override;

	ModelRenderer *udders;
	ModelRenderer *horn1;
	ModelRenderer *horn2;
};
