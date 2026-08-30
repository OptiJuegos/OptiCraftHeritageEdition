#include "ModelSign.h"

#include "ModelRenderer.h"

ModelSign::ModelSign()
{
    signBoard = new ModelRenderer(this, 0, 0);
    signBoard->addBox(-12.0f, -14.0f, -1.0f, 24, 12, 2, 0.0f);
    signStick = new ModelRenderer(this, 0, 14);
    signStick->addBox(-1.0f, -2.0f, -1.0f, 2, 14, 2, 0.0f);
}

ModelSign::~ModelSign()
{
    delete signBoard;
    delete signStick;
}

void ModelSign::renderSign()
{
    signBoard->render(1.0f / 16.0f);
    signStick->render(1.0f / 16.0f);
}
