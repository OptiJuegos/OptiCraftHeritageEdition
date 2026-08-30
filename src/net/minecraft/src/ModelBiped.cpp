#include "ModelBiped.h"

#include "MathHelper.h"
#include "ModelRenderer.h"

ModelBiped::~ModelBiped()
{
	delete bipedHead;
	delete bipedHeadwear;
	delete bipedBody;
	delete bipedRightArm;
	delete bipedLeftArm;
	delete bipedRightLeg;
	delete bipedLeftLeg;
	delete bipedEars;
	delete bipedCloak;
}

ModelBiped::ModelBiped() : ModelBiped(0.0f)
{
}

ModelBiped::ModelBiped(float f) : ModelBiped(f, 0.0f)
{
}

ModelBiped::ModelBiped(float f, float f1)
{
	heldItemLeft = 0;
	heldItemRight = 0;
	isSneak = false;
	aimedBow = false;
	bipedCloak = new ModelRenderer(0, 0);
	bipedCloak->addBox(-5.0f, 0.0f, -1.0f, 10, 16, 1, f);
	bipedEars = new ModelRenderer(24, 0);
	bipedEars->addBox(-3.0f, -6.0f, -1.0f, 6, 6, 1, f);
	bipedHead = new ModelRenderer(0, 0);
	bipedHead->addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, f);
	bipedHead->setRotationPoint(0.0f, 0.0f + f1, 0.0f);
	bipedHeadwear = new ModelRenderer(32, 0);
	bipedHeadwear->addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, f + 0.5f);
	bipedHeadwear->setRotationPoint(0.0f, 0.0f + f1, 0.0f);
	bipedBody = new ModelRenderer(16, 16);
	bipedBody->addBox(-4.0f, 0.0f, -2.0f, 8, 12, 4, f);
	bipedBody->setRotationPoint(0.0f, 0.0f + f1, 0.0f);
	bipedRightArm = new ModelRenderer(40, 16);
	bipedRightArm->addBox(-3.0f, -2.0f, -2.0f, 4, 12, 4, f);
	bipedRightArm->setRotationPoint(-5.0f, 2.0f + f1, 0.0f);
	bipedLeftArm = new ModelRenderer(40, 16);
	bipedLeftArm->mirror = true;
	bipedLeftArm->addBox(-1.0f, -2.0f, -2.0f, 4, 12, 4, f);
	bipedLeftArm->setRotationPoint(5.0f, 2.0f + f1, 0.0f);
	bipedRightLeg = new ModelRenderer(0, 16);
	bipedRightLeg->addBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, f);
	bipedRightLeg->setRotationPoint(-2.0f, 12.0f + f1, 0.0f);
	bipedLeftLeg = new ModelRenderer(0, 16);
	bipedLeftLeg->mirror = true;
	bipedLeftLeg->addBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, f);
	bipedLeftLeg->setRotationPoint(2.0f, 12.0f + f1, 0.0f);
}

void ModelBiped::render(float f, float f1, float f2, float f3, float f4, float f5)
{
	setRotationAngles(f, f1, f2, f3, f4, f5);
	bipedHead->render(f5);
	bipedBody->render(f5);
	bipedRightArm->render(f5);
	bipedLeftArm->render(f5);
	bipedRightLeg->render(f5);
	bipedLeftLeg->render(f5);
	bipedHeadwear->render(f5);
}

void ModelBiped::setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5)
{
	bipedHead->rotateAngleY = f3 / 57.295776f;
	bipedHead->rotateAngleX = f4 / 57.295776f;
	bipedHeadwear->rotateAngleY = bipedHead->rotateAngleY;
	bipedHeadwear->rotateAngleX = bipedHead->rotateAngleX;
	bipedRightArm->rotateAngleX = MathHelper::cos(f * 0.6662f + 3.1415927f) * 2.0f * f1 * 0.5f;
	bipedLeftArm->rotateAngleX = MathHelper::cos(f * 0.6662f) * 2.0f * f1 * 0.5f;
	bipedRightArm->rotateAngleZ = 0.0f;
	bipedLeftArm->rotateAngleZ = 0.0f;
	bipedRightLeg->rotateAngleX = MathHelper::cos(f * 0.6662f) * 1.4f * f1;
	bipedLeftLeg->rotateAngleX = MathHelper::cos(f * 0.6662f + 3.1415927f) * 1.4f * f1;
	bipedRightLeg->rotateAngleY = 0.0f;
	bipedLeftLeg->rotateAngleY = 0.0f;
	if (isRiding)
	{
		bipedRightArm->rotateAngleX += -0.6283185f;
		bipedLeftArm->rotateAngleX += -0.6283185f;
		bipedRightLeg->rotateAngleX = -1.2566371f;
		bipedLeftLeg->rotateAngleX = -1.2566371f;
		bipedRightLeg->rotateAngleY = 0.3141593f;
		bipedLeftLeg->rotateAngleY = -0.3141593f;
	}
	if (heldItemLeft != 0)
	{
		bipedLeftArm->rotateAngleX = bipedLeftArm->rotateAngleX * 0.5f - 0.3141593f * static_cast<float>(heldItemLeft);
	}
	if (heldItemRight != 0)
	{
		bipedRightArm->rotateAngleX = bipedRightArm->rotateAngleX * 0.5f - 0.3141593f * static_cast<float>(heldItemRight);
	}
	bipedRightArm->rotateAngleY = 0.0f;
	bipedLeftArm->rotateAngleY = 0.0f;
	if (onGround > -9990.0f)
	{
		float f6 = onGround;
		bipedBody->rotateAngleY = MathHelper::sin(MathHelper::sqrt_float(f6) * 3.1415927f * 2.0f) * 0.2f;
		bipedRightArm->rotationPointZ = MathHelper::sin(bipedBody->rotateAngleY) * 5.0f;
		bipedRightArm->rotationPointX = -MathHelper::cos(bipedBody->rotateAngleY) * 5.0f;
		bipedLeftArm->rotationPointZ = -MathHelper::sin(bipedBody->rotateAngleY) * 5.0f;
		bipedLeftArm->rotationPointX = MathHelper::cos(bipedBody->rotateAngleY) * 5.0f;
		bipedRightArm->rotateAngleY += bipedBody->rotateAngleY;
		bipedLeftArm->rotateAngleY += bipedBody->rotateAngleY;
		bipedLeftArm->rotateAngleX += bipedBody->rotateAngleY;
		f6 = 1.0f - onGround;
		f6 *= f6;
		f6 *= f6;
		f6 = 1.0f - f6;
		float f7 = MathHelper::sin(f6 * 3.1415927f);
		float f8 = MathHelper::sin(onGround * 3.1415927f) * -(bipedHead->rotateAngleX - 0.7f) * 0.75f;
		bipedRightArm->rotateAngleX -= f7 * 1.2f + f8;
		bipedRightArm->rotateAngleY += bipedBody->rotateAngleY * 2.0f;
		bipedRightArm->rotateAngleZ = MathHelper::sin(onGround * 3.1415927f) * -0.4f;
	}
	if (isSneak)
	{
		bipedBody->rotateAngleX = 0.5f;
		bipedRightLeg->rotateAngleX -= 0.0f;
		bipedLeftLeg->rotateAngleX -= 0.0f;
		bipedRightArm->rotateAngleX += 0.4f;
		bipedLeftArm->rotateAngleX += 0.4f;
		bipedRightLeg->rotationPointZ = 4.0f;
		bipedLeftLeg->rotationPointZ = 4.0f;
		bipedRightLeg->rotationPointY = 9.0f;
		bipedLeftLeg->rotationPointY = 9.0f;
		bipedHead->rotationPointY = 1.0f;
	}
	else
	{
		bipedBody->rotateAngleX = 0.0f;
		bipedRightLeg->rotationPointZ = 0.0f;
		bipedLeftLeg->rotationPointZ = 0.0f;
		bipedRightLeg->rotationPointY = 12.0f;
		bipedLeftLeg->rotationPointY = 12.0f;
		bipedHead->rotationPointY = 0.0f;
	}
	bipedRightArm->rotateAngleZ += MathHelper::cos(f2 * 0.09f) * 0.05f + 0.05f;
	bipedLeftArm->rotateAngleZ -= MathHelper::cos(f2 * 0.09f) * 0.05f + 0.05f;
	bipedRightArm->rotateAngleX += MathHelper::sin(f2 * 0.067f) * 0.05f;
	bipedLeftArm->rotateAngleX -= MathHelper::sin(f2 * 0.067f) * 0.05f;
	if (aimedBow)
	{
		const float bowYawOffset = 0.0f;
		const float bowPitchOffset = 0.0f;
		bipedRightArm->rotateAngleZ = 0.0f;
		bipedLeftArm->rotateAngleZ = 0.0f;
		bipedRightArm->rotateAngleY = -(0.1f - bowYawOffset * 0.6f) + bipedHead->rotateAngleY;
		bipedLeftArm->rotateAngleY = 0.1f - bowYawOffset * 0.6f + bipedHead->rotateAngleY + 0.4f;
		bipedRightArm->rotateAngleX = -1.5707964f + bipedHead->rotateAngleX;
		bipedLeftArm->rotateAngleX = -1.5707964f + bipedHead->rotateAngleX;
		bipedRightArm->rotateAngleX -= bowYawOffset * 1.2f - bowPitchOffset * 0.4f;
		bipedLeftArm->rotateAngleX -= bowYawOffset * 1.2f - bowPitchOffset * 0.4f;
		bipedRightArm->rotateAngleZ += MathHelper::cos(f2 * 0.09f) * 0.05f + 0.05f;
		bipedLeftArm->rotateAngleZ -= MathHelper::cos(f2 * 0.09f) * 0.05f + 0.05f;
		bipedRightArm->rotateAngleX += MathHelper::sin(f2 * 0.067f) * 0.05f;
		bipedLeftArm->rotateAngleX -= MathHelper::sin(f2 * 0.067f) * 0.05f;
	}
}

void ModelBiped::renderEars(float f)
{
	bipedEars->rotateAngleY = bipedHead->rotateAngleY;
	bipedEars->rotateAngleX = bipedHead->rotateAngleX;
	bipedEars->rotationPointX = 0.0f;
	bipedEars->rotationPointY = 0.0f;
	bipedEars->render(f);
}

void ModelBiped::renderCloak(float f)
{
	bipedCloak->render(f);
}
