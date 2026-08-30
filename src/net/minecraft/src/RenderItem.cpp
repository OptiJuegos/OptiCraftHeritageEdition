#include "RenderItem.h"
#include "java/Arithmetic.h"
#include "java/System.h"

#include <cmath>
#include "AxisAlignedBB.h"
#include "EntityItem.h"
#include "ItemStack.h"
#include "Block.h"
#include "Tessellator.h"
#include "Item.h"
#include "RenderManager.h"
#include "RenderEngine.h"
#include "FontRenderer.h"
#include "MathHelper.h"
#include "platform/RenderAPI.h"



void RenderItem::renderAABB(AxisAlignedBB *aabb)
{
    if (aabb == nullptr)
        return;

    // Match Minecraft Beta 1.7.3 Render.renderAABB exactly.
    // Advanced OpenGL occlusion queries compile this box into glRenderList + 2;
    // if the vertex winding differs, GL_CULL_FACE can make the query report
    // zero samples and whole chunks disappear.
    Tessellator *tessellator = &Tessellator::instance;
    tessellator->startDrawingQuads();

    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->minZ);

    tessellator->addVertex(aabb->minX, aabb->minY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->maxZ);

    tessellator->addVertex(aabb->minX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->maxZ);

    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->minZ);

    tessellator->addVertex(aabb->minX, aabb->minY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->minZ);

    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->maxZ);

    tessellator->draw();
}


RenderItem::RenderItem() {
    random = Random();
    field_27004_a = true;
    zLevel = 0.0f;
    shadowSize = 0.15f;
    shadowOpaque = 0.75f;
}

