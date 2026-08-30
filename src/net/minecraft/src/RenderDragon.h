#pragma once

#include "RenderLiving.h"

class EntityDragon;
class ModelDragon;

// net.minecraft.src.RenderDragon
class RenderDragon : public RenderLiving
{
public:
    static EntityDragon* entityDragon;

    RenderDragon();

    void doRender(Entity* entity, double x, double y, double z, float yaw, float partialTick) override;
    void doRenderLiving(EntityLiving* entity, double x, double y, double z, float yaw, float partialTick) override;

protected:
    void rotateCorpse(EntityLiving* entity, float age, float yaw, float partialTick) override;
    void renderEquippedItems(EntityLiving* entity, float partialTick) override;
    void renderModel(EntityLiving* entity, float limbSwing, float limbAmount, float age, float headYaw, float headPitch, float scale) override;
    int shouldRenderPass(EntityLiving* entity, int pass, float partialTick) override;

private:
    void renderDragon(EntityDragon* dragon, double x, double y, double z, float yaw, float partialTick);
    void renderHealingBeam(EntityDragon* dragon, double x, double y, double z, float partialTick);
    void renderDragonDying(EntityDragon* dragon, float partialTick);
    void rotateDragonBody(EntityDragon* dragon, float age, float yaw, float partialTick);

    ModelDragon* modelDragon;
};
