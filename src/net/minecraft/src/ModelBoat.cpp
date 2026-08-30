#include "ModelBoat.h"

#include "ModelRenderer.h"

ModelBoat::~ModelBoat()
{
	delete boatSides[0];
	delete boatSides[1];
	delete boatSides[2];
	delete boatSides[3];
	delete boatSides[4];
}

ModelBoat::ModelBoat()
{
	boatSides[0] = new ModelRenderer(0, 8);
	boatSides[1] = new ModelRenderer(0, 0);
	boatSides[2] = new ModelRenderer(0, 0);
	boatSides[3] = new ModelRenderer(0, 0);
	boatSides[4] = new ModelRenderer(0, 0);
	int_t byte0 = 24;
	int_t byte1 = 6;
	int_t byte2 = 20;
	int_t byte3 = 4;
	boatSides[0]->addBox(-byte0 / 2, -byte2 / 2 + 2, -3.0f, byte0, byte2 - 4, 4, 0.0f);
	boatSides[0]->setRotationPoint(0.0f, 0 + byte3, 0.0f);
	boatSides[1]->addBox(-byte0 / 2 + 2, -byte1 - 1, -1.0f, byte0 - 4, byte1, 2, 0.0f);
	boatSides[1]->setRotationPoint(-byte0 / 2 + 1, 0 + byte3, 0.0f);
	boatSides[2]->addBox(-byte0 / 2 + 2, -byte1 - 1, -1.0f, byte0 - 4, byte1, 2, 0.0f);
	boatSides[2]->setRotationPoint(byte0 / 2 - 1, 0 + byte3, 0.0f);
	boatSides[3]->addBox(-byte0 / 2 + 2, -byte1 - 1, -1.0f, byte0 - 4, byte1, 2, 0.0f);
	boatSides[3]->setRotationPoint(0.0f, 0 + byte3, -byte2 / 2 + 1);
	boatSides[4]->addBox(-byte0 / 2 + 2, -byte1 - 1, -1.0f, byte0 - 4, byte1, 2, 0.0f);
	boatSides[4]->setRotationPoint(0.0f, 0 + byte3, byte2 / 2 - 1);
	boatSides[0]->rotateAngleX = 1.5707964f;
	boatSides[1]->rotateAngleY = 4.712389f;
	boatSides[2]->rotateAngleY = 1.5707964f;
	boatSides[3]->rotateAngleY = 3.1415927f;
}

void ModelBoat::render(float f, float f1, float f2, float f3, float f4, float f5)
{
	for (int_t i = 0; i < 5; i++)
	{
		boatSides[i]->render(f5);
	}
}

void ModelBoat::setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5)
{
}
