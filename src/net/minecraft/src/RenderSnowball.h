#ifndef RENDERSNOWBALL_H
#define RENDERSNOWBALL_H

#include "Render.h"

class RenderSnowball : public Render {
public:
    explicit RenderSnowball(int_t itemIconIndex);

    virtual void doRender(Entity* entity, double d, double d1, double d2, float f, float f1) override;

private:
    void renderIcon(int_t iconIndex);

    int_t itemIconIndex;
};

#endif
