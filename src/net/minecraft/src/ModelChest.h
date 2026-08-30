#pragma once

#include "ModelBase.h"

class ModelRenderer;

// net.minecraft.src.ModelChest
class ModelChest : public ModelBase
{
public:
    ModelChest();
    ~ModelChest() override;
    virtual void renderAll();

    ModelRenderer* chestLid;
    ModelRenderer* chestBelow;
    ModelRenderer* chestKnob;
};
