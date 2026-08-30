#include "ModelEnderCrystal.h"

#include "ModelRenderer.h"
#include "platform/RenderAPI.h"

ModelEnderCrystal::ModelEnderCrystal(float scale)
{
    field_41058_h = new ModelRenderer(this, "glass");
    field_41058_h->setTextureOffset(0, 0)->addBox(-4.0f, -4.0f, -4.0f, 8, 8, 8);
    field_41057_g = new ModelRenderer(this, "cube");
    field_41057_g->setTextureOffset(32, 0)->addBox(-4.0f, -4.0f, -4.0f, 8, 8, 8);
    field_41059_i = new ModelRenderer(this, "base");
    field_41059_i->setTextureOffset(0, 16)->addBox(-6.0f, 0.0f, -6.0f, 12, 4, 12);
}

ModelEnderCrystal::~ModelEnderCrystal()
{
    delete field_41057_g;
    delete field_41058_h;
    delete field_41059_i;
}

void ModelEnderCrystal::renderCrystal(float rotation, float bob, float scale)
{
    renderPushMatrix();
    renderScale(2.0f, 2.0f, 2.0f);
    renderTranslate(0.0f, -0.5f, 0.0f);
    field_41059_i->render(scale);
    renderRotate(rotation, 0.0f, 1.0f, 0.0f);
    renderTranslate(0.0f, 0.8f + bob, 0.0f);
    renderRotate(60.0f, 0.7071f, 0.0f, 0.7071f);
    field_41058_h->render(scale);
    const float innerScale = 14.0f / 16.0f;
    renderScale(innerScale, innerScale, innerScale);
    renderRotate(60.0f, 0.7071f, 0.0f, 0.7071f);
    renderRotate(rotation, 0.0f, 1.0f, 0.0f);
    field_41058_h->render(scale);
    renderScale(innerScale, innerScale, innerScale);
    renderRotate(60.0f, 0.7071f, 0.0f, 0.7071f);
    renderRotate(rotation, 0.0f, 1.0f, 0.0f);
    field_41057_g->render(scale);
    renderPopMatrix();
}
