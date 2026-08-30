#include "RenderFallingSand.h"
#include "EntityFallingSand.h"
#include "Block.h"
#include "World.h"
#include "MathHelper.h"
#include "platform/RenderAPI.h"
#include "java/Arithmetic.h"

RenderFallingSand::RenderFallingSand() {
    shadowSize = 0.5f;
}

void RenderFallingSand::doRenderFallingSand(EntityFallingSand* entityfallingsand, double d, double d1, double d2, float f, float f1) {
    renderPushMatrix();
    renderTranslate((float)d, (float)d1, (float)d2);
    loadTexture("/terrain.png");
    Block* block = Block::blocksList[entityfallingsand->blockID];
    World* world = entityfallingsand->worldObj;
    const int_t blockX = MathHelper::floor_double(entityfallingsand->posX);
    const int_t blockY = MathHelper::floor_double(entityfallingsand->posY);
    const int_t blockZ = MathHelper::floor_double(entityfallingsand->posZ);
    renderDisable(RenderCapability::Lighting);
    if (block == Block::dragonEgg)
    {
        renderBlocks->blockAccess = world;
        Tessellator* tessellator = &Tessellator::instance;
        tessellator->startDrawingQuads();
        tessellator->setTranslationF(static_cast<float>(JavaArithmetic::intSub(0, blockX)) - 0.5f,
                                     static_cast<float>(JavaArithmetic::intSub(0, blockY)) - 0.5f,
                                     static_cast<float>(JavaArithmetic::intSub(0, blockZ)) - 0.5f);
        renderBlocks->renderBlockByRenderType(block, blockX, blockY, blockZ);
        tessellator->setTranslationF(0.0f, 0.0f, 0.0f);
        tessellator->draw();
    }
    else
    {
        renderBlocks->renderBlockFallingSand(block, world, blockX, blockY, blockZ);
    }
    renderEnable(RenderCapability::Lighting);
    renderPopMatrix();
}

void RenderFallingSand::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    doRenderFallingSand((EntityFallingSand*)entity, d, d1, d2, f, f1);
}