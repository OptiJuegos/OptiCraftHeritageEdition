#ifndef RENDERSHEEP_H
#define RENDERSHEEP_H

#include "RenderLiving.h"

class EntitySheep;
class ModelBase;

class RenderSheep : public RenderLiving {
public:
    RenderSheep(ModelBase* modelBase, ModelBase* modelBase2, float shadowSize = 0.7f);
    ~RenderSheep() override;

    void renderSheep(EntitySheep* entitySheep, double d, double d1, double d2, float f, float f1);

    virtual void doRenderLiving(EntityLiving* entityLiving, double d, double d1, double d2, float f, float f1) override;

    virtual void doRender(Entity* entity, double d, double d1, double d2, float f, float f1) override;

protected:
    virtual int getColorMultiplier(EntityLiving* entityLiving, float f, float f1) override;

    int getSheepColorMultiplier(EntitySheep* entitySheep, float f, float f1);

    virtual int shouldRenderPass(EntityLiving* entityLiving, int i, float f) override;

    int renderSheepFurPass(EntitySheep* entitySheep, int i, float f);
};

#endif