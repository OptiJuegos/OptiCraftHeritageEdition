#ifndef RENDERFALLINGSAND_H
#define RENDERFALLINGSAND_H

#include "Render.h"

class EntityFallingSand;

class RenderFallingSand : public Render {
public:
    RenderFallingSand();

    void doRenderFallingSand(EntityFallingSand* entityfallingsand, double d, double d1, double d2, float f, float f1);

    virtual void doRender(Entity* entity, double d, double d1, double d2, float f, float f1) override;

};

#endif