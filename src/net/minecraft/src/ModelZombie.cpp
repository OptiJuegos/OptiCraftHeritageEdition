#include "ModelZombie.h"

#include "MathHelper.h"
#include "ModelRenderer.h"

ModelZombie::ModelZombie()
{
}

void ModelZombie::setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5)
{
	ModelBiped::setRotationAngles(f, f1, f2, f3, f4, f5);
	float f6 = MathHelper::sin(onGround * 3.1415927f);
	float f7 = MathHelper::sin((1.0f - (1.0f - onGround) * (1.0f - onGround)) * 3.1415927f);
	bipedRightArm->rotateAngleZ = 0.0f;
	bipedLeftArm->rotateAngleZ = 0.0f;
	bipedRightArm->rotateAngleY = -(0.1f - f6 * 0.6f);
	bipedLeftArm->rotateAngleY = 0.1f - f6 * 0.6f;
	bipedRightArm->rotateAngleX = -1.5707964f;
	bipedLeftArm->rotateAngleX = -1.5707964f;
	bipedRightArm->rotateAngleX -= f6 * 1.2f - f7 * 0.4f;
	bipedLeftArm->rotateAngleX -= f6 * 1.2f - f7 * 0.4f;
	bipedRightArm->rotateAngleZ += MathHelper::cos(f2 * 0.09f) * 0.05f + 0.05f;
	bipedLeftArm->rotateAngleZ -= MathHelper::cos(f2 * 0.09f) * 0.05f + 0.05f;
	bipedRightArm->rotateAngleX += MathHelper::sin(f2 * 0.067f) * 0.05f;
	bipedLeftArm->rotateAngleX -= MathHelper::sin(f2 * 0.067f) * 0.05f;
}
