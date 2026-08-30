#include "SignModel.h"

#include "ModelRenderer.h"

SignModel::SignModel()
{
	signBoard = new ModelRenderer(0, 0);
	signBoard->addBox(-12.0f, -14.0f, -1.0f, 24, 12, 2, 0.0f);
	signStick = new ModelRenderer(0, 14);
	signStick->addBox(-1.0f, -2.0f, -1.0f, 2, 14, 2, 0.0f);
}

SignModel::~SignModel()
{
	delete signBoard;
	delete signStick;
}

void SignModel::renderSign()
{
	signBoard->render(0.0625f);
	signStick->render(0.0625f);
}
