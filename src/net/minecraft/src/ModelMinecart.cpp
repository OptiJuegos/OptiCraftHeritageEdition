#include "ModelMinecart.h"

#include "ModelRenderer.h"

ModelMinecart::~ModelMinecart()
{
	delete sideModels[0];
	delete sideModels[1];
	delete sideModels[2];
	delete sideModels[3];
	delete sideModels[4];
	delete sideModels[5];
	delete sideModels[6];
}

ModelMinecart::ModelMinecart()
{
	sideModels[0] = new ModelRenderer(0, 10);
	sideModels[1] = new ModelRenderer(0, 0);
	sideModels[2] = new ModelRenderer(0, 0);
	sideModels[3] = new ModelRenderer(0, 0);
	sideModels[4] = new ModelRenderer(0, 0);
	sideModels[5] = new ModelRenderer(44, 10);
	sideModels[6] = nullptr;
	int_t byte0 = 20;
	int_t byte1 = 8;
	int_t byte2 = 16;
	int_t byte3 = 4;
	sideModels[0]->addBox(-byte0 / 2, -byte2 / 2, -1.0f, byte0, byte2, 2, 0.0f);
	sideModels[0]->setRotationPoint(0.0f, 0 + byte3, 0.0f);
	sideModels[5]->addBox(-byte0 / 2 + 1, -byte2 / 2 + 1, -1.0f, byte0 - 2, byte2 - 2, 1, 0.0f);
	sideModels[5]->setRotationPoint(0.0f, 0 + byte3, 0.0f);
	sideModels[1]->addBox(-byte0 / 2 + 2, -byte1 - 1, -1.0f, byte0 - 4, byte1, 2, 0.0f);
	sideModels[1]->setRotationPoint(-byte0 / 2 + 1, 0 + byte3, 0.0f);
	sideModels[2]->addBox(-byte0 / 2 + 2, -byte1 - 1, -1.0f, byte0 - 4, byte1, 2, 0.0f);
	sideModels[2]->setRotationPoint(byte0 / 2 - 1, 0 + byte3, 0.0f);
	sideModels[3]->addBox(-byte0 / 2 + 2, -byte1 - 1, -1.0f, byte0 - 4, byte1, 2, 0.0f);
	sideModels[3]->setRotationPoint(0.0f, 0 + byte3, -byte2 / 2 + 1);
	sideModels[4]->addBox(-byte0 / 2 + 2, -byte1 - 1, -1.0f, byte0 - 4, byte1, 2, 0.0f);
	sideModels[4]->setRotationPoint(0.0f, 0 + byte3, byte2 / 2 - 1);
	sideModels[0]->rotateAngleX = 1.5707964f;
	sideModels[1]->rotateAngleY = 4.712389f;
	sideModels[2]->rotateAngleY = 1.5707964f;
	sideModels[3]->rotateAngleY = 3.1415927f;
	sideModels[5]->rotateAngleX = -1.5707964f;
}

void ModelMinecart::render(float f, float f1, float f2, float f3, float f4, float f5)
{
	sideModels[5]->rotationPointY = 4.0f - f2;
	for (int_t i = 0; i < 6; i++)
	{
		sideModels[i]->render(f5);
	}
}

void ModelMinecart::setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5)
{
}
