#ifndef RENDERBOAT_H
#define RENDERBOAT_H

#include "Render.h"

class EntityBoat;
class ModelBase;

class RenderBoat : public Render {
public:
    RenderBoat();
    ~RenderBoat() override;

    void renderBoat(EntityBoat* entityBoat, double d, double d1, double d2, float f, float f1);

    virtual void doRender(Entity* entity, double d, double d1, double d2, float f, float f1) override;

protected:
    ModelBase* modelBoat;
};

#endif