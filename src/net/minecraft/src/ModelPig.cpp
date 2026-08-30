#include "ModelPig.h"

#include "ModelRenderer.h"

ModelPig::ModelPig() : ModelPig(0.0f)
{
}

ModelPig::ModelPig(float f) : ModelQuadruped(6, f)
{
	head->setTextureOffset(16, 16)->addBox(-2.0f, 0.0f, -9.0f, 4, 3, 1, f);
	field_40331_g = 4.0f;
}
