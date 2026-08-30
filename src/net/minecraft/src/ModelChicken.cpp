#include "ModelChicken.h"

#include "MathHelper.h"
#include "ModelRenderer.h"
#include "platform/RenderAPI.h"

ModelChicken::~ModelChicken()
{
	delete head;
	delete body;
	delete rightLeg;
	delete leftLeg;
	delete rightWing;
	delete leftWing;
	delete bill;
	delete chin;
}

ModelChicken::ModelChicken()
{
	int_t byte0 = 16;
	head = new ModelRenderer(0, 0);
	head->addBox(-2.0f, -6.0f, -2.0f, 4, 6, 3, 0.0f);
	head->setRotationPoint(0.0f, -1 + byte0, -4.0f);
	bill = new ModelRenderer(14, 0);
	bill->addBox(-2.0f, -4.0f, -4.0f, 4, 2, 2, 0.0f);
	bill->setRotationPoint(0.0f, -1 + byte0, -4.0f);
	chin = new ModelRenderer(14, 4);
	chin->addBox(-1.0f, -2.0f, -3.0f, 2, 2, 2, 0.0f);
	chin->setRotationPoint(0.0f, -1 + byte0, -4.0f);
	body = new ModelRenderer(0, 9);
	body->addBox(-3.0f, -4.0f, -3.0f, 6, 8, 6, 0.0f);
	body->setRotationPoint(0.0f, 0 + byte0, 0.0f);
	rightLeg = new ModelRenderer(26, 0);
	rightLeg->addBox(-1.0f, 0.0f, -3.0f, 3, 5, 3);
	rightLeg->setRotationPoint(-2.0f, 3 + byte0, 1.0f);
	leftLeg = new ModelRenderer(26, 0);
	leftLeg->addBox(-1.0f, 0.0f, -3.0f, 3, 5, 3);
	leftLeg->setRotationPoint(1.0f, 3 + byte0, 1.0f);
	rightWing = new ModelRenderer(24, 13);
	rightWing->addBox(0.0f, 0.0f, -3.0f, 1, 4, 6);
	rightWing->setRotationPoint(-4.0f, -3 + byte0, 0.0f);
	leftWing = new ModelRenderer(24, 13);
	leftWing->addBox(-1.0f, 0.0f, -3.0f, 1, 4, 6);
	leftWing->setRotationPoint(4.0f, -3 + byte0, 0.0f);
}

void ModelChicken::render(float f, float f1, float f2, float f3, float f4, float f5)
{
	setRotationAngles(f, f1, f2, f3, f4, f5);
	if (isChild)
	{
		renderPushMatrix();
		renderTranslate(0.0f, 5.0f * f5, 2.0f * f5);
		head->render(f5);
		bill->render(f5);
		chin->render(f5);
		renderPopMatrix();
		renderPushMatrix();
		renderScale(0.5f, 0.5f, 0.5f);
		renderTranslate(0.0f, 24.0f * f5, 0.0f);
		body->render(f5);
		rightLeg->render(f5);
		leftLeg->render(f5);
		rightWing->render(f5);
		leftWing->render(f5);
		renderPopMatrix();
	}
	else
	{
		head->render(f5);
		bill->render(f5);
		chin->render(f5);
		body->render(f5);
		rightLeg->render(f5);
		leftLeg->render(f5);
		rightWing->render(f5);
		leftWing->render(f5);
	}
}

void ModelChicken::setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5)
{
	head->rotateAngleX = -(f4 / 57.295776f);
	head->rotateAngleY = f3 / 57.295776f;
	bill->rotateAngleX = head->rotateAngleX;
	bill->rotateAngleY = head->rotateAngleY;
	chin->rotateAngleX = head->rotateAngleX;
	chin->rotateAngleY = head->rotateAngleY;
	body->rotateAngleX = 1.5707964f;
	rightLeg->rotateAngleX = MathHelper::cos(f * 0.6662f) * 1.4f * f1;
	leftLeg->rotateAngleX = MathHelper::cos(f * 0.6662f + 3.1415927f) * 1.4f * f1;
	rightWing->rotateAngleZ = f2;
	leftWing->rotateAngleZ = -f2;
}
