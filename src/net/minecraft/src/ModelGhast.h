#pragma once

#include "ModelBase.h"

class ModelRenderer;

// net.minecraft.src.ModelGhast
class ModelGhast : public ModelBase
{
public:
	ModelGhast();
	~ModelGhast() override;

	void setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5) override;
	void render(float f, float f1, float f2, float f3, float f4, float f5) override;

	ModelRenderer *body;
	ModelRenderer *tentacles[9];
};
