#pragma once

#include "ModelQuadruped.h"

// net.minecraft.src.ModelSheep2
class ModelSheep2 : public ModelQuadruped
{
public:
	ModelSheep2();
	void setLivingAnimations(EntityLiving *entityliving, float f, float f1, float f2) override;
	void setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5) override;

private:
	float field_44017_o;
};
