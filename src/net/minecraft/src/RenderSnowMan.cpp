#include "RenderSnowMan.h"

#include "Block.h"
#include "EntitySnowman.h"
#include "Item.h"
#include "ItemRenderer.h"
#include "ItemStack.h"
#include "ModelSnowMan.h"
#include "ModelRenderer.h"
#include "RenderBlocks.h"
#include "RenderManager.h"
#include "platform/RenderAPI.h"

RenderSnowMan::RenderSnowMan() : RenderLiving(new ModelSnowMan(), 0.5f)
{
    snowmanModel = static_cast<ModelSnowMan*>(mainModel);
    setRenderPassModel(snowmanModel);
}

void RenderSnowMan::renderEquippedItems(EntityLiving* entity, float partialTick)
{
    RenderLiving::renderEquippedItems(entity, partialTick);
    EntitySnowman* snowman = static_cast<EntitySnowman*>(entity);
    ItemStack pumpkin(Block::pumpkin, 1);
    if (!pumpkin.isValid() || pumpkin.getItem() == nullptr || pumpkin.getItem()->shiftedIndex >= 256)
        return;

    renderPushMatrix();
    snowmanModel->field_40305_c->postRender(1.0f / 16.0f);
    if (RenderBlocks::renderItemIn3d(Block::blocksList[pumpkin.itemID]->getRenderType()))
    {
        const float scale = 10.0f / 16.0f;
        renderTranslate(0.0f, -(11.0f / 32.0f), 0.0f);
        renderRotate(180.0f, 0.0f, 1.0f, 0.0f);
        renderScale(scale, -scale, scale);
    }
    if (renderManager != nullptr && renderManager->itemRenderer != nullptr)
        renderManager->itemRenderer->renderItem(snowman, &pumpkin);
    renderPopMatrix();
}
