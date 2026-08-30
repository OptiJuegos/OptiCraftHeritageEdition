#pragma once

#include "Entity.h"

class NBTTagCompound;
class EntityPlayer;

// net.minecraft.src.EntityEnderEye
class EntityEnderEye : public Entity
{
public:
    explicit EntityEnderEye(World *world);
    EntityEnderEye(World *world, double x, double y, double z);

    bool isInRangeToRenderDist(double distance) override;
    void moveTowards(double x, int_t y, double z);
    void setVelocity(double x, double y, double z) override;
    void onUpdate() override;
    void writeEntityToNBT(NBTTagCompound *tag) override;
    void readEntityFromNBT(NBTTagCompound *tag) override;
    void onCollideWithPlayer(EntityPlayer *player) override;
    float getShadowSize() override;
    float getEntityBrightness(float partialTick) override;
    int_t getBrightnessForRender(float partialTick) override;
    bool canAttackWithItem() override;

    int_t field_40096_a = 0;

protected:
    void entityInit() override;

private:
    double targetX = 0.0;
    double targetY = 0.0;
    double targetZ = 0.0;
    int_t despawnTimer = 0;
    bool shatterOrDrop = false;
};
