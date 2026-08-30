#pragma once

#include "RenderLiving.h"

class EntityMooshroom;

// net.minecraft.src.RenderMooshroom
class RenderMooshroom : public RenderLiving
{
public:
    RenderMooshroom(ModelBase* model, float shadowSize);

protected:
    void renderEquippedItems(EntityLiving* entity, float partialTick) override;
};
