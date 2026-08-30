#pragma once

#include "RenderLiving.h"

class EntityVillager;
class ModelVillager;

// net.minecraft.src.RenderVillager
class RenderVillager : public RenderLiving
{
public:
    RenderVillager();

    void doRenderLiving(EntityLiving* entity, double x, double y, double z, float yaw, float partialTick) override;
    void doRender(Entity* entity, double x, double y, double z, float yaw, float partialTick) override;

protected:
    void preRenderCallback(EntityLiving* entity, float partialTick) override;
    int shouldRenderPass(EntityLiving* entity, int pass, float partialTick) override;
    void renderEquippedItems(EntityLiving* entity, float partialTick) override;
    void passSpecialRender(EntityLiving* entity, double x, double y, double z) override;

private:
    void renderVillager(EntityVillager* villager, double x, double y, double z, float yaw, float partialTick);
    ModelVillager* villagerModel;
};
