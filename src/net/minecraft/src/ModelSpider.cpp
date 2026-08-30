#include "ModelSpider.h"

#include "MathHelper.h"
#include "ModelRenderer.h"

ModelSpider::~ModelSpider()
{
	delete spiderHead;
	delete spiderNeck;
	delete spiderBody;
	delete spiderLeg1;
	delete spiderLeg2;
	delete spiderLeg3;
	delete spiderLeg4;
	delete spiderLeg5;
	delete spiderLeg6;
	delete spiderLeg7;
	delete spiderLeg8;
}

ModelSpider::ModelSpider()
{
	float f = 0.0f;
	int_t i = 15;
	spiderHead = new ModelRenderer(32, 4);
	spiderHead->addBox(-4.0f, -4.0f, -8.0f, 8, 8, 8, f);
	spiderHead->setRotationPoint(0.0f, 0 + i, -3.0f);
	spiderNeck = new ModelRenderer(0, 0);
	spiderNeck->addBox(-3.0f, -3.0f, -3.0f, 6, 6, 6, f);
	spiderNeck->setRotationPoint(0.0f, i, 0.0f);
	spiderBody = new ModelRenderer(0, 12);
	spiderBody->addBox(-5.0f, -4.0f, -6.0f, 10, 8, 12, f);
	spiderBody->setRotationPoint(0.0f, 0 + i, 9.0f);
	spiderLeg1 = new ModelRenderer(18, 0);
	spiderLeg1->addBox(-15.0f, -1.0f, -1.0f, 16, 2, 2, f);
	spiderLeg1->setRotationPoint(-4.0f, 0 + i, 2.0f);
	spiderLeg2 = new ModelRenderer(18, 0);
	spiderLeg2->addBox(-1.0f, -1.0f, -1.0f, 16, 2, 2, f);
	spiderLeg2->setRotationPoint(4.0f, 0 + i, 2.0f);
	spiderLeg3 = new ModelRenderer(18, 0);
	spiderLeg3->addBox(-15.0f, -1.0f, -1.0f, 16, 2, 2, f);
	spiderLeg3->setRotationPoint(-4.0f, 0 + i, 1.0f);
	spiderLeg4 = new ModelRenderer(18, 0);
	spiderLeg4->addBox(-1.0f, -1.0f, -1.0f, 16, 2, 2, f);
	spiderLeg4->setRotationPoint(4.0f, 0 + i, 1.0f);
	spiderLeg5 = new ModelRenderer(18, 0);
	spiderLeg5->addBox(-15.0f, -1.0f, -1.0f, 16, 2, 2, f);
	spiderLeg5->setRotationPoint(-4.0f, 0 + i, 0.0f);
	spiderLeg6 = new ModelRenderer(18, 0);
	spiderLeg6->addBox(-1.0f, -1.0f, -1.0f, 16, 2, 2, f);
	spiderLeg6->setRotationPoint(4.0f, 0 + i, 0.0f);
	spiderLeg7 = new ModelRenderer(18, 0);
	spiderLeg7->addBox(-15.0f, -1.0f, -1.0f, 16, 2, 2, f);
	spiderLeg7->setRotationPoint(-4.0f, 0 + i, -1.0f);
	spiderLeg8 = new ModelRenderer(18, 0);
	spiderLeg8->addBox(-1.0f, -1.0f, -1.0f, 16, 2, 2, f);
	spiderLeg8->setRotationPoint(4.0f, 0 + i, -1.0f);
}

void ModelSpider::render(float f, float f1, float f2, float f3, float f4, float f5)
{
	setRotationAngles(f, f1, f2, f3, f4, f5);
	spiderHead->render(f5);
	spiderNeck->render(f5);
	spiderBody->render(f5);
	spiderLeg1->render(f5);
	spiderLeg2->render(f5);
	spiderLeg3->render(f5);
	spiderLeg4->render(f5);
	spiderLeg5->render(f5);
	spiderLeg6->render(f5);
	spiderLeg7->render(f5);
	spiderLeg8->render(f5);
}

void ModelSpider::setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5)
{
	spiderHead->rotateAngleY = f3 / 57.295776f;
	spiderHead->rotateAngleX = f4 / 57.295776f;
	float f6 = 0.7853982f;
	spiderLeg1->rotateAngleZ = -f6;
	spiderLeg2->rotateAngleZ = f6;
	spiderLeg3->rotateAngleZ = -f6 * 0.74f;
	spiderLeg4->rotateAngleZ = f6 * 0.74f;
	spiderLeg5->rotateAngleZ = -f6 * 0.74f;
	spiderLeg6->rotateAngleZ = f6 * 0.74f;
	spiderLeg7->rotateAngleZ = -f6;
	spiderLeg8->rotateAngleZ = f6;
	float f7 = -0.0f;
	float f8 = 0.3926991f;
	spiderLeg1->rotateAngleY = f8 * 2.0f + f7;
	spiderLeg2->rotateAngleY = -f8 * 2.0f - f7;
	spiderLeg3->rotateAngleY = f8 * 1.0f + f7;
	spiderLeg4->rotateAngleY = -f8 * 1.0f - f7;
	spiderLeg5->rotateAngleY = -f8 * 1.0f + f7;
	spiderLeg6->rotateAngleY = f8 * 1.0f - f7;
	spiderLeg7->rotateAngleY = -f8 * 2.0f + f7;
	spiderLeg8->rotateAngleY = f8 * 2.0f - f7;
	float f9 = -(MathHelper::cos(f * 0.6662f * 2.0f + 0.0f) * 0.4f) * f1;
	float f10 = -(MathHelper::cos(f * 0.6662f * 2.0f + 3.1415927f) * 0.4f) * f1;
	float f11 = -(MathHelper::cos(f * 0.6662f * 2.0f + 1.5707964f) * 0.4f) * f1;
	float f12 = -(MathHelper::cos(f * 0.6662f * 2.0f + 4.712389f) * 0.4f) * f1;
	float f13 = MathHelper::abs(MathHelper::sin(f * 0.6662f + 0.0f) * 0.4f) * f1;
	float f14 = MathHelper::abs(MathHelper::sin(f * 0.6662f + 3.1415927f) * 0.4f) * f1;
	float f15 = MathHelper::abs(MathHelper::sin(f * 0.6662f + 1.5707964f) * 0.4f) * f1;
	float f16 = MathHelper::abs(MathHelper::sin(f * 0.6662f + 4.712389f) * 0.4f) * f1;
	spiderLeg1->rotateAngleY += f9;
	spiderLeg2->rotateAngleY += -f9;
	spiderLeg3->rotateAngleY += f10;
	spiderLeg4->rotateAngleY += -f10;
	spiderLeg5->rotateAngleY += f11;
	spiderLeg6->rotateAngleY += -f11;
	spiderLeg7->rotateAngleY += f12;
	spiderLeg8->rotateAngleY += -f12;
	spiderLeg1->rotateAngleZ += f13;
	spiderLeg2->rotateAngleZ += -f13;
	spiderLeg3->rotateAngleZ += f14;
	spiderLeg4->rotateAngleZ += -f14;
	spiderLeg5->rotateAngleZ += f15;
	spiderLeg6->rotateAngleZ += -f15;
	spiderLeg7->rotateAngleZ += f16;
	spiderLeg8->rotateAngleZ += -f16;
}
