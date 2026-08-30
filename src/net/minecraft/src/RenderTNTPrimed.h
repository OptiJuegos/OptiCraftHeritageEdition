#ifndef RENDERTNTPRIMED_H
#define RENDERTNTPRIMED_H

#include "Render.h"

class EntityTNTPrimed;
class Tessellator;

class RenderTNTPrimed : public Render {
public:
    RenderTNTPrimed();

    void renderTNTPrimed(EntityTNTPrimed* entityTNTPrimed, double d, double d1, double d2, float f, float f1);

    virtual void doRender(Entity* entity, double d, double d1, double d2, float f, float f1) override;

private:
    void renderBlockOnTNT(float brightness);
};

#endif