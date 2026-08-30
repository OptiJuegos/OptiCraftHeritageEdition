#pragma once

#include "ModelBase.h"

class ModelRenderer;

// net.minecraft.src.ModelSign
class ModelSign : public ModelBase
{
public:
    ModelSign();
    ~ModelSign() override;
    void renderSign();

    ModelRenderer* signBoard;
    ModelRenderer* signStick;
};
