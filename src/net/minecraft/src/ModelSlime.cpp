#include "ModelSlime.h"

#include "ModelRenderer.h"

ModelSlime::~ModelSlime()
{
	delete slimeBodies;
	delete slimeRightEye;
	delete slimeLeftEye;
	delete slimeMouth;
}

ModelSlime::ModelSlime(int_t i)
{
	slimeRightEye = nullptr;
	slimeLeftEye = nullptr;
	slimeMouth = nullptr;
	slimeBodies = new ModelRenderer(0, i);
	slimeBodies->addBox(-4.0f, 16.0f, -4.0f, 8, 8, 8);
	if (i > 0)
	{
		delete slimeBodies; // Java reassignment relied on GC
		slimeBodies = new ModelRenderer(0, i);
		slimeBodies->addBox(-3.0f, 17.0f, -3.0f, 6, 6, 6);
		slimeRightEye = new ModelRenderer(32, 0);
		slimeRightEye->addBox(-3.25f, 18.0f, -3.5f, 2, 2, 2);
		slimeLeftEye = new ModelRenderer(32, 4);
		slimeLeftEye->addBox(1.25f, 18.0f, -3.5f, 2, 2, 2);
		slimeMouth = new ModelRenderer(32, 8);
		slimeMouth->addBox(0.0f, 21.0f, -3.5f, 1, 1, 1);
	}
}

void ModelSlime::setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5)
{
}

void ModelSlime::render(float f, float f1, float f2, float f3, float f4, float f5)
{
	setRotationAngles(f, f1, f2, f3, f4, f5);
	slimeBodies->render(f5);
	if (slimeRightEye != nullptr)
	{
		slimeRightEye->render(f5);
		slimeLeftEye->render(f5);
		slimeMouth->render(f5);
	}
}
