#include "ItemRenderer.h"
#include "Minecraft.h"
#include "ItemStack.h"
#include "Item.h"
#include "Block.h"
#include "BlockFire.h"
#include "Material.h"
#include "MathHelper.h"
#include "RenderBlocks.h"
#include "MapItemRenderer.h"
#include "OpenGlHelper.h"
#include "Tessellator.h"
#include "RenderHelper.h"
#include "RenderManager.h"
#include "RenderPlayer.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "EntityPlayerSP.h"
#include "MapData.h"
#include "ItemMap.h"
#include "InventoryPlayer.h"
#include "platform/RenderAPI.h"
#include "java/System.h"



namespace {
    bool isRenderableStack(ItemStack* stack) {
        return stack != nullptr && stack->isValid();
    }
}

ItemRenderer::ItemRenderer(Minecraft* minecraft)
    : itemToRender(nullptr)
    , equippedProgress(0.0f)
    , prevEquippedProgress(0.0f)
    , field_20099_f(-1) {
    renderBlocksInstance = new RenderBlocks();
    mc = minecraft;
    field_28131_f = new MapItemRenderer(minecraft->fontRenderer, minecraft->gameSettings, minecraft->renderEngine);
}

ItemRenderer::~ItemRenderer() {
    delete renderBlocksInstance;
    delete field_28131_f;
}

void ItemRenderer::renderItem(EntityLiving* entityliving, ItemStack* itemstack) {
    renderItem(entityliving, itemstack, 0);
}

void ItemRenderer::renderItem(EntityLiving* entityliving, ItemStack* itemstack, int renderPass) {
    if (!isRenderableStack(itemstack)) {
        return;
    }

    Item* item = itemstack->getItem();
    if (item == nullptr) {
        return;
    }

    renderPushMatrix();
    if (itemstack->itemID >= 0 && itemstack->itemID < 256 && Block::blocksList[itemstack->itemID] != nullptr && RenderBlocks::renderItemIn3d(Block::blocksList[itemstack->itemID]->getRenderType())) {
        renderBindTexture(mc->renderEngine->getTexture("/terrain.png"));
        renderBlocksInstance->renderBlockAsItem(Block::blocksList[itemstack->itemID], itemstack->getItemDamage(), 1.0f);
    } else {
        renderBindTexture(mc->renderEngine->getTexture(itemstack->itemID < 256 ? "/terrain.png" : "/gui/items.png"));
        Tessellator* tessellator = &Tessellator::instance;
        int icon = entityliving->getItemIcon(itemstack, renderPass);
        float minU = (static_cast<float>((icon % 16) * 16) + 0.0f) / 256.0f;
        float maxU = (static_cast<float>((icon % 16) * 16) + 15.99f) / 256.0f;
        float minV = (static_cast<float>((icon / 16) * 16) + 0.0f) / 256.0f;
        float maxV = (static_cast<float>((icon / 16) * 16) + 15.99f) / 256.0f;

        renderEnable(RenderCapability::RescaleNormal);
        renderTranslate(0.0f, -0.3f, 0.0f);
        renderScale(1.5f, 1.5f, 1.5f);
        renderRotate(50.0f, 0.0f, 1.0f, 0.0f);
        renderRotate(335.0f, 0.0f, 0.0f, 1.0f);
        renderTranslate(-0.9375f, -0.0625f, 0.0f);
        renderItemIn2D(tessellator, maxU, minV, minU, maxV);

#ifndef PS2_PLATFORM
        // Enchantment glint, see RenderItem::renderItemIntoGUI.
        if (itemstack->hasEffect() && renderPass == 0) {
            renderDepthFunc(RenderCompare::Equal);
            renderDisable(RenderCapability::Lighting);
            renderBindTexture(mc->renderEngine->getTexture("%blur%/misc/glint.png"));
            renderEnable(RenderCapability::Blend);
            renderBlendFunc(RenderBlendFactor::SrcColor, RenderBlendFactor::One);
            const float tint = 0.76f;
            renderColor4f(0.5f * tint, 0.25f * tint, 0.8f * tint, 1.0f);
            renderMatrixMode(RenderMatrixMode::Texture);

            renderPushMatrix();
            const float textureScale = 0.125f;
            renderScale(textureScale, textureScale, textureScale);
            float offset = static_cast<float>(System::currentTimeMillis() % 3000LL) / 3000.0f * 8.0f;
            renderTranslate(offset, 0.0f, 0.0f);
            renderRotate(-50.0f, 0.0f, 0.0f, 1.0f);
            renderItemIn2D(tessellator, 0.0f, 0.0f, 1.0f, 1.0f);
            renderPopMatrix();

            renderPushMatrix();
            renderScale(textureScale, textureScale, textureScale);
            offset = static_cast<float>(System::currentTimeMillis() % 4873LL) / 4873.0f * 8.0f;
            renderTranslate(-offset, 0.0f, 0.0f);
            renderRotate(10.0f, 0.0f, 0.0f, 1.0f);
            renderItemIn2D(tessellator, 0.0f, 0.0f, 1.0f, 1.0f);
            renderPopMatrix();

            renderMatrixMode(RenderMatrixMode::ModelView);
            renderDisable(RenderCapability::Blend);
            renderEnable(RenderCapability::Lighting);
            renderDepthFunc(RenderCompare::LessEqual);
        }
#endif

        renderDisable(RenderCapability::RescaleNormal);
    }
    renderPopMatrix();
}

