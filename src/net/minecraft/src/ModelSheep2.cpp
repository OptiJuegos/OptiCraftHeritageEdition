#include "ModelSheep2.h"

#include "ModelRenderer.h"
#include "EntitySheep.h"

ModelSheep2::ModelSheep2() : ModelQuadruped(12, 0.0f)
{
	field_44017_o = 0.0f;
	delete head; // Java reassignment relied on GC
	head = new ModelRenderer(0, 0);
	head->addBox(-3.0f, -4.0f, -6.0f, 6, 6, 8, 0.0f);
	head->setRotationPoint(0.0f, 6.0f, -8.0f);
	delete body; // Java reassignment relied on GC
	body = new ModelRenderer(28, 8);
	body->addBox(-4.0f, -10.0f, -7.0f, 8, 16, 6, 0.0f);
	body->setRotationPoint(0.0f, 5.0f, 2.0f);
}

void ModelSheep2::setLivingAnimations(EntityLiving *entityliving, float f, float f1, float f2)
{
	ModelQuadruped::setLivingAnimations(entityliving, f, f1, f2);
	EntitySheep *sheep = static_cast<EntitySheep *>(entityliving);
	head->rotationPointY = 6.0f + sheep->getHeadRotationPointY(f2) * 9.0f;
	field_44017_o = sheep->getHeadRotationAngleX(f2);
}

void ModelSheep2::setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5)
{
	ModelQuadruped::setRotationAngles(f, f1, f2, f3, f4, f5);
	head->rotateAngleX = field_44017_o;
}
