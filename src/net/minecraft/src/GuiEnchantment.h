#pragma once

#include "GuiContainer.h"
#include "java/Random.h"


class ContainerEnchantment;
class InventoryPlayer;
class ItemStack;
class ModelBook;
class World;

// net.minecraft.src.GuiEnchantment
class GuiEnchantment : public GuiContainer
{
public:
    GuiEnchantment(InventoryPlayer *inventory, World *world, int_t x, int_t y, int_t z);
    ~GuiEnchantment() override;

    void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;
    void updateScreen() override;

protected:
    void mouseClicked(int_t mouseX, int_t mouseY, int_t button) override;
    void drawGuiContainerForegroundLayer() override;
    void drawGuiContainerBackgroundLayer(float_t partialTick) override;

private:
    void updateBookAnimation();
    void drawBook(float_t partialTick);

    Random animationRandom;
    ContainerEnchantment *containerEnchantment;
    int_t tickCount;
    float pageFlip;
    float pageFlipPrev;
    float pageFlipTarget;
    float pageFlipVelocity;
    float bookSpreadPrev;
    float bookSpread;
    ItemStack *lastStackSnapshot;
    const ItemStack *lastStackIdentity;
    int_t lastMouseX;
    int_t lastMouseY;
};