void ItemRenderer::renderItemIn2D(Tessellator* tessellator, float maxU, float minV, float minU, float maxV) {
    const float size = 1.0f;
    const float thickness = 0.0625f;

    tessellator->startDrawingQuads();
    tessellator->setNormal(0.0f, 0.0f, 1.0f);
    tessellator->addVertexWithUV(0.0, 0.0, 0.0, maxU, maxV);
    tessellator->addVertexWithUV(size, 0.0, 0.0, minU, maxV);
    tessellator->addVertexWithUV(size, 1.0, 0.0, minU, minV);
    tessellator->addVertexWithUV(0.0, 1.0, 0.0, maxU, minV);
    tessellator->draw();

    tessellator->startDrawingQuads();
    tessellator->setNormal(0.0f, 0.0f, -1.0f);
    tessellator->addVertexWithUV(0.0, 1.0, -thickness, maxU, minV);
    tessellator->addVertexWithUV(size, 1.0, -thickness, minU, minV);
    tessellator->addVertexWithUV(size, 0.0, -thickness, minU, maxV);
    tessellator->addVertexWithUV(0.0, 0.0, -thickness, maxU, maxV);
    tessellator->draw();

    tessellator->startDrawingQuads();
    tessellator->setNormal(-1.0f, 0.0f, 0.0f);
    for (int i = 0; i < 16; ++i) {
        float step = static_cast<float>(i) / 16.0f;
        float u = maxU + (minU - maxU) * step - 0.001953125f;
        float x = size * step;
        tessellator->addVertexWithUV(x, 0.0, -thickness, u, maxV);
        tessellator->addVertexWithUV(x, 0.0, 0.0, u, maxV);
        tessellator->addVertexWithUV(x, 1.0, 0.0, u, minV);
        tessellator->addVertexWithUV(x, 1.0, -thickness, u, minV);
    }
    tessellator->draw();

    tessellator->startDrawingQuads();
    tessellator->setNormal(1.0f, 0.0f, 0.0f);
    for (int i = 0; i < 16; ++i) {
        float step = static_cast<float>(i) / 16.0f;
        float u = maxU + (minU - maxU) * step - 0.001953125f;
        float x = size * step + thickness;
        tessellator->addVertexWithUV(x, 1.0, -thickness, u, minV);
        tessellator->addVertexWithUV(x, 1.0, 0.0, u, minV);
        tessellator->addVertexWithUV(x, 0.0, 0.0, u, maxV);
        tessellator->addVertexWithUV(x, 0.0, -thickness, u, maxV);
    }
    tessellator->draw();

    tessellator->startDrawingQuads();
    tessellator->setNormal(0.0f, 1.0f, 0.0f);
    for (int i = 0; i < 16; ++i) {
        float step = static_cast<float>(i) / 16.0f;
        float v = maxV + (minV - maxV) * step - 0.001953125f;
        float y = size * step + thickness;
        tessellator->addVertexWithUV(0.0, y, 0.0, maxU, v);
        tessellator->addVertexWithUV(size, y, 0.0, minU, v);
        tessellator->addVertexWithUV(size, y, -thickness, minU, v);
        tessellator->addVertexWithUV(0.0, y, -thickness, maxU, v);
    }
    tessellator->draw();

    tessellator->startDrawingQuads();
    tessellator->setNormal(0.0f, -1.0f, 0.0f);
    for (int i = 0; i < 16; ++i) {
        float step = static_cast<float>(i) / 16.0f;
        float v = maxV + (minV - maxV) * step - 0.001953125f;
        float y = size * step;
        tessellator->addVertexWithUV(size, y, 0.0, minU, v);
        tessellator->addVertexWithUV(0.0, y, 0.0, maxU, v);
        tessellator->addVertexWithUV(0.0, y, -thickness, maxU, v);
        tessellator->addVertexWithUV(size, y, -thickness, minU, v);
    }
    tessellator->draw();
}

