#include "ModelOcelot.h"

#include "EntityOcelot.h"
#include "MathHelper.h"
#include "ModelRenderer.h"
#include "platform/RenderAPI.h"

ModelOcelot::ModelOcelot()
	: backLeftLeg(nullptr), backRightLeg(nullptr), frontLeftLeg(nullptr), frontRightLeg(nullptr), tailBase(nullptr),
	  tailTip(nullptr), head(nullptr), nose(nullptr), earLeft(nullptr), earRight(nullptr), body(nullptr), animationState(1)
{
	head = new ModelRenderer(0, 0);
	head->addBox(-2.5f, -2.0f, -3.0f, 5, 4, 5);
	head->setRotationPoint(0.0f, 15.0f, -9.0f);
	nose = new ModelRenderer(0, 24);
	nose->addBox(-1.5f, 0.0f, -4.0f, 3, 2, 2);
	nose->setRotationPoint(0.0f, 15.0f, -9.0f);
	earLeft = new ModelRenderer(0, 10);
	earLeft->addBox(-2.0f, -3.0f, 0.0f, 1, 1, 2);
	earLeft->setRotationPoint(0.0f, 15.0f, -9.0f);
	earRight = new ModelRenderer(6, 10);
	earRight->addBox(1.0f, -3.0f, 0.0f, 1, 1, 2);
	earRight->setRotationPoint(0.0f, 15.0f, -9.0f);

	body = new ModelRenderer(20, 0);
	body->addBox(-2.0f, 3.0f, -8.0f, 4, 16, 6, 0.0f);
	body->setRotationPoint(0.0f, 12.0f, -10.0f);

	tailBase = new ModelRenderer(0, 15);
	tailBase->addBox(-0.5f, 0.0f, 0.0f, 1, 8, 1);
	tailBase->rotateAngleX = 0.9f;
	tailBase->setRotationPoint(0.0f, 15.0f, 8.0f);

	tailTip = new ModelRenderer(4, 15);
	tailTip->addBox(-0.5f, 0.0f, 0.0f, 1, 8, 1);
	tailTip->setRotationPoint(0.0f, 20.0f, 14.0f);

	backLeftLeg = new ModelRenderer(8, 13);
	backLeftLeg->addBox(-1.0f, 0.0f, 1.0f, 2, 6, 2);
	backLeftLeg->setRotationPoint(1.1f, 18.0f, 5.0f);
	backRightLeg = new ModelRenderer(8, 13);
	backRightLeg->addBox(-1.0f, 0.0f, 1.0f, 2, 6, 2);
	backRightLeg->setRotationPoint(-1.1f, 18.0f, 5.0f);
	frontLeftLeg = new ModelRenderer(40, 0);
	frontLeftLeg->addBox(-1.0f, 0.0f, 0.0f, 2, 10, 2);
	frontLeftLeg->setRotationPoint(1.2f, 13.8f, -5.0f);
	frontRightLeg = new ModelRenderer(40, 0);
	frontRightLeg->addBox(-1.0f, 0.0f, 0.0f, 2, 10, 2);
	frontRightLeg->setRotationPoint(-1.2f, 13.8f, -5.0f);
}

ModelOcelot::~ModelOcelot()
{
	delete backLeftLeg;
	delete backRightLeg;
	delete frontLeftLeg;
	delete frontRightLeg;
	delete tailBase;
	delete tailTip;
	delete head;
	delete nose;
	delete earLeft;
	delete earRight;
	delete body;
}

void ModelOcelot::render(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scale)
{
	setRotationAngles(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale);
	if (isChild)
	{
		const float childScale = 2.0f;
		renderPushMatrix();
		renderScale(1.5f / childScale, 1.5f / childScale, 1.5f / childScale);
		renderTranslate(0.0f, 10.0f * scale, 4.0f * scale);
		head->render(scale);
		nose->render(scale);
		earLeft->render(scale);
		earRight->render(scale);
		renderPopMatrix();
		renderPushMatrix();
		renderScale(1.0f / childScale, 1.0f / childScale, 1.0f / childScale);
		renderTranslate(0.0f, 24.0f * scale, 0.0f);
		body->render(scale);
		backLeftLeg->render(scale);
		backRightLeg->render(scale);
		frontLeftLeg->render(scale);
		frontRightLeg->render(scale);
		tailBase->render(scale);
		tailTip->render(scale);
		renderPopMatrix();
		return;
	}
	head->render(scale);
	nose->render(scale);
	earLeft->render(scale);
	earRight->render(scale);
	body->render(scale);
	tailBase->render(scale);
	tailTip->render(scale);
	backLeftLeg->render(scale);
	backRightLeg->render(scale);
	frontLeftLeg->render(scale);
	frontRightLeg->render(scale);
}

void ModelOcelot::setRotationAngles(float limbSwing, float limbSwingAmount, float, float netHeadYaw, float headPitch, float)
{
	head->rotateAngleX = headPitch / 57.295776f;
	head->rotateAngleY = netHeadYaw / 57.295776f;
	nose->rotateAngleX = earLeft->rotateAngleX = earRight->rotateAngleX = head->rotateAngleX;
	nose->rotateAngleY = earLeft->rotateAngleY = earRight->rotateAngleY = head->rotateAngleY;
	if (animationState == 3)
		return;
	body->rotateAngleX = 1.5707964f;
	if (animationState == 2)
	{
		backLeftLeg->rotateAngleX = MathHelper::cos(limbSwing * 0.6662f) * limbSwingAmount;
		backRightLeg->rotateAngleX = MathHelper::cos(limbSwing * 0.6662f + 0.3f) * limbSwingAmount;
		frontLeftLeg->rotateAngleX = MathHelper::cos(limbSwing * 0.6662f + 3.4415927f) * limbSwingAmount;
		frontRightLeg->rotateAngleX = MathHelper::cos(limbSwing * 0.6662f + 3.1415927f) * limbSwingAmount;
		tailTip->rotateAngleX = 1.727876f + 0.31415927f * MathHelper::cos(limbSwing) * limbSwingAmount;
	}
	else
	{
		backLeftLeg->rotateAngleX = MathHelper::cos(limbSwing * 0.6662f) * limbSwingAmount;
		backRightLeg->rotateAngleX = MathHelper::cos(limbSwing * 0.6662f + 3.1415927f) * limbSwingAmount;
		frontLeftLeg->rotateAngleX = MathHelper::cos(limbSwing * 0.6662f + 3.1415927f) * limbSwingAmount;
		frontRightLeg->rotateAngleX = MathHelper::cos(limbSwing * 0.6662f) * limbSwingAmount;
		tailTip->rotateAngleX = 1.727876f + (animationState == 1 ? 0.7853982f : 0.4712389f) * MathHelper::cos(limbSwing) * limbSwingAmount;
	}
}

void ModelOcelot::setLivingAnimations(EntityLiving *entity, float, float, float)
{
	EntityOcelot *ocelot = dynamic_cast<EntityOcelot *>(entity);
	if (ocelot == nullptr)
		return;
	body->rotationPointY = 12.0f;
	body->rotationPointZ = -10.0f;
	head->rotationPointY = nose->rotationPointY = earLeft->rotationPointY = earRight->rotationPointY = 15.0f;
	head->rotationPointZ = nose->rotationPointZ = earLeft->rotationPointZ = earRight->rotationPointZ = -9.0f;
	tailBase->rotationPointY = 15.0f;
	tailBase->rotationPointZ = 8.0f;
	tailTip->rotationPointY = 20.0f;
	tailTip->rotationPointZ = 14.0f;
	frontLeftLeg->rotationPointY = frontRightLeg->rotationPointY = 13.8f;
	frontLeftLeg->rotationPointZ = frontRightLeg->rotationPointZ = -5.0f;
	backLeftLeg->rotationPointY = backRightLeg->rotationPointY = 18.0f;
	backLeftLeg->rotationPointZ = backRightLeg->rotationPointZ = 5.0f;
	tailBase->rotateAngleX = 0.9f;
	if (ocelot->isSneaking())
	{
		body->rotationPointY += 1.0f;
		head->rotationPointY += 2.0f;
		nose->rotationPointY += 2.0f;
		earLeft->rotationPointY += 2.0f;
		earRight->rotationPointY += 2.0f;
		tailBase->rotationPointY += 1.0f;
		tailTip->rotationPointY -= 4.0f;
		tailTip->rotationPointZ += 2.0f;
		tailBase->rotateAngleX = tailTip->rotateAngleX = 1.5707964f;
		animationState = 0;
	}
	else if (ocelot->isSprinting())
	{
		tailTip->rotationPointY = tailBase->rotationPointY;
		tailTip->rotationPointZ += 2.0f;
		tailBase->rotateAngleX = tailTip->rotateAngleX = 1.5707964f;
		animationState = 2;
	}
	else if (ocelot->isSitting())
	{
		body->rotateAngleX = 0.7853982f;
		body->rotationPointY -= 4.0f;
		body->rotationPointZ += 5.0f;
		head->rotationPointY -= 3.3f;
		nose->rotationPointY -= 3.3f;
		earLeft->rotationPointY -= 3.3f;
		earRight->rotationPointY -= 3.3f;
		head->rotationPointZ += 1.0f;
		nose->rotationPointZ += 1.0f;
		earLeft->rotationPointZ += 1.0f;
		earRight->rotationPointZ += 1.0f;
		tailBase->rotationPointY += 8.0f;
		tailBase->rotationPointZ -= 2.0f;
		tailTip->rotationPointY += 2.0f;
		tailTip->rotationPointZ -= 0.8f;
		tailBase->rotateAngleX = 1.727876f;
		tailTip->rotateAngleX = 2.670354f;
		frontLeftLeg->rotateAngleX = frontRightLeg->rotateAngleX = -0.15707964f;
		frontLeftLeg->rotationPointY = frontRightLeg->rotationPointY = 15.8f;
		frontLeftLeg->rotationPointZ = frontRightLeg->rotationPointZ = -7.0f;
		backLeftLeg->rotateAngleX = backRightLeg->rotateAngleX = -1.5707964f;
		backLeftLeg->rotationPointY = backRightLeg->rotationPointY = 21.0f;
		backLeftLeg->rotationPointZ = backRightLeg->rotationPointZ = 1.0f;
		animationState = 3;
	}
	else
	{
		animationState = 1;
	}
}
