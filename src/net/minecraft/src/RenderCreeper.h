#ifndef RENDERCREEPER_H
#define RENDERCREEPER_H

#include "RenderLiving.h"

class EntityCreeper;
class ModelBase;
class ModelCreeper;

class RenderCreeper : public RenderLiving {
public:
    RenderCreeper();
    ~RenderCreeper() override;

protected:
    void updateCreeperScale(EntityCreeper* entityCreeper, float f);

    int updateCreeperColorMultiplier(EntityCreeper* entityCreeper, float f, float f1);

    int shouldRenderCreeperPass(EntityCreeper* entityCreeper, int i, float f);

    int inheritCreeperRenderPass(EntityCreeper* entityCreeper, int i, float f);

    virtual void preRenderCallback(EntityLiving* entityLiving, float f) override;

    virtual int getColorMultiplier(EntityLiving* entityLiving, float f, float f1) override;

    virtual int shouldRenderPass(EntityLiving* entityLiving, int i, float f) override;

    virtual int inheritRenderPass(EntityLiving* entityLiving, int i, float f) override;

private:
    ModelBase* chargedCreeperModel;
};

#endif