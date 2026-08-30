#ifndef RENDERMINE_H
#define RENDERMINE_H

#include "Render.h"

class EntityMinecart;
class ModelBase;

class RenderMinecart : public Render {
public:
    RenderMinecart();
    ~RenderMinecart() override;

    void renderMinecart(EntityMinecart* entityMinecart, double d, double d1, double d2, float f, float f1);

    virtual void doRender(Entity* entity, double d, double d1, double d2, float f, float f1) override;

private:
    ModelBase* modelMinecart;
};

#endif