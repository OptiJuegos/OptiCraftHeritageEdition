#include "ModelQuadruped.h"

#include "MathHelper.h"
#include "ModelRenderer.h"
#include "platform/RenderAPI.h"

ModelQuadruped::~ModelQuadruped()
{
	delete head;
	delete body;
	delete leg1;
	delete leg2;
	delete leg3;
	delete leg4;
}

ModelQuadruped::ModelQuadruped(int_t i, float f)
{
	field_40331_g = 8.0f;
	field_40332_n = 4.0f;
	head = new ModelRenderer(0, 0);
	head->addBox(-4.0f, -4.0f, -8.0f, 8, 8, 8, f);
	head->setRotationPoint(0.0f, 18 - i, -6.0f);
	body = new ModelRenderer(28, 8);
	body->addBox(-5.0f, -10.0f, -7.0f, 10, 16, 8, f);
	body->setRotationPoint(0.0f, 17 - i, 2.0f);
	leg1 = new ModelRenderer(0, 16);
	leg1->addBox(-2.0f, 0.0f, -2.0f, 4, i, 4, f);
	leg1->setRotationPoint(-3.0f, 24 - i, 7.0f);
	leg2 = new ModelRenderer(0, 16);
	leg2->addBox(-2.0f, 0.0f, -2.0f, 4, i, 4, f);
	leg2->setRotationPoint(3.0f, 24 - i, 7.0f);
	leg3 = new ModelRenderer(0, 16);
	leg3->addBox(-2.0f, 0.0f, -2.0f, 4, i, 4, f);
	leg3->setRotationPoint(-3.0f, 24 - i, -5.0f);
	leg4 = new ModelRenderer(0, 16);
	leg4->addBox(-2.0f, 0.0f, -2.0f, 4, i, 4, f);
	leg4->setRotationPoint(3.0f, 24 - i, -5.0f);
}

void ModelQuadruped::render(float f, float f1, float f2, float f3, float f4, float f5)
{
	setRotationAngles(f, f1, f2, f3, f4, f5);
	if (isChild)
	{
		renderPushMatrix();
		renderTranslate(0.0f, field_40331_g * f5, field_40332_n * f5);
		head->render(f5);
		renderPopMatrix();
		renderPushMatrix();
		renderScale(0.5f, 0.5f, 0.5f);
		renderTranslate(0.0f, 24.0f * f5, 0.0f);
		body->render(f5);
		leg1->render(f5);
		leg2->render(f5);
		leg3->render(f5);
		leg4->render(f5);
		renderPopMatrix();
	}
	else
	{
		head->render(f5);
		body->render(f5);
		leg1->render(f5);
		leg2->render(f5);
		leg3->render(f5);
		leg4->render(f5);
	}
}

void ModelQuadruped::setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5)
{
	head->rotateAngleX = f4 / 57.295776f;
	head->rotateAngleY = f3 / 57.295776f;
	body->rotateAngleX = 1.5707964f;
	leg1->rotateAngleX = MathHelper::cos(f * 0.6662f) * 1.4f * f1;
	leg2->rotateAngleX = MathHelper::cos(f * 0.6662f + 3.1415927f) * 1.4f * f1;
	leg3->rotateAngleX = MathHelper::cos(f * 0.6662f + 3.1415927f) * 1.4f * f1;
	leg4->rotateAngleX = MathHelper::cos(f * 0.6662f) * 1.4f * f1;
}
