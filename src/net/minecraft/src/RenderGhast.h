#ifndef RENDERGHAST_H
#define RENDERGHAST_H

#include "RenderLiving.h"

class EntityGhast;

class RenderGhast : public RenderLiving {
public:
    RenderGhast();

protected:
    void preRenderGhastScale(EntityGhast* entityghast, float f);

    virtual void preRenderCallback(EntityLiving* entityliving, float f) override;
};

#endif