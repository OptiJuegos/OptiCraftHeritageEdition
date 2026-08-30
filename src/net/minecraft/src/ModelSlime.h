#pragma once

#include "ModelBase.h"
#include "java/Type.h"

class ModelRenderer;

// net.minecraft.src.ModelSlime
class ModelSlime : public ModelBase
{
public:
	ModelSlime(int_t i);
	~ModelSlime() override;

	void setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5) override;
	void render(float f, float f1, float f2, float f3, float f4, float f5) override;

	ModelRenderer *slimeBodies;
	ModelRenderer *slimeRightEye;
	ModelRenderer *slimeLeftEye;
	ModelRenderer *slimeMouth;
};
