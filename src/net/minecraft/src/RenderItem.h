#ifndef RENDERITEM_H
#define RENDERITEM_H

#include "Render.h"
#include "java/Random.h"

class AxisAlignedBB;
class EntityItem;
class FontRenderer;
class RenderEngine;
class ItemStack;

class RenderItem : public Render {
public:
    RenderItem();

    void doRenderItem(EntityItem* entityitem, double d, double d1, double d2, float f, float f1);

    static void renderAABB(AxisAlignedBB *aabb);
    void drawItemIntoGui(FontRenderer* fontrenderer, RenderEngine* renderengine, int i, int j, int k, int l, int i1);
    void renderItemIntoGUI(FontRenderer* fontrenderer, RenderEngine* renderengine, ItemStack* itemstack, int i, int j);
    void renderItemOverlayIntoGUI(FontRenderer* fontrenderer, RenderEngine* renderengine, ItemStack* itemstack, int i, int j);
    void renderTexturedQuad(int i, int j, int k, int l, int i1, int j1);

    virtual void doRender(Entity* entity, double d, double d1, double d2, float f, float f1) override;

    bool field_27004_a;
    float zLevel;

private:
    void renderQuad(Tessellator* tessellator, int i, int j, int k, int l, int i1);
    void renderGuiItemGlint(int seed, int x, int y, int width, int height);

    Random random;
};

#endif
