#include "GuiContainerCreative.h"

#include "AchievementList.h"
#include "ContainerCreative.h"
#include "EntityPlayer.h"
#include "EntityPlayerSP.h"
#include "FontRenderer.h"
#include "GuiAchievements.h"
#include "GuiButton.h"
#include "GuiInventory.h"
#include "GuiStats.h"
#include "InventoryBasic.h"
#include "InventoryPlayer.h"
#include "ItemStack.h"
#include "Minecraft.h"
#include "PlayerController.h"
#include "RenderEngine.h"
#include "Slot.h"
#include "StatCollector.h"
#include "pc/lwjgl/Mouse.h"
#include "platform/RenderAPI.h"

InventoryBasic GuiContainerCreative::inventory("tmp", 72, false);

GuiContainerCreative::GuiContainerCreative(EntityPlayer *player)
    : GuiContainer(new ContainerCreative(player), true)
    , currentScroll(0.0f)
    , isScrolling(false)
    , wasClicking(false)
{
    player->craftingInventory = inventorySlots;
    field_948_f = true;
    player->addStat(AchievementList::openInventory, 1);
    ySize = 208;
}

InventoryBasic *GuiContainerCreative::getInventory()
{
    return &inventory;
}

void GuiContainerCreative::updateScreen()
{
    if (!mc->playerController->isInCreativeMode())
    {
        mc->displayGuiScreen(new GuiInventory(mc->thePlayer));
        return;
    }
    GuiContainer::updateScreen();
}

void GuiContainerCreative::handleMouseClick(Slot *slot, int_t slotId, int_t button, bool shift)
{
    InventoryPlayer *playerInventory = mc->thePlayer->inventory;

    if (slot != nullptr)
    {
        if (slot->getInventory() == &inventory)
        {
            ItemStack *held = playerInventory->getItemStack();
            ItemStack *listed = slot->getStack();

            if (held != nullptr && listed != nullptr && held->itemID == listed->itemID)
            {
                if (button == 0)
                {
                    if (shift)
                        held->stackSize = held->getMaxStackSize();
                    else if (held->stackSize < held->getMaxStackSize())
                        ++held->stackSize;
                }
                else if (held->stackSize <= 1)
                {
                    delete held;
                    playerInventory->setItemStack(nullptr);
                }
                else
                {
                    --held->stackSize;
                }
            }
            else if (held != nullptr)
            {
                delete held;
                playerInventory->setItemStack(nullptr);
            }
            else if (listed == nullptr)
            {
                playerInventory->setItemStack(nullptr);
            }
            else
            {
                ItemStack *copy = ItemStack::copyItemStack(listed);
                if (copy != nullptr && shift)
                    copy->stackSize = copy->getMaxStackSize();
                playerInventory->setItemStack(copy);
            }
            return;
        }

        inventorySlots->slotClick(slot->slotNumber, button, shift, mc->thePlayer);
        ItemStack *stack = inventorySlots->getSlot(slot->slotNumber)->getStack();
        int_t packetSlot = slot->slotNumber - (int_t)inventorySlots->slots.size() + 45;
        mc->playerController->sendSlotPacket(stack, packetSlot);
        return;
    }

    ItemStack *held = playerInventory->getItemStack();
    if (held == nullptr)
        return;

    if (button == 0)
    {
        playerInventory->setItemStack(nullptr);
        mc->thePlayer->dropPlayerItem(held);
        mc->playerController->sendPacketDropItem(held);
    }
    else if (button == 1)
    {
        ItemStack *dropped = held->splitStack(1);
        mc->thePlayer->dropPlayerItem(dropped);
        mc->playerController->sendPacketDropItem(dropped);
        if (held->stackSize == 0)
        {
            delete held;
            playerInventory->setItemStack(nullptr);
        }
    }
}

void GuiContainerCreative::initGui()
{
    if (!mc->playerController->isInCreativeMode())
    {
        mc->displayGuiScreen(new GuiInventory(mc->thePlayer));
        return;
    }

    GuiContainer::initGui();
    for (GuiButton *button : controlList)
        delete button;
    controlList.clear();
}

void GuiContainerCreative::handleMouseInput()
{
    GuiContainer::handleMouseInput();
    int_t wheel = lwjgl::Mouse::getEventDWheel();
    if (wheel == 0)
        return;

    ContainerCreative *container = static_cast<ContainerCreative *>(inventorySlots);
    int_t rows = (int_t)container->itemList.size() / 8 - 8 + 1;
    if (rows <= 0)
        return;

    wheel = wheel > 0 ? 1 : -1;
    currentScroll -= (float_t)wheel / (float_t)rows;
    if (currentScroll < 0.0f) currentScroll = 0.0f;
    if (currentScroll > 1.0f) currentScroll = 1.0f;
    container->scrollTo(currentScroll);
}

void GuiContainerCreative::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    bool clicking = lwjgl::Mouse::isButtonDown(0);
    int_t guiLeft = (width - xSize) / 2;
    int_t guiTop = (height - ySize) / 2;
    int_t scrollLeft = guiLeft + 155;
    int_t scrollTop = guiTop + 17;
    int_t scrollRight = scrollLeft + 14;
    int_t scrollBottom = scrollTop + 162;

    if (!wasClicking && clicking && mouseX >= scrollLeft && mouseY >= scrollTop
        && mouseX < scrollRight && mouseY < scrollBottom)
        isScrolling = true;
    if (!clicking)
        isScrolling = false;
    wasClicking = clicking;

    if (isScrolling)
    {
        currentScroll = (float_t)(mouseY - (scrollTop + 8)) / ((float_t)(scrollBottom - scrollTop) - 16.0f);
        if (currentScroll < 0.0f) currentScroll = 0.0f;
        if (currentScroll > 1.0f) currentScroll = 1.0f;
        static_cast<ContainerCreative *>(inventorySlots)->scrollTo(currentScroll);
    }

    GuiContainer::drawScreen(mouseX, mouseY, partialTick);
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    renderDisable(RenderCapability::Lighting);
}

void GuiContainerCreative::drawGuiContainerForegroundLayer()
{
    fontRenderer->drawString(StatCollector::translateToLocal("container.creative"), 8, 6, 0x404040);
}

void GuiContainerCreative::drawGuiContainerBackgroundLayer(float_t)
{
    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    mc->renderEngine->bindTexture(mc->renderEngine->getTexture("/gui/allitems.png"));
    int_t guiLeft = (width - xSize) / 2;
    int_t guiTop = (height - ySize) / 2;
    drawTexturedModalRect(guiLeft, guiTop, 0, 0, xSize, ySize);
    int_t scrollTop = guiTop + 17;
    int_t scrollBottom = scrollTop + 162;
    drawTexturedModalRect(guiLeft + 154,
                          guiTop + 17 + (int_t)((float_t)(scrollBottom - scrollTop - 17) * currentScroll),
                          0, 208, 16, 16);
}

void GuiContainerCreative::actionPerformed(GuiButton *button)
{
    if (button->id == 0)
        mc->displayGuiScreen(new GuiAchievements(mc->statFileWriter));
    else if (button->id == 1)
        mc->displayGuiScreen(new GuiStats(this, mc->statFileWriter));
}