void ItemRenderer::renderItemInFirstPerson(float partialTick) {
    float equipped = prevEquippedProgress + (equippedProgress - prevEquippedProgress) * partialTick;
    EntityPlayerSP* player = mc->thePlayer;
    if (player == nullptr || mc->theWorld == nullptr) {
        return;
    }

    float pitch = player->prevRotationPitch + (player->rotationPitch - player->prevRotationPitch) * partialTick;
    renderPushMatrix();
    renderRotate(pitch, 1.0f, 0.0f, 0.0f);
    renderRotate(player->prevRotationYaw + (player->rotationYaw - player->prevRotationYaw) * partialTick, 0.0f, 1.0f, 0.0f);
    RenderHelper::enableStandardItemLighting();
    renderPopMatrix();

    float armPitch = player->prevRenderArmPitch + (player->renderArmPitch - player->prevRenderArmPitch) * partialTick;
    float armYaw = player->prevRenderArmYaw + (player->renderArmYaw - player->prevRenderArmYaw) * partialTick;
    renderRotate((player->rotationPitch - armPitch) * 0.1f, 1.0f, 0.0f, 0.0f);
    renderRotate((player->rotationYaw - armYaw) * 0.1f, 0.0f, 1.0f, 0.0f);

    ItemStack* itemstack = itemToRender;
    if (!isRenderableStack(itemstack)) {
        itemstack = nullptr;
        itemToRender = nullptr;
    }

    const int blockX = MathHelper::floor_double(player->posX);
    const int blockY = MathHelper::floor_double(player->posY);
    const int blockZ = MathHelper::floor_double(player->posZ);
    float brightness = 1.0f;
    int packedLight = mc->theWorld->getLightBrightnessForSkyBlocks(blockX, blockY, blockZ, 0);
    OpenGlHelper::setLightmapTextureCoords(OpenGlHelper::lightmapTexUnit,
                                            static_cast<float>(packedLight & 0xffff),
                                            static_cast<float>((packedLight >> 16) & 0xffff));
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    Item* heldItem = itemstack != nullptr ? itemstack->getItem() : nullptr;
    if (heldItem != nullptr) {
        int color = heldItem->getColorFromDamage(itemstack->getItemDamage(), 0);
        float red = static_cast<float>(color >> 16 & 0xff) / 255.0f;
        float green = static_cast<float>(color >> 8 & 0xff) / 255.0f;
        float blue = static_cast<float>(color & 0xff) / 255.0f;
        renderColor4f(brightness * red, brightness * green, brightness * blue, 1.0f);
    } else {
        renderColor4f(brightness, brightness, brightness, 1.0f);
    }

    if (itemstack != nullptr && Item::mapItem != nullptr && itemstack->itemID == Item::mapItem->shiftedIndex) {
        renderPushMatrix();
        float mapScale = 0.8f;
        float swing = player->getSwingProgress(partialTick);
        float swingSin = MathHelper::sin(swing * 3.1415927f);
        float swingSqrtSin = MathHelper::sin(MathHelper::sqrt_float(swing) * 3.1415927f);
        renderTranslate(-swingSqrtSin * 0.4f,
                        MathHelper::sin(MathHelper::sqrt_float(swing) * 3.1415927f * 2.0f) * 0.2f,
                        -swingSin * 0.2f);

        float mapPitch = 1.0f - pitch / 45.0f + 0.1f;
        if (mapPitch < 0.0f) mapPitch = 0.0f;
        if (mapPitch > 1.0f) mapPitch = 1.0f;
        mapPitch = -MathHelper::cos(mapPitch * 3.1415927f) * 0.5f + 0.5f;
        renderTranslate(0.0f, -(1.0f - equipped) * 1.2f - mapPitch * 0.5f + 0.04f, -0.9f * mapScale);
        renderRotate(90.0f, 0.0f, 1.0f, 0.0f);
        renderRotate(mapPitch * -85.0f, 0.0f, 0.0f, 1.0f);
        renderEnable(RenderCapability::RescaleNormal);
        renderBindTexture(mc->renderEngine->getTextureForDownloadableImage(player->skinUrl, player->getEntityTexture()));

        for (int arm = 0; arm < 2; ++arm) {
            int side = arm * 2 - 1;
            renderPushMatrix();
            renderTranslate(0.0f, -0.6f, 1.1f * static_cast<float>(side));
            renderRotate(static_cast<float>(-45 * side), 1.0f, 0.0f, 0.0f);
            renderRotate(-90.0f, 0.0f, 0.0f, 1.0f);
            renderRotate(59.0f, 0.0f, 0.0f, 1.0f);
            renderRotate(static_cast<float>(-65 * side), 0.0f, 1.0f, 0.0f);
            Render* render = RenderManager::instance->getEntityRenderObject(player);
            RenderPlayer* playerRenderer = dynamic_cast<RenderPlayer*>(render);
            if (playerRenderer != nullptr) {
                playerRenderer->drawFirstPersonHand();
            }
            renderPopMatrix();
        }

        swing = player->getSwingProgress(partialTick);
        float swingSquaredSin = MathHelper::sin(swing * swing * 3.1415927f);
        swingSqrtSin = MathHelper::sin(MathHelper::sqrt_float(swing) * 3.1415927f);
        renderRotate(-swingSquaredSin * 20.0f, 0.0f, 1.0f, 0.0f);
        renderRotate(-swingSqrtSin * 20.0f, 0.0f, 0.0f, 1.0f);
        renderRotate(-swingSqrtSin * 80.0f, 1.0f, 0.0f, 0.0f);
        renderScale(0.38f, 0.38f, 0.38f);
        renderRotate(90.0f, 0.0f, 1.0f, 0.0f);
        renderRotate(180.0f, 0.0f, 0.0f, 1.0f);
        renderTranslate(-1.0f, -1.0f, 0.0f);
        renderScale(0.015625f, 0.015625f, 0.015625f);
        mc->renderEngine->bindTexture(mc->renderEngine->getTexture("/misc/mapbg.png"));
        Tessellator* tessellator = &Tessellator::instance;
        renderNormal3f(0.0f, 0.0f, -1.0f);
        tessellator->startDrawingQuads();
        const int border = 7;
        tessellator->addVertexWithUV(-border, 128 + border, 0.0, 0.0, 1.0);
        tessellator->addVertexWithUV(128 + border, 128 + border, 0.0, 1.0, 1.0);
        tessellator->addVertexWithUV(128 + border, -border, 0.0, 1.0, 0.0);
        tessellator->addVertexWithUV(-border, -border, 0.0, 0.0, 0.0);
        tessellator->draw();
        MapData* mapdata = static_cast<ItemMap*>(Item::mapItem)->getMapData(itemstack, mc->theWorld);
        field_28131_f->renderMap(player, mc->renderEngine, mapdata);
        renderPopMatrix();
    } else if (itemstack != nullptr && heldItem != nullptr) {
        renderPushMatrix();
        const float handScale = 0.8f;
        EnumAction action = EnumAction::none;
        const bool usingItem = player->getItemInUseCount() > 0;

        if (usingItem) {
            action = itemstack->getItemUseAction();
            if (action == EnumAction::eat || action == EnumAction::drink) {
                float useRemaining = static_cast<float>(player->getItemInUseCount()) - partialTick + 1.0f;
                float useProgress = 1.0f - useRemaining / static_cast<float>(itemstack->getMaxItemUseDuration());
                float ease = 1.0f - useProgress;
                ease *= ease * ease;
                ease *= ease * ease;
                ease *= ease * ease;
                float applied = 1.0f - ease;
                float bob = MathHelper::abs(MathHelper::cos(useRemaining / 4.0f * 3.1415927f) * 0.1f);
                if (useProgress <= 0.2f) {
                    bob = 0.0f;
                }
                renderTranslate(0.0f, bob, 0.0f);
                renderTranslate(applied * 0.6f, -applied * 0.5f, 0.0f);
                renderRotate(applied * 90.0f, 0.0f, 1.0f, 0.0f);
                renderRotate(applied * 10.0f, 1.0f, 0.0f, 0.0f);
                renderRotate(applied * 30.0f, 0.0f, 0.0f, 1.0f);
            }
        } else {
            float swing = player->getSwingProgress(partialTick);
            float swingSin = MathHelper::sin(swing * 3.1415927f);
            float swingSqrtSin = MathHelper::sin(MathHelper::sqrt_float(swing) * 3.1415927f);
            renderTranslate(-swingSqrtSin * 0.4f,
                            MathHelper::sin(MathHelper::sqrt_float(swing) * 3.1415927f * 2.0f) * 0.2f,
                            -swingSin * 0.2f);
        }

        renderTranslate(0.7f * handScale, -0.65f * handScale - (1.0f - equipped) * 0.6f, -0.9f * handScale);
        renderRotate(45.0f, 0.0f, 1.0f, 0.0f);
        renderEnable(RenderCapability::RescaleNormal);

        float swing = player->getSwingProgress(partialTick);
        float swingSquaredSin = MathHelper::sin(swing * swing * 3.1415927f);
        float swingSqrtSin = MathHelper::sin(MathHelper::sqrt_float(swing) * 3.1415927f);
        renderRotate(-swingSquaredSin * 20.0f, 0.0f, 1.0f, 0.0f);
        renderRotate(-swingSqrtSin * 20.0f, 0.0f, 0.0f, 1.0f);
        renderRotate(-swingSqrtSin * 80.0f, 1.0f, 0.0f, 0.0f);
        renderScale(0.4f, 0.4f, 0.4f);

        if (usingItem) {
            if (action == EnumAction::block) {
                renderTranslate(-0.5f, 0.2f, 0.0f);
                renderRotate(30.0f, 0.0f, 1.0f, 0.0f);
                renderRotate(-80.0f, 1.0f, 0.0f, 0.0f);
                renderRotate(60.0f, 0.0f, 1.0f, 0.0f);
            } else if (action == EnumAction::bow) {
                renderRotate(-18.0f, 0.0f, 0.0f, 1.0f);
                renderRotate(-12.0f, 0.0f, 1.0f, 0.0f);
                renderRotate(-8.0f, 1.0f, 0.0f, 0.0f);
                renderTranslate(-0.9f, 0.2f, 0.0f);
                float bowTicks = static_cast<float>(itemstack->getMaxItemUseDuration()) -
                                 (static_cast<float>(player->getItemInUseCount()) - partialTick + 1.0f);
                float bowPull = bowTicks / 20.0f;
                bowPull = (bowPull * bowPull + bowPull * 2.0f) / 3.0f;
                if (bowPull > 1.0f) {
                    bowPull = 1.0f;
                }
                if (bowPull > 0.1f) {
                    renderTranslate(0.0f, MathHelper::sin((bowTicks - 0.1f) * 1.3f) * 0.01f * (bowPull - 0.1f), 0.0f);
                }
                renderTranslate(0.0f, 0.0f, bowPull * 0.1f);
                renderRotate(-335.0f, 0.0f, 0.0f, 1.0f);
                renderRotate(-50.0f, 0.0f, 1.0f, 0.0f);
                renderTranslate(0.0f, 0.5f, 0.0f);
                renderScale(1.0f, 1.0f, 1.0f + bowPull * 0.2f);
                renderTranslate(0.0f, -0.5f, 0.0f);
                renderRotate(50.0f, 0.0f, 1.0f, 0.0f);
                renderRotate(335.0f, 0.0f, 0.0f, 1.0f);
            }
        }

        if (heldItem->shouldRotateAroundWhenRendering()) {
            renderRotate(180.0f, 0.0f, 1.0f, 0.0f);
        }

        if (heldItem->func_46058_c()) {
            renderItem(player, itemstack, 0);
            int color = heldItem->getColorFromDamage(itemstack->getItemDamage(), 1);
            float red = static_cast<float>(color >> 16 & 0xff) / 255.0f;
            float green = static_cast<float>(color >> 8 & 0xff) / 255.0f;
            float blue = static_cast<float>(color & 0xff) / 255.0f;
            renderColor4f(brightness * red, brightness * green, brightness * blue, 1.0f);
            renderItem(player, itemstack, 1);
        } else {
            renderItem(player, itemstack, 0);
        }

        renderPopMatrix();
    } else {
        renderPushMatrix();
        const float handScale = 0.8f;
        float swing = player->getSwingProgress(partialTick);
        float swingSin = MathHelper::sin(swing * 3.1415927f);
        float swingSqrtSin = MathHelper::sin(MathHelper::sqrt_float(swing) * 3.1415927f);
        renderTranslate(-swingSqrtSin * 0.3f,
                        MathHelper::sin(MathHelper::sqrt_float(swing) * 3.1415927f * 2.0f) * 0.4f,
                        -swingSin * 0.4f);
        renderTranslate(0.8f * handScale, -0.75f * handScale - (1.0f - equipped) * 0.6f, -0.9f * handScale);
        renderRotate(45.0f, 0.0f, 1.0f, 0.0f);
        renderEnable(RenderCapability::RescaleNormal);
        swing = player->getSwingProgress(partialTick);
        float swingSquaredSin = MathHelper::sin(swing * swing * 3.1415927f);
        swingSqrtSin = MathHelper::sin(MathHelper::sqrt_float(swing) * 3.1415927f);
        renderRotate(swingSqrtSin * 70.0f, 0.0f, 1.0f, 0.0f);
        renderRotate(-swingSquaredSin * 20.0f, 0.0f, 0.0f, 1.0f);
        renderBindTexture(mc->renderEngine->getTextureForDownloadableImage(player->skinUrl, player->getEntityTexture()));
        renderTranslate(-1.0f, 3.6f, 3.5f);
        renderRotate(120.0f, 0.0f, 0.0f, 1.0f);
        renderRotate(200.0f, 1.0f, 0.0f, 0.0f);
        renderRotate(-135.0f, 0.0f, 1.0f, 0.0f);
        renderTranslate(5.6f, 0.0f, 0.0f);
        Render* render = RenderManager::instance->getEntityRenderObject(player);
        RenderPlayer* playerRenderer = dynamic_cast<RenderPlayer*>(render);
        if (playerRenderer != nullptr) {
            playerRenderer->drawFirstPersonHand();
        }
        renderPopMatrix();
    }

    renderDisable(RenderCapability::RescaleNormal);
    RenderHelper::disableStandardItemLighting();
}

void ItemRenderer::renderOverlays(float f) {
    renderDisable(RenderCapability::AlphaTest);
    if (mc->thePlayer->isBurning()) {
        int i = mc->renderEngine->getTexture("/terrain.png");
        renderBindTexture(i);
        renderFireInFirstPerson(f);
    }
    if (mc->thePlayer->isEntityInsideOpaqueBlock()) {
        int j  = MathHelper::floor_double(mc->thePlayer->posX);
        int l  = MathHelper::floor_double(mc->thePlayer->posY);
        int i1 = MathHelper::floor_double(mc->thePlayer->posZ);
        int j1 = mc->renderEngine->getTexture("/terrain.png");
        renderBindTexture(j1);
        int k1 = mc->theWorld->getBlockId(j, l, i1);
        if (mc->theWorld->isBlockNormalCube(j, l, i1)) {
            renderInsideOfBlock(f, Block::blocksList[k1]->getBlockTextureFromSide(2));
        } else {
            for (int l1 = 0; l1 < 8; l1++) {
                float f1 = ((float)((l1 >> 0) % 2) - 0.5f) * mc->thePlayer->width * 0.9f;
                float f2 = ((float)((l1 >> 1) % 2) - 0.5f) * mc->thePlayer->height * 0.2f;
                float f3 = ((float)((l1 >> 2) % 2) - 0.5f) * mc->thePlayer->width * 0.9f;
                int i2 = MathHelper::floor_float((float)j + f1);
                int j2 = MathHelper::floor_float((float)l + f2);
                int k2 = MathHelper::floor_float((float)i1 + f3);
                if (mc->theWorld->isBlockNormalCube(i2, j2, k2)) {
                    k1 = mc->theWorld->getBlockId(i2, j2, k2);
                }
            }
        }
        if (Block::blocksList[k1] != nullptr) {
            renderInsideOfBlock(f, Block::blocksList[k1]->getBlockTextureFromSide(2));
        }
    }
    if (mc->thePlayer->isInsideOfMaterial(Material::water)) {
        int k = mc->renderEngine->getTexture("/misc/water.png");
        renderBindTexture(k);
        renderWarpedTextureOverlay(f);
    }
    renderEnable(RenderCapability::AlphaTest);
}

