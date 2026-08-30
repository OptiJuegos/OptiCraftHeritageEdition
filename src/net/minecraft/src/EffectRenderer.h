#pragma once

#include <memory>
#include <string>
#include <vector>

#include "java/Type.h"
#include "java/Random.h"

class Entity;
class EntityFX;
class EntityDiggingFX;
class RenderEngine;
class Tessellator;
class World;

// net.minecraft.src.EffectRenderer
class EffectRenderer
{
public:
    EffectRenderer(World *world, RenderEngine *renderengine);
    ~EffectRenderer();

    void addEffect(EntityFX *entityfx);
    void updateEffects();
    void renderParticles(Entity *entity, float_t f);
    void renderLitParticles(Entity *entity, float_t f);
    void clearEffects(World *world);
    void addBlockDestroyEffects(int_t i, int_t j, int_t k, int_t l, int_t i1);
    void addBlockHitEffects(int_t i, int_t j, int_t k, int_t l);
    void renderBlockBreakParticles(Entity *entity, float f);
    std::string getStatistics();

protected:
    World *worldObj;

private:
    std::vector<std::unique_ptr<EntityFX>> fxLayers[4];
    RenderEngine            *renderer;
    Random                   rand;
};
