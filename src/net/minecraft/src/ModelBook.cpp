#include "ModelBook.h"

#include "MathHelper.h"
#include "ModelRenderer.h"

#include <cmath>

ModelBook::ModelBook()
    : coverRight(new ModelRenderer(0, 0))
    , coverLeft(new ModelRenderer(16, 0))
    , pagesRight(new ModelRenderer(0, 10))
    , pagesLeft(new ModelRenderer(12, 10))
    , flippingPageRight(new ModelRenderer(24, 10))
    , flippingPageLeft(new ModelRenderer(24, 10))
    , bookSpine(new ModelRenderer(12, 0))
{
    coverRight->addBox(-6.0f, -5.0f, 0.0f, 6, 10, 0);
    coverLeft->addBox(0.0f, -5.0f, 0.0f, 6, 10, 0);
    pagesRight->addBox(0.0f, -4.0f, -0.99f, 5, 8, 1);
    pagesLeft->addBox(0.0f, -4.0f, -0.01f, 5, 8, 1);
    flippingPageRight->addBox(0.0f, -4.0f, 0.0f, 5, 8, 0);
    flippingPageLeft->addBox(0.0f, -4.0f, 0.0f, 5, 8, 0);
    bookSpine->addBox(-1.0f, -5.0f, 0.0f, 2, 10, 0);

    coverRight->setRotationPoint(0.0f, 0.0f, -1.0f);
    coverLeft->setRotationPoint(0.0f, 0.0f, 1.0f);
    bookSpine->rotateAngleY = 3.14159265358979323846f * 0.5f;
}

ModelBook::~ModelBook()
{
    delete coverRight;
    delete coverLeft;
    delete pagesRight;
    delete pagesLeft;
    delete flippingPageRight;
    delete flippingPageLeft;
    delete bookSpine;
}

void ModelBook::render(float time, float flipRight, float flipLeft, float open, float yaw, float scale)
{
    setRotationAngles(time, flipRight, flipLeft, open, yaw, scale);
    coverRight->render(scale);
    coverLeft->render(scale);
    bookSpine->render(scale);
    pagesRight->render(scale);
    pagesLeft->render(scale);
    flippingPageRight->render(scale);
    flippingPageLeft->render(scale);
}

void ModelBook::setRotationAngles(float time, float flipRight, float flipLeft, float open, float, float)
{
    const float angle = (MathHelper::sin(time * 0.02f) * 0.1f + 1.25f) * open;
    coverRight->rotateAngleY = 3.14159265358979323846f + angle;
    coverLeft->rotateAngleY = -angle;
    pagesRight->rotateAngleY = angle;
    pagesLeft->rotateAngleY = -angle;
    flippingPageRight->rotateAngleY = angle - angle * 2.0f * flipRight;
    flippingPageLeft->rotateAngleY = angle - angle * 2.0f * flipLeft;

    const float pageX = MathHelper::sin(angle);
    pagesRight->rotationPointX = pageX;
    pagesLeft->rotationPointX = pageX;
    flippingPageRight->rotationPointX = pageX;
    flippingPageLeft->rotationPointX = pageX;
}