void ItemRenderer::renderInsideOfBlock(float f, int i) {
    Tessellator* tessellator = &Tessellator::instance;
    float f1 = mc->thePlayer->getEntityBrightness(f);
    f1 = 0.1f;
    renderColor4f(f1, f1, f1, 0.5f);
    renderPushMatrix();
    float f2 = -1.0f;
    float f3 =  1.0f;
    float f4 = -1.0f;
    float f5 =  1.0f;
    float f6 = -0.5f;
    float f7 = 0.0078125f;
    float f8  = (float)(i % 16) / 256.0f - f7;
    float f9  = ((float)(i % 16) + 15.99f) / 256.0f + f7;
    float f10 = (float)(i / 16) / 256.0f - f7;
    float f11 = ((float)(i / 16) + 15.99f) / 256.0f + f7;
    tessellator->startDrawingQuads();
    tessellator->addVertexWithUV(f2, f4, f6, f9, f11);
    tessellator->addVertexWithUV(f3, f4, f6, f8, f11);
    tessellator->addVertexWithUV(f3, f5, f6, f8, f10);
    tessellator->addVertexWithUV(f2, f5, f6, f9, f10);
    tessellator->draw();
    renderPopMatrix();
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void ItemRenderer::renderWarpedTextureOverlay(float f) {
    Tessellator* tessellator = &Tessellator::instance;
    float f1 = mc->thePlayer->getEntityBrightness(f);
    renderColor4f(f1, f1, f1, 0.5f);
    renderEnable(RenderCapability::Blend);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
    renderPushMatrix();
    float f2 =  4.0f;
    float f3 = -1.0f;
    float f4 =  1.0f;
    float f5 = -1.0f;
    float f6 =  1.0f;
    float f7 = -0.5f;
    float f8 = -mc->thePlayer->rotationYaw / 64.0f;
    float f9 =  mc->thePlayer->rotationPitch / 64.0f;
    tessellator->startDrawingQuads();
    tessellator->addVertexWithUV(f3, f5, f7, f2 + f8, f2 + f9);
    tessellator->addVertexWithUV(f4, f5, f7, 0.0 + f8, f2 + f9);
    tessellator->addVertexWithUV(f4, f6, f7, 0.0 + f8, 0.0 + f9);
    tessellator->addVertexWithUV(f3, f6, f7, f2 + f8, 0.0 + f9);
    tessellator->draw();
    renderPopMatrix();
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    renderDisable(RenderCapability::Blend);
}

void ItemRenderer::renderFireInFirstPerson(float f) {
    Tessellator* tessellator = &Tessellator::instance;
    renderColor4f(1.0f, 1.0f, 1.0f, 0.9f);
    renderEnable(RenderCapability::Blend);
    renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
    float f1 = 1.0f;
    for (int i = 0; i < 2; i++) {
        renderPushMatrix();
        int j = Block::fire->blockIndexInTexture + i * 16;
        int k = (j & 0xf) << 4;
        int l = j & 0xf0;
        float f2 = (float)k / 256.0f;
        float f3 = ((float)k + 15.99f) / 256.0f;
        float f4 = (float)l / 256.0f;
        float f5 = ((float)l + 15.99f) / 256.0f;
        float f6 = (0.0f - f1) / 2.0f;
        float f7 = f6 + f1;
        float f8 = 0.0f - f1 / 2.0f;
        float f9 = f8 + f1;
        float f10 = -0.5f;
        renderTranslate((float)(-(i * 2 - 1)) * 0.24f, -0.3f, 0.0f);
        renderRotate((float)(i * 2 - 1) * 10.0f, 0.0f, 1.0f, 0.0f);
        tessellator->startDrawingQuads();
        tessellator->addVertexWithUV(f6, f8, f10, f3, f5);
        tessellator->addVertexWithUV(f7, f8, f10, f2, f5);
        tessellator->addVertexWithUV(f7, f9, f10, f2, f4);
        tessellator->addVertexWithUV(f6, f9, f10, f3, f4);
        tessellator->draw();
        renderPopMatrix();
    }
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    renderDisable(RenderCapability::Blend);
}

void ItemRenderer::updateEquippedItem() {
    prevEquippedProgress = equippedProgress;
    EntityPlayerSP* entityplayersp = mc->thePlayer;
    EntityPlayer* entityplayer = (EntityPlayer*)entityplayersp;
    ItemStack* itemstack1 = entityplayer->inventory->getCurrentItem();
    if (!isRenderableStack(itemToRender)) {
        itemToRender = nullptr;
    }
    if (!isRenderableStack(itemstack1)) {
        itemstack1 = nullptr;
    }
    bool flag = field_20099_f == entityplayer->inventory->currentItem && itemstack1 == itemToRender;
    if (itemToRender == nullptr && itemstack1 == nullptr) {
        flag = true;
    }
    if (itemstack1 != nullptr && itemToRender != nullptr && itemstack1 != itemToRender &&
        itemstack1->itemID == itemToRender->itemID && itemstack1->getItemDamage() == itemToRender->getItemDamage()) {
        itemToRender = itemstack1;
        flag = true;
    }
    float f = 0.4f;
    float f1 = flag ? 1.0f : 0.0f;
    float f2 = f1 - equippedProgress;
    if (f2 < -f) f2 = -f;
    if (f2 >  f) f2 =  f;
    equippedProgress += f2;
    if (equippedProgress < 0.1f) {
        itemToRender = itemstack1;
        field_20099_f = entityplayer->inventory->currentItem;
    }
}

void ItemRenderer::resetEquippedProgress() {
    equippedProgress = 0.0f;
}

void ItemRenderer::resetEquippedProgressAfterBlockPlace() {
    equippedProgress = 0.0f;
}

void ItemRenderer::resetEquippedProgressAfterItemUse() {
    equippedProgress = 0.0f;
}

void ItemRenderer::refreshItem() {
    if (mc != nullptr && mc->thePlayer != nullptr && mc->thePlayer->inventory != nullptr) {
        itemToRender = mc->thePlayer->inventory->getCurrentItem();
        field_20099_f = mc->thePlayer->inventory->currentItem;
    }
}
