#pragma once

#include "ModelBase.h"

class ModelRenderer;

// net.minecraft.src.ModelEnderCrystal
class ModelEnderCrystal : public ModelBase
{
public:
    explicit ModelEnderCrystal(float scale);
    ~ModelEnderCrystal() override;
    void renderCrystal(float rotation, float bob, float scale);

private:
    ModelRenderer* field_41057_g;
    ModelRenderer* field_41058_h;
    ModelRenderer* field_41059_i;
};
