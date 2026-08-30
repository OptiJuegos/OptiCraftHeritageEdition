#ifndef RENDERBIPED_H
#define RENDERBIPED_H

#include "RenderLiving.h"

class ModelBiped;
class EntityLiving;
class ItemStack;

class RenderBiped : public RenderLiving {
public:
    RenderBiped(ModelBiped* modelBiped, float f);

    ModelBiped* modelBipedMain;

protected:
    virtual void renderEquippedItems(EntityLiving* entityLiving, float f);
};

#endif