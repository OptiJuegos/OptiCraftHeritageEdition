#pragma once

#include "ModelBase.h"

class ModelRenderer;

// net.minecraft.src.ModelBook
class ModelBook : public ModelBase
{
public:
    ModelBook();
    ~ModelBook() override;

    void render(float time, float flipRight, float flipLeft, float open, float yaw, float scale) override;
    void setRotationAngles(float time, float flipRight, float flipLeft, float open, float yaw, float scale) override;

private:
    ModelRenderer *coverRight;
    ModelRenderer *coverLeft;
    ModelRenderer *pagesRight;
    ModelRenderer *pagesLeft;
    ModelRenderer *flippingPageRight;
    ModelRenderer *flippingPageLeft;
    ModelRenderer *bookSpine;
};
