#ifndef RENDERARROW_H
#define RENDERARROW_H

#include "Render.h"

class EntityArrow;

class RenderArrow : public Render {
public:
    RenderArrow();

    void renderArrow(EntityArrow* entityArrow, double d, double d1, double d2, float f, float f1);

    virtual void doRender(Entity* entity, double d, double d1, double d2, float f, float f1) override;

private:
};

#endif