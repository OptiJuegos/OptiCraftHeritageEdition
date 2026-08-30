#pragma once

#include "ModelBase.h"

class ModelRenderer;

// net.minecraft.src.ModelMinecart
class ModelMinecart : public ModelBase
{
public:
	ModelMinecart();
	~ModelMinecart() override;

	void render(float f, float f1, float f2, float f3, float f4, float f5) override;
	void setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5) override;

	ModelRenderer *sideModels[7];
};
