#include "ModelCow.h"

#include "ModelRenderer.h"

ModelCow::~ModelCow()
{
	delete udders;
	delete horn1;
	delete horn2;
}

ModelCow::ModelCow() : ModelQuadruped(12, 0.0f)
{
	delete head; // Java reassignment relied on GC
	head = new ModelRenderer(0, 0);
	head->addBox(-4.0f, -4.0f, -6.0f, 8, 8, 6, 0.0f);
	head->setRotationPoint(0.0f, 4.0f, -8.0f);
	head->setTextureOffset(22, 0)->addBox(-5.0f, -5.0f, -4.0f, 1, 3, 1, 0.0f);
	head->setTextureOffset(22, 0)->addBox(4.0f, -5.0f, -4.0f, 1, 3, 1, 0.0f);
	horn1 = nullptr;
	horn2 = nullptr;
	udders = nullptr;
	delete body; // Java reassignment relied on GC
	body = new ModelRenderer(18, 4);
	body->addBox(-6.0f, -10.0f, -7.0f, 12, 18, 10, 0.0f);
	body->setRotationPoint(0.0f, 5.0f, 2.0f);
	body->setTextureOffset(52, 0)->addBox(-2.0f, 2.0f, -8.0f, 4, 6, 1);
	leg1->rotationPointX--;
	leg2->rotationPointX++;
	leg1->rotationPointZ += 0.0f;
	leg2->rotationPointZ += 0.0f;
	leg3->rotationPointX--;
	leg4->rotationPointX++;
	leg3->rotationPointZ--;
	leg4->rotationPointZ--;
	field_40332_n += 2.0f;
}

void ModelCow::render(float f, float f1, float f2, float f3, float f4, float f5)
{
	ModelQuadruped::render(f, f1, f2, f3, f4, f5);
}

void ModelCow::setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5)
{
	ModelQuadruped::setRotationAngles(f, f1, f2, f3, f4, f5);
}
