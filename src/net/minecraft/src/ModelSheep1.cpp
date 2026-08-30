#include "ModelSheep1.h"

#include "ModelRenderer.h"
#include "EntitySheep.h"

ModelSheep1::ModelSheep1() : ModelQuadruped(12, 0.0f)
{
	field_44016_o = 0.0f;
	delete head; // Java reassignment relied on GC
	head = new ModelRenderer(0, 0);
	head->addBox(-3.0f, -4.0f, -4.0f, 6, 6, 6, 0.6f);
	head->setRotationPoint(0.0f, 6.0f, -8.0f);
	delete body; // Java reassignment relied on GC
	body = new ModelRenderer(28, 8);
	body->addBox(-4.0f, -10.0f, -7.0f, 8, 16, 6, 1.75f);
	body->setRotationPoint(0.0f, 5.0f, 2.0f);
	float f = 0.5f;
	delete leg1; // Java reassignment relied on GC
	leg1 = new ModelRenderer(0, 16);
	leg1->addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, f);
	leg1->setRotationPoint(-3.0f, 12.0f, 7.0f);
	delete leg2; // Java reassignment relied on GC
	leg2 = new ModelRenderer(0, 16);
	leg2->addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, f);
	leg2->setRotationPoint(3.0f, 12.0f, 7.0f);
	delete leg3; // Java reassignment relied on GC
	leg3 = new ModelRenderer(0, 16);
	leg3->addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, f);
	leg3->setRotationPoint(-3.0f, 12.0f, -5.0f);
	delete leg4; // Java reassignment relied on GC
	leg4 = new ModelRenderer(0, 16);
	leg4->addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, f);
	leg4->setRotationPoint(3.0f, 12.0f, -5.0f);
}

void ModelSheep1::setLivingAnimations(EntityLiving *entityliving, float f, float f1, float f2)
{
	ModelQuadruped::setLivingAnimations(entityliving, f, f1, f2);
	EntitySheep *sheep = static_cast<EntitySheep *>(entityliving);
	head->rotationPointY = 6.0f + sheep->getHeadRotationPointY(f2) * 9.0f;
	field_44016_o = sheep->getHeadRotationAngleX(f2);
}

void ModelSheep1::setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5)
{
	ModelQuadruped::setRotationAngles(f, f1, f2, f3, f4, f5);
	head->rotateAngleX = field_44016_o;
}