void RenderItem::doRenderItem(EntityItem* entityitem, double d, double d1, double d2, float f, float f1) {
    random.setSeed(187LL);
    ItemStack* itemstack = entityitem != nullptr ? entityitem->item : nullptr;
    if (itemstack == nullptr || !itemstack->isValid()) {
        return;
    }
    Item* item = itemstack->getItem();
    if (item == nullptr) {
        return;
    }
    renderPushMatrix();
    float f2 = MathHelper::sin(((float)entityitem->age + f1) / 10.0f + entityitem->hoverStart) * 0.1f + 0.1f;
    float f3 = (((float)entityitem->age + f1) / 20.0f + entityitem->hoverStart) * 57.295776f;
    int byte0 = 1;
    if (entityitem->item->stackSize > 1) {
        byte0 = 2;
    }
    if (entityitem->item->stackSize > 5) {
        byte0 = 3;
    }
    if (entityitem->item->stackSize > 20) {
        byte0 = 4;
    }
    renderTranslate((float)d, (float)d1 + f2, (float)d2);
    renderEnable(RenderCapability::RescaleNormal);
    if (itemstack->itemID >= 0 && itemstack->itemID < 256 && Block::blocksList[itemstack->itemID] != nullptr && RenderBlocks::renderItemIn3d(Block::blocksList[itemstack->itemID]->getRenderType())) {
        renderRotate(f3, 0.0f, 1.0f, 0.0f);
        loadTexture("/terrain.png");
        float f4 = 0.25f;
        const int renderType = Block::blocksList[itemstack->itemID]->getRenderType();
        if (renderType == 1 || renderType == 19 || renderType == 12 || renderType == 2) {
            f4 = 0.5f;
        }
        renderScale(f4, f4, f4);
        for (int j = 0; j < byte0; j++) {
            renderPushMatrix();
            if (j > 0) {
                float f5 = ((random.nextFloat() * 2.0f - 1.0f) * 0.2f) / f4;
                float f7 = ((random.nextFloat() * 2.0f - 1.0f) * 0.2f) / f4;
                float f9 = ((random.nextFloat() * 2.0f - 1.0f) * 0.2f) / f4;
                renderTranslate(f5, f7, f9);
            }
            renderBlocks->renderBlockOnInventory(Block::blocksList[itemstack->itemID], itemstack->getItemDamage(), 1.0f);
            renderPopMatrix();
        }
    } else {
        renderScale(0.5f, 0.5f, 0.5f);
        loadTexture("/gui/items.png");
        const int renderPasses = item->func_46058_c() ? 2 : 1;
        for (int renderPass = 0; renderPass < renderPasses; ++renderPass) {
            const int icon = item->func_46058_c()
                ? item->func_46057_a(itemstack->getItemDamage(), renderPass)
                : itemstack->getIconIndex();
            if (!item->func_46058_c() && itemstack->itemID < 256) {
                loadTexture("/terrain.png");
            }

            if (field_27004_a) {
                const int color = item->getColorFromDamage(itemstack->getItemDamage(), renderPass);
                const float red = (float)(color >> 16 & 0xff) / 255.0f;
                const float green = (float)(color >> 8 & 0xff) / 255.0f;
                const float blue = (float)(color & 0xff) / 255.0f;
                renderColor4f(red, green, blue, 1.0f);
            }

            Tessellator* tessellator = &Tessellator::instance;
            const float minU = (float)((icon % 16) * 16) / 256.0f;
            const float maxU = (float)((icon % 16) * 16 + 16) / 256.0f;
            const float minV = (float)((icon / 16) * 16) / 256.0f;
            const float maxV = (float)((icon / 16) * 16 + 16) / 256.0f;
            const float size = 1.0f;
            const float xOffset = 0.5f;
            const float yOffset = 0.25f;
            for (int copy = 0; copy < byte0; ++copy) {
                renderPushMatrix();
                if (copy > 0) {
                    const float offsetX = (random.nextFloat() * 2.0f - 1.0f) * 0.3f;
                    const float offsetY = (random.nextFloat() * 2.0f - 1.0f) * 0.3f;
                    const float offsetZ = (random.nextFloat() * 2.0f - 1.0f) * 0.3f;
                    renderTranslate(offsetX, offsetY, offsetZ);
                }
                renderRotate(180.0f - renderManager->playerViewY, 0.0f, 1.0f, 0.0f);
                tessellator->startDrawingQuads();
                tessellator->setNormal(0.0f, 1.0f, 0.0f);
                tessellator->addVertexWithUV(-xOffset, -yOffset, 0.0f, minU, maxV);
                tessellator->addVertexWithUV(size - xOffset, -yOffset, 0.0f, maxU, maxV);
                tessellator->addVertexWithUV(size - xOffset, 1.0f - yOffset, 0.0f, maxU, minV);
                tessellator->addVertexWithUV(-xOffset, 1.0f - yOffset, 0.0f, minU, minV);
                tessellator->draw();
                renderPopMatrix();
            }
        }
    }
    renderDisable(RenderCapability::RescaleNormal);
    renderPopMatrix();
}

