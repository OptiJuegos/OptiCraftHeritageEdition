#ifndef RENDERPAINTING_H
#define RENDERPAINTING_H

#include "Render.h"

class EntityPainting;
class EnumArt;

class RenderPainting : public Render {
public:
    RenderPainting();

    void renderPainting(EntityPainting* entityPainting, double d, double d1, double d2, float f, float f1);

    virtual void doRender(Entity* entity, double d, double d1, double d2, float f, float f1) override;

private:
    void renderPaintingAt(EntityPainting* entityPainting, float f, float f1, float f2, int i, int j);
    void setPaintingBrightness(EntityPainting* entityPainting, float x, float y);
};

#endif