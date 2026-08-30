#pragma once

#include "ModelBiped.h"

// net.minecraft.src.ModelZombie
class ModelZombie : public ModelBiped
{
public:
	ModelZombie();

	void setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5) override;
};
