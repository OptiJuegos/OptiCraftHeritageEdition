#include "GuiEnchantment.h"

#include "ContainerEnchantment.h"
#include "EnchantmentNameParts.h"
#include "EntityPlayerSP.h"
#include "FontRenderer.h"
#include "GameSettings.h"
#include "InventoryPlayer.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "Minecraft.h"
#include "ModelBook.h"
#include "PlayerController.h"
#include "RenderEngine.h"
#include "RenderHelper.h"
#include "ScaledResolution.h"
#include "Slot.h"
#include "StatCollector.h"
#include "World.h"
#include "platform/RenderAPI.h"

#include <algorithm>
#include <cmath>

namespace
{
ModelBook &getBookModel()
{
    static ModelBook model;
    return model;
}
}

GuiEnchantment::GuiEnchantment(InventoryPlayer *inventory, World *world, int_t x, int_t y, int_t z)
    : GuiContainer(new ContainerEnchantment(inventory, world, x, y, z), true)
    , containerEnchantment(static_cast<ContainerEnchantment *>(inventorySlots))
    , tickCount(0)
    , pageFlip(0.0f)
    , pageFlipPrev(0.0f)
    , pageFlipTarget(0.0f)
    , pageFlipVelocity(0.0f)
    , bookSpreadPrev(0.0f)
    , bookSpread(0.0f)
    , lastStackSnapshot(nullptr)
    , lastStackIdentity(nullptr)
    , lastMouseX(0)
    , lastMouseY(0)
{
}

GuiEnchantment::~GuiEnchantment()
{
    delete lastStackSnapshot;
    lastStackSnapshot = nullptr;
    lastStackIdentity = nullptr;
}

void GuiEnchantment::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    lastMouseX = mouseX;
    lastMouseY = mouseY;
    GuiContainer::drawScreen(mouseX, mouseY, partialTick);
}

void GuiEnchantment::drawGuiContainerForegroundLayer()
{
    fontRenderer->drawString(StatCollector::translateToLocal("container.enchant"), 12, 6, 0x404040);
    fontRenderer->drawString(StatCollector::translateToLocal("container.inventory"), 8, ySize - 96 + 2, 0x404040);
}

void GuiEnchantment::updateScreen()
{
    GuiContainer::updateScreen();
    updateBookAnimation();
}

void GuiEnchantment::mouseClicked(int_t mouseX, int_t mouseY, int_t button)
{
    GuiContainer::mouseClicked(mouseX, mouseY, button);
    if (button != 0 || containerEnchantment == nullptr || mc == nullptr || mc->thePlayer == nullptr)
        return;

    const int_t guiX = (width - xSize) / 2;
    const int_t guiY = (height - ySize) / 2;
    for (int_t option = 0; option < 3; ++option)
    {
        const int_t relativeX = mouseX - (guiX + 60);
        const int_t relativeY = mouseY - (guiY + 14 + 19 * option);
        if (relativeX >= 0 && relativeY >= 0 && relativeX < 108 && relativeY < 19 &&
            containerEnchantment->enchantItem(mc->thePlayer, option))
        {
            mc->playerController->sendEnchantPacket(containerEnchantment->windowId, option);
        }
    }
}

void GuiEnchantment::drawBook(float_t partialTick)
{
    if (mc == nullptr)
        return;

    renderPushMatrix();
    renderMatrixMode(RenderMatrixMode::Projection);
    renderPushMatrix();
    renderLoadIdentity();

    ScaledResolution scaledResolution(mc->gameSettings, mc->displayWidth, mc->displayHeight);
    renderViewport(
        (scaledResolution.getScaledWidth() - 320) / 2 * scaledResolution.scaleFactor,
        (scaledResolution.getScaledHeight() - 240) / 2 * scaledResolution.scaleFactor,
        320 * scaledResolution.scaleFactor,
        240 * scaledResolution.scaleFactor);
    renderTranslate(-0.34f, 0.23f, 0.0f);
    // Java uses gluPerspective(90, 4/3, 9, 80). At 90 degrees the near-plane
    // half-height is exactly zNear; width is scaled by the aspect ratio.
    renderFrustum(-12.0, 12.0, -9.0, 9.0, 9.0, 80.0);

    renderMatrixMode(RenderMatrixMode::ModelView);
    renderLoadIdentity();
    RenderHelper::enableStandardItemLighting();
    renderTranslate(0.0f, 3.3f, -16.0f);
    renderScale(5.0f, 5.0f, 5.0f);
    renderRotate(180.0f, 0.0f, 0.0f, 1.0f);

    mc->renderEngine->bindTexture(mc->renderEngine->getTexture("/item/book.png"));
    renderRotate(20.0f, 1.0f, 0.0f, 0.0f);

    const float spread = bookSpreadPrev + (bookSpread - bookSpreadPrev) * partialTick;
    renderTranslate((1.0f - spread) * 0.2f, (1.0f - spread) * 0.1f, (1.0f - spread) * 0.25f);
    renderRotate(-(1.0f - spread) * 90.0f - 90.0f, 0.0f, 1.0f, 0.0f);
    renderRotate(180.0f, 1.0f, 0.0f, 0.0f);

    float flipRight = pageFlipPrev + (pageFlip - pageFlipPrev) * partialTick + 0.25f;
    float flipLeft = pageFlipPrev + (pageFlip - pageFlipPrev) * partialTick + 0.75f;
    flipRight = (flipRight - static_cast<float>(MathHelper::floor_float(flipRight))) * 1.6f - 0.3f;
    flipLeft = (flipLeft - static_cast<float>(MathHelper::floor_float(flipLeft))) * 1.6f - 0.3f;
    flipRight = std::max(0.0f, std::min(1.0f, flipRight));
    flipLeft = std::max(0.0f, std::min(1.0f, flipLeft));

    renderEnable(RenderCapability::RescaleNormal);
    getBookModel().render(0.0f, flipRight, flipLeft, spread, 0.0f, 1.0f / 16.0f);
    renderDisable(RenderCapability::RescaleNormal);
    RenderHelper::disableStandardItemLighting();

    renderMatrixMode(RenderMatrixMode::Projection);
    renderViewport(0, 0, mc->displayWidth, mc->displayHeight);
    renderPopMatrix();
    renderMatrixMode(RenderMatrixMode::ModelView);
    renderPopMatrix();
    RenderHelper::disableStandardItemLighting();
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void GuiEnchantment::drawGuiContainerBackgroundLayer(float_t partialTick)
{
    if (mc == nullptr || containerEnchantment == nullptr)
        return;

    const int_t texture = mc->renderEngine->getTexture("/gui/enchant.png");
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    mc->renderEngine->bindTexture(texture);
    const int_t guiX = (width - xSize) / 2;
    const int_t guiY = (height - ySize) / 2;
    drawTexturedModalRect(guiX, guiY, 0, 0, xSize, ySize);

    drawBook(partialTick);

    mc->renderEngine->bindTexture(texture);
    EnchantmentNameParts::getInstance().setRandSeed(containerEnchantment->nameSeed);
    FontRenderer *enchantFont = mc->getStandardGalacticFontRenderer();

    for (int_t option = 0; option < 3; ++option)
    {
        const std::string name = EnchantmentNameParts::getInstance().generateRandomEnchantName();
        const int_t level = containerEnchantment->enchantLevels[option];
        renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        const int_t optionX = guiX + 60;
        const int_t optionY = guiY + 14 + 19 * option;
        if (level == 0)
        {
            drawTexturedModalRect(optionX, optionY, 0, 185, 108, 19);
            continue;
        }

        const bool affordable = mc->thePlayer->experienceLevel >= level || mc->thePlayer->capabilities.isCreativeMode;
        int_t nameColor = 0x685e4a;
        int_t levelColor = 0x80ff20;
        if (!affordable)
        {
            drawTexturedModalRect(optionX, optionY, 0, 185, 108, 19);
            nameColor = (nameColor & 0xfefffe) >> 1;
            levelColor = 0x408020;
        }
        else
        {
            const int_t relativeX = lastMouseX - optionX;
            const int_t relativeY = lastMouseY - optionY;
            if (relativeX >= 0 && relativeY >= 0 && relativeX < 108 && relativeY < 19)
            {
                drawTexturedModalRect(optionX, optionY, 0, 204, 108, 19);
                nameColor = 0xffff00;
            }
            else
            {
                drawTexturedModalRect(optionX, optionY, 0, 166, 108, 19);
            }
        }

        enchantFont->drawSplitString(name, optionX + 2, optionY + 2, 104, nameColor);
        const std::string levelText = std::to_string(level);
        fontRenderer->drawStringWithShadow(levelText, optionX + 106 - fontRenderer->getStringWidth(levelText), optionY + 9, levelColor);
    }
}

void GuiEnchantment::updateBookAnimation()
{
    if (inventorySlots == nullptr)
        return;

    Slot *slot = inventorySlots->getSlot(0);
    ItemStack *stack = slot != nullptr ? slot->getStack() : nullptr;
    const bool sameObject = stack != nullptr && stack == lastStackIdentity;
    if (!sameObject && !ItemStack::areItemStacksEqual(stack, lastStackSnapshot))
    {
        delete lastStackSnapshot;
        lastStackSnapshot = stack != nullptr ? stack->copy() : nullptr;
        lastStackIdentity = stack;
        do
        {
            pageFlipTarget += static_cast<float>(animationRandom.nextIntDifference(4));
        }
        while (pageFlip <= pageFlipTarget + 1.0f && pageFlip >= pageFlipTarget - 1.0f);
    }
    else if (stack != lastStackIdentity)
    {
        delete lastStackSnapshot;
        lastStackSnapshot = stack != nullptr ? stack->copy() : nullptr;
        lastStackIdentity = stack;
    }

    ++tickCount;
    pageFlipPrev = pageFlip;
    bookSpreadPrev = bookSpread;

    bool hasOffer = false;
    for (int_t option = 0; option < 3; ++option)
    {
        if (containerEnchantment->enchantLevels[option] != 0)
        {
            hasOffer = true;
            break;
        }
    }

    bookSpread += hasOffer ? 0.2f : -0.2f;
    bookSpread = std::max(0.0f, std::min(1.0f, bookSpread));

    float delta = (pageFlipTarget - pageFlip) * 0.4f;
    delta = std::max(-0.2f, std::min(0.2f, delta));
    pageFlipVelocity += (delta - pageFlipVelocity) * 0.9f;
    pageFlip += pageFlipVelocity;
}
