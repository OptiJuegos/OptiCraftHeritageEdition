#include "RenderBiped.h"
#include "ModelBiped.h"
#include "EntityLiving.h"
#include "ItemStack.h"
#include "Block.h"
#include "RenderBlocks.h"
#include "Item.h"
#include "RenderManager.h"
#include "ItemRenderer.h"
#include "platform/RenderAPI.h"
#include "ModelRenderer.h"

RenderBiped::RenderBiped(ModelBiped* modelBiped, float f) : RenderLiving(modelBiped, f) {
    modelBipedMain = modelBiped;
}

void RenderBiped::renderEquippedItems(EntityLiving* entityLiving, float f) {
    ItemStack* itemStack = entityLiving->getHeldItem();
    if (itemStack != nullptr && itemStack->isValid()) {
        Item* item = itemStack->getItem();
        if (item == nullptr) return;
        renderPushMatrix();
        modelBipedMain->bipedRightArm->postRender(0.0625f);
        renderTranslate(-0.0625f, 0.4375f, 0.0625f);

        if (itemStack->itemID >= 0 && itemStack->itemID < 256 && Block::blocksList[itemStack->itemID] != nullptr && RenderBlocks::renderItemIn3d(Block::blocksList[itemStack->itemID]->getRenderType())) {
            float f1 = 0.5f;
            renderTranslate(0.0f, 0.1875f, -0.3125f);
            f1 *= 0.75f;
            renderRotate(20.0f, 1.0f, 0.0f, 0.0f);
            renderRotate(45.0f, 0.0f, 1.0f, 0.0f);
            renderScale(f1, -f1, f1);
        } else if (Item::bow != nullptr && itemStack->itemID == Item::bow->shiftedIndex) {
            float f2 = 0.625f;
            renderTranslate(0.0f, 0.125f, 0.3125f);
            renderRotate(-20.0f, 0.0f, 1.0f, 0.0f);
            renderScale(f2, -f2, f2);
            renderRotate(-100.0f, 1.0f, 0.0f, 0.0f);
            renderRotate(45.0f, 0.0f, 1.0f, 0.0f);
        } else if (item->isFull3D()) {
            float f2 = 0.625f;
            renderTranslate(0.0f, 0.1875f, 0.0f);
            renderScale(f2, -f2, f2);
            renderRotate(-100.0f, 1.0f, 0.0f, 0.0f);
            renderRotate(45.0f, 0.0f, 1.0f, 0.0f);
        } else {
            float f3 = 0.375f;
            renderTranslate(0.25f, 0.1875f, -0.1875f);
            renderScale(f3, f3, f3);
            renderRotate(60.0f, 0.0f, 0.0f, 1.0f);
            renderRotate(-90.0f, 1.0f, 0.0f, 0.0f);
            renderRotate(20.0f, 0.0f, 0.0f, 1.0f);
        }

        renderManager->itemRenderer->renderItem(entityLiving, itemStack, 0);
        if (item->func_46058_c()) {
            renderManager->itemRenderer->renderItem(entityLiving, itemStack, 1);
        }
        renderPopMatrix();
    }
}