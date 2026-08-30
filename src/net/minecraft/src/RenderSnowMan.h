#pragma once

#include "RenderLiving.h"

class EntitySnowman;
class ModelSnowMan;

// net.minecraft.src.RenderSnowMan
class RenderSnowMan : public RenderLiving
{
public:
    RenderSnowMan();

protected:
    void renderEquippedItems(EntityLiving* entity, float partialTick) override;

private:
    ModelSnowMan* snowmanModel;
};
