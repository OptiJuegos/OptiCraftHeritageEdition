#pragma once

#include "ModelBase.h"

class ModelRenderer;

// net.minecraft.src.ModelBoat
class ModelBoat : public ModelBase
{
public:
	ModelBoat();
	~ModelBoat() override;

	void render(float f, float f1, float f2, float f3, float f4, float f5) override;
	void setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5) override;

	ModelRenderer *boatSides[5];
};
