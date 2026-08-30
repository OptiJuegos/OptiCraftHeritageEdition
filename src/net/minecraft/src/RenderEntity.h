#ifndef RENDERENTITY_H
#define RENDERENTITY_H

#include "Render.h"

class Entity;

class RenderEntity : public Render {
public:
    RenderEntity();

    virtual void doRender(Entity* entity, double d, double d1, double d2, float f, float f1) override;
};

#endif