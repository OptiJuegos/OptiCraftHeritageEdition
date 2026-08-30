#include "ModelCreeper.h"

#include "MathHelper.h"
#include "ModelRenderer.h"

ModelCreeper::~ModelCreeper()
{
	delete head;
	delete creeperAura;
	delete body;
	delete leg1;
	delete leg2;
	delete leg3;
	delete leg4;
}

ModelCreeper::ModelCreeper() : ModelCreeper(0.0f)
{
}

ModelCreeper::ModelCreeper(float f)
{
	int_t i = 4;
	head = new ModelRenderer(0, 0);
	head->addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, f);
	head->setRotationPoint(0.0f, i, 0.0f);
	creeperAura = new ModelRenderer(32, 0);
	creeperAura->addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, f + 0.5f);
	creeperAura->setRotationPoint(0.0f, i, 0.0f);
	body = new ModelRenderer(16, 16);
	body->addBox(-4.0f, 0.0f, -2.0f, 8, 12, 4, f);
	body->setRotationPoint(0.0f, i, 0.0f);
	leg1 = new ModelRenderer(0, 16);
	leg1->addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, f);
	leg1->setRotationPoint(-2.0f, 12 + i, 4.0f);
	leg2 = new ModelRenderer(0, 16);
	leg2->addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, f);
	leg2->setRotationPoint(2.0f, 12 + i, 4.0f);
	leg3 = new ModelRenderer(0, 16);
	leg3->addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, f);
	leg3->setRotationPoint(-2.0f, 12 + i, -4.0f);
	leg4 = new ModelRenderer(0, 16);
	leg4->addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, f);
	leg4->setRotationPoint(2.0f, 12 + i, -4.0f);
}

void ModelCreeper::render(float f, float f1, float f2, float f3, float f4, float f5)
{
	setRotationAngles(f, f1, f2, f3, f4, f5);
	head->render(f5);
	body->render(f5);
	leg1->render(f5);
	leg2->render(f5);
	leg3->render(f5);
	leg4->render(f5);
}

void ModelCreeper::setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5)
{
	head->rotateAngleY = f3 / 57.295776f;
	head->rotateAngleX = f4 / 57.295776f;
	leg1->rotateAngleX = MathHelper::cos(f * 0.6662f) * 1.4f * f1;
	leg2->rotateAngleX = MathHelper::cos(f * 0.6662f + 3.1415927f) * 1.4f * f1;
	leg3->rotateAngleX = MathHelper::cos(f * 0.6662f + 3.1415927f) * 1.4f * f1;
	leg4->rotateAngleX = MathHelper::cos(f * 0.6662f) * 1.4f * f1;
}