void RenderItem::drawItemIntoGui(FontRenderer* fontrenderer, RenderEngine* renderengine, int i, int j, int k, int l, int i1) {
    if (!ItemStack::isValidItemID(i)) {
        return;
    }
    Item* item = Item::itemsList[i];
    if (i >= 0 && i < 256 && Block::blocksList[i] != nullptr && RenderBlocks::renderItemIn3d(Block::blocksList[i]->getRenderType())) {
        int j1 = i;
        renderengine->bindTexture(renderengine->getTexture("/terrain.png"));
        Block* block = Block::blocksList[j1];
        renderPushMatrix();
        renderTranslate(l - 2, i1 + 3, -3.0f + zLevel);
        renderScale(10.0f, 10.0f, 10.0f);
        renderTranslate(1.0f, 0.5f, 1.0f);
        renderScale(1.0f, 1.0f, -1.0f);
        renderRotate(210.0f, 1.0f, 0.0f, 0.0f);
        renderRotate(45.0f, 0.0f, 1.0f, 0.0f);
        int l1 = item->getColorFromDamage(j);
        float f2 = (float)(l1 >> 16 & 0xff) / 255.0f;
        float f4 = (float)(l1 >> 8 & 0xff) / 255.0f;
        float f5 = (float)(l1 & 0xff) / 255.0f;
        if (field_27004_a) {
            renderColor4f(f2, f4, f5, 1.0f);
        }
        renderRotate(-90.0f, 0.0f, 1.0f, 0.0f);
        renderBlocks->field_31088_b = field_27004_a;
        renderBlocks->renderBlockOnInventory(block, j, 1.0f);
        renderBlocks->field_31088_b = true;
        renderPopMatrix();
    } else if (item->func_46058_c()) {
        renderDisable(RenderCapability::Lighting);
        renderengine->bindTexture(renderengine->getTexture("/gui/items.png"));
        for (int renderPass = 0; renderPass <= 1; ++renderPass) {
            const int icon = item->func_46057_a(j, renderPass);
            const int color = item->getColorFromDamage(j, renderPass);
            const float red = (float)(color >> 16 & 0xff) / 255.0f;
            const float green = (float)(color >> 8 & 0xff) / 255.0f;
            const float blue = (float)(color & 0xff) / 255.0f;
            if (field_27004_a) {
                renderColor4f(red, green, blue, 1.0f);
            }
            renderTexturedQuad(l, i1, (icon % 16) * 16, (icon / 16) * 16, 16, 16);
        }
        renderEnable(RenderCapability::Lighting);
    } else if (k >= 0) {
        renderDisable(RenderCapability::Lighting);
        if (i < 256) {
            renderengine->bindTexture(renderengine->getTexture("/terrain.png"));
        } else {
            renderengine->bindTexture(renderengine->getTexture("/gui/items.png"));
        }
        int k1 = item->getColorFromDamage(j);
        float f = (float)(k1 >> 16 & 0xff) / 255.0f;
        float f1 = (float)(k1 >> 8 & 0xff) / 255.0f;
        float f3 = (float)(k1 & 0xff) / 255.0f;
        if (field_27004_a) {
            renderColor4f(f, f1, f3, 1.0f);
        }
        renderTexturedQuad(l, i1, (k % 16) * 16, (k / 16) * 16, 16, 16);
        renderEnable(RenderCapability::Lighting);
    }
    renderEnable(RenderCapability::CullFace);
}

void RenderItem::renderItemIntoGUI(FontRenderer* fontrenderer, RenderEngine* renderengine, ItemStack* itemstack, int i, int j) {
    if (itemstack == nullptr || !itemstack->isValid()) {
        return;
    }

    drawItemIntoGui(fontrenderer, renderengine, itemstack->itemID, itemstack->getItemDamage(), itemstack->getIconIndex(), i, j);
    if (itemstack->hasEffect()) {
        renderDepthFunc(RenderCompare::Greater);
        renderDisable(RenderCapability::Lighting);
        renderDepthMask(false);
        renderengine->bindTexture(renderengine->getTexture("%blur%/misc/glint.png"));
        zLevel -= 50.0f;
        renderEnable(RenderCapability::Blend);
        renderBlendFunc(RenderBlendFactor::DstColor, RenderBlendFactor::DstColor);
        renderColor4f(0.5f, 0.25f, 0.8f, 1.0f);
        renderGuiItemGlint(i * 431278612 + j * 32178161, i - 2, j - 2, 20, 20);
        renderDisable(RenderCapability::Blend);
        renderDepthMask(true);
        zLevel += 50.0f;
        renderEnable(RenderCapability::Lighting);
        renderDepthFunc(RenderCompare::LessEqual);
    }
}

void RenderItem::renderGuiItemGlint(int seed, int x, int y, int width, int height) {
    (void)seed;
    Tessellator* tessellator = &Tessellator::instance;
    for (int pass = 0; pass < 2; ++pass) {
        renderBlendFunc(RenderBlendFactor::SrcColor, RenderBlendFactor::One);

        const float uScale = 0.00390625f;
        const float vScale = 0.00390625f;
        const long_t period = 3000LL + static_cast<long_t>(pass) * 1873LL;
        const float uOffset = static_cast<float>(System::currentTimeMillis() % period) /
                              (3000.0f + static_cast<float>(pass * 1873)) * 256.0f;
        const float vOffset = 0.0f;
        const float skew = pass == 1 ? -1.0f : 4.0f;

        tessellator->startDrawingQuads();
        tessellator->addVertexWithUV(x, y + height, zLevel,
                                     (uOffset + static_cast<float>(height) * skew) * uScale,
                                     (vOffset + static_cast<float>(height)) * vScale);
        tessellator->addVertexWithUV(x + width, y + height, zLevel,
                                     (uOffset + static_cast<float>(width) + static_cast<float>(height) * skew) * uScale,
                                     (vOffset + static_cast<float>(height)) * vScale);
        tessellator->addVertexWithUV(x + width, y, zLevel,
                                     (uOffset + static_cast<float>(width)) * uScale,
                                     vOffset * vScale);
        tessellator->addVertexWithUV(x, y, zLevel, uOffset * uScale, vOffset * vScale);
        tessellator->draw();
    }
}

