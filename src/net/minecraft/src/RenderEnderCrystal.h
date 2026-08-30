#pragma once

#include "Render.h"

class EntityEnderCrystal;
class ModelEnderCrystal;

// net.minecraft.src.RenderEnderCrystal
class RenderEnderCrystal : public Render
{
public:
    RenderEnderCrystal();
    ~RenderEnderCrystal() override;
    void doRender(Entity* entity, double x, double y, double z, float yaw, float partialTick) override;

private:
    ModelEnderCrystal* crystalModel;
};
