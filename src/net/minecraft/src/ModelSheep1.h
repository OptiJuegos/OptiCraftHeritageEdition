#pragma once

#include "ModelQuadruped.h"

// net.minecraft.src.ModelSheep1
class ModelSheep1 : public ModelQuadruped
{
public:
	ModelSheep1();
	void setLivingAnimations(EntityLiving *entityliving, float f, float f1, float f2) override;
	void setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5) override;

private:
	float field_44016_o;
};