void RenderItem::renderItemOverlayIntoGUI(FontRenderer* fontrenderer, RenderEngine* renderengine, ItemStack* itemstack, int i, int j) {
    if (itemstack == nullptr || !itemstack->isValid()) {
        return;
    }
    if (itemstack->stackSize > 1) {
        std::string s = std::to_string(itemstack->stackSize);
        renderDisable(RenderCapability::Lighting);
        renderDisable(RenderCapability::DepthTest);
        fontrenderer->drawStringWithShadow(s, (i + 19) - 2 - fontrenderer->getStringWidth(s), j + 6 + 3, 0xffffff);
        renderEnable(RenderCapability::Lighting);
        renderEnable(RenderCapability::DepthTest);
    }
    if (itemstack->isItemDamaged()) {
        int k = JavaArithmetic::roundFloat(13.0f - ((float)itemstack->getItemDamageForDisplay() * 13.0f) / (float)itemstack->getMaxDamage());
        int l = JavaArithmetic::roundFloat(255.0f - ((float)itemstack->getItemDamageForDisplay() * 255.0f) / (float)itemstack->getMaxDamage());
        renderDisable(RenderCapability::Lighting);
        renderDisable(RenderCapability::DepthTest);
        renderDisable(RenderCapability::Texture2D);
        Tessellator* tessellator = &Tessellator::instance;
        int i1 = (255 - l) << 16 | l << 8;
        int j1 = ((255 - l) / 4) << 16 | 0x3f00;
        renderQuad(tessellator, i + 2, j + 13, 13, 2, 0);
        renderQuad(tessellator, i + 2, j + 13, 12, 1, j1);
        renderQuad(tessellator, i + 2, j + 13, k, 1, i1);
        renderEnable(RenderCapability::Texture2D);
        renderEnable(RenderCapability::Lighting);
        renderEnable(RenderCapability::DepthTest);
        renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

void RenderItem::renderQuad(Tessellator* tessellator, int i, int j, int k, int l, int i1) {
    tessellator->startDrawingQuads();
    tessellator->setColorOpaque_I(i1);
    tessellator->addVertex(i + 0, j + 0, 0.0f);
    tessellator->addVertex(i + 0, j + l, 0.0f);
    tessellator->addVertex(i + k, j + l, 0.0f);
    tessellator->addVertex(i + k, j + 0, 0.0f);
    tessellator->draw();
}

void RenderItem::renderTexturedQuad(int i, int j, int k, int l, int i1, int j1) {
    float f = zLevel;
    float f1 = 0.00390625f;
    float f2 = 0.00390625f;
    Tessellator* tessellator = &Tessellator::instance;
    tessellator->startDrawingQuads();
    tessellator->addVertexWithUV(i + 0, j + j1, f, (float)(k + 0) * f1, (float)(l + j1) * f2);
    tessellator->addVertexWithUV(i + i1, j + j1, f, (float)(k + i1) * f1, (float)(l + j1) * f2);
    tessellator->addVertexWithUV(i + i1, j + 0, f, (float)(k + i1) * f1, (float)(l + 0) * f2);
    tessellator->addVertexWithUV(i + 0, j + 0, f, (float)(k + 0) * f1, (float)(l + 0) * f2);
    tessellator->draw();
}

void RenderItem::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    doRenderItem((EntityItem*)entity, d, d1, d2, f, f1);
}
