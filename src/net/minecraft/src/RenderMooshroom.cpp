#include "RenderMooshroom.h"

#include "Block.h"
#include "BlockFlower.h"
#include "EntityMooshroom.h"
#include "ModelQuadruped.h"
#include "ModelRenderer.h"
#include "platform/RenderAPI.h"

RenderMooshroom::RenderMooshroom(ModelBase* model, float shadow) : RenderLiving(model, shadow)
{
}

void RenderMooshroom::renderEquippedItems(EntityLiving* entity, float partialTick)
{
    RenderLiving::renderEquippedItems(entity, partialTick);
    EntityMooshroom* mooshroom = static_cast<EntityMooshroom*>(entity);
    if (mooshroom->isChild())
        return;

    loadTexture("/terrain.png");
    renderEnable(RenderCapability::CullFace);
    renderPushMatrix();
    renderScale(1.0f, -1.0f, 1.0f);
    renderTranslate(0.2f, 0.4f, 0.5f);
    renderRotate(42.0f, 0.0f, 1.0f, 0.0f);
    renderBlocks->renderBlockAsItem(Block::mushroomRed, 0, 1.0f);
    renderTranslate(0.1f, 0.0f, -0.6f);
    renderRotate(42.0f, 0.0f, 1.0f, 0.0f);
    renderBlocks->renderBlockAsItem(Block::mushroomRed, 0, 1.0f);
    renderPopMatrix();

    renderPushMatrix();
    static_cast<ModelQuadruped*>(mainModel)->head->postRender(1.0f / 16.0f);
    renderScale(1.0f, -1.0f, 1.0f);
    renderTranslate(0.0f, 12.0f / 16.0f, -0.2f);
    renderRotate(12.0f, 0.0f, 1.0f, 0.0f);
    renderBlocks->renderBlockAsItem(Block::mushroomRed, 0, 1.0f);
    renderPopMatrix();
    renderDisable(RenderCapability::CullFace);
}
