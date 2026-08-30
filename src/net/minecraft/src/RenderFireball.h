#ifndef RENDERFIREBALL_H
#define RENDERFIREBALL_H

#include "Render.h"

class EntityFireball;

class RenderFireball : public Render {
public:
    explicit RenderFireball(float scale);

    void doRenderFireball(EntityFireball* entityfireball, double d, double d1, double d2, float f, float f1);

    virtual void doRender(Entity* entity, double d, double d1, double d2, float f, float f1) override;

private:
    float scale;
};

#endif