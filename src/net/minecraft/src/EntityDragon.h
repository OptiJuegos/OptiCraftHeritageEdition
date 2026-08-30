#pragma once

#include <array>
#include <vector>

#include "EntityDragonBase.h"

class AxisAlignedBB;
class DamageSource;
class EntityDragonPart;
class EntityEnderCrystal;

// net.minecraft.src.EntityDragon
class EntityDragon : public EntityDragonBase
{
public:
    explicit EntityDragon(World *world);
    ~EntityDragon() override;

    std::array<double, 3> func_40160_a(int_t offset, float partialTick) const;
    void onLivingUpdate() override;
    bool attackEntityFromPart(EntityDragonPart *part, const DamageSource &source, int_t damage) override;
    bool canBeCollidedWith() override;
    int_t func_41010_ax();
    EntityEnderCrystal* getHealingEnderCrystal();

    std::vector<Entity *> getParts() const override;

    double targetX;
    double targetY;
    double targetZ;
    std::array<std::array<double, 3>, 64> field_40162_d;
    int_t field_40164_e;

    EntityDragonPart *dragonPartHead;
    EntityDragonPart *dragonPartBody;
    EntityDragonPart *dragonPartTail1;
    EntityDragonPart *dragonPartTail2;
    EntityDragonPart *dragonPartTail3;
    EntityDragonPart *dragonPartWing1;
    EntityDragonPart *dragonPartWing2;

    float field_40173_aw;
    float field_40172_ax;
    bool field_40163_ay;
    bool field_40161_az;
    int_t field_40178_aA;
    EntityEnderCrystal *healingEnderCrystal;

protected:
    void entityInit() override;
    void onDeathUpdate() override;
    void despawnEntity() override;

private:
    void updateDragonEnderCrystal();
    void func_41007_az();
    void collideWithEntities(const std::vector<Entity *> &entities);
    void attackEntitiesInList(const std::vector<Entity *> &entities);
    void func_41006_aA();
    float simplifyAngle(double angle) const;
    bool destroyBlocksInAABB(AxisAlignedBB *box);
    void createEnderPortal(int_t x, int_t z);
    Entity *resolveTarget();
    EntityEnderCrystal *resolveHealingCrystal();

    int_t targetEntityId;
    int_t healingCrystalEntityId;
};
