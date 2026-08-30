#include "ModelGhast.h"

#include "MathHelper.h"
#include "ModelRenderer.h"
#include "java/Random.h"
#include "platform/RenderAPI.h"

ModelGhast::~ModelGhast()
{
	delete body;
	delete tentacles[0];
	delete tentacles[1];
	delete tentacles[2];
	delete tentacles[3];
	delete tentacles[4];
	delete tentacles[5];
	delete tentacles[6];
	delete tentacles[7];
	delete tentacles[8];
}

ModelGhast::ModelGhast()
{
	int_t byte0 = -16;
	body = new ModelRenderer(0, 0);
	body->addBox(-8.0f, -8.0f, -8.0f, 16, 16, 16);
	body->rotationPointY += 24 + byte0;
	Random random(1660LL);
	for (int_t i = 0; i < 9; i++)
	{
		tentacles[i] = new ModelRenderer(0, 0);
		float f = (((((float)(i % 3) - (float)((i / 3) % 2) * 0.5f) + 0.25f) / 2.0f) * 2.0f - 1.0f) * 5.0f;
		float f1 = (((float)(i / 3) / 2.0f) * 2.0f - 1.0f) * 5.0f;
		int_t j = random.nextInt(7) + 8;
		tentacles[i]->addBox(-1.0f, 0.0f, -1.0f, 2, j, 2);
		tentacles[i]->rotationPointX = f;
		tentacles[i]->rotationPointZ = f1;
		tentacles[i]->rotationPointY = 31 + byte0;
	}
}

void ModelGhast::setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5)
{
	for (int_t i = 0; i < 9; i++)
	{
		tentacles[i]->rotateAngleX = 0.2f * MathHelper::sin(f2 * 0.3f + (float)i) + 0.4f;
	}
}

void ModelGhast::render(float f, float f1, float f2, float f3, float f4, float f5)
{
	setRotationAngles(f, f1, f2, f3, f4, f5);
	renderPushMatrix();
	renderTranslate(0.0f, 0.6f, 0.0f);
	body->render(f5);
	for (int_t i = 0; i < 9; i++)
	{
		tentacles[i]->render(f5);
	}
	renderPopMatrix();
}
