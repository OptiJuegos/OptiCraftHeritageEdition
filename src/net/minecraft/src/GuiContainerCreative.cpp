#include "GuiContainerCreative.h"

#include "AchievementList.h"
#include "ContainerCreative.h"
#include "EntityPlayer.h"
#include "EntityPlayerSP.h"
#include "FontRenderer.h"
#include "GameSettings.h"
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
#include "Block.h"
#if defined(CTR_PLATFORM)
#include "3ds/input/DsPadKeyCodes.h"
#endif
#include "BlockFlower.h"
#include "Item.h"
#include "KeyBinding.h"
#include "RenderHelper.h"
#include "RenderItem.h"
#include "SoundManager.h"
#include "pc/lwjgl/Keyboard.h"
#include "pc/lwjgl/Mouse.h"
#include "platform/PlatformConfig.h"
#include "platform/Input.h"
#include "platform/RenderAPI.h"
#if PLATFORM_PS2
#include "ps2/input/Ps2PadKeyCodes.h"
#include "ps2/input/Ps2PadState.h"
#endif

InventoryBasic GuiContainerCreative::inventory("tmp", 72, false);
ItemStack *GuiContainerCreative::s_tabIcons[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
RenderItem *GuiContainerCreative::creativeItemRenderer = new RenderItem();

static const char *s_creativeTabNames[6] = {
    "Building Blocks",
    "Decoration",
    "Redstone & Transport",
    "Materials & Misc",
    "Food & Alchemy",
    "Tools, Weapons & Armor"
};

GuiContainerCreative::GuiContainerCreative(EntityPlayer *player)
    : GuiContainer(new ContainerCreative(player), true, player)
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
        EntityPlayer *p = getContainerPlayer();
        if (p == nullptr && mc != nullptr) p = mc->thePlayer;
        if (mc != nullptr && mc->isSplitScreenActive())
            mc->displayPlayerScreen(getOwnerPlayerIndex(), new GuiInventory(p ? p : mc->thePlayer));
        else
            mc->displayGuiScreen(new GuiInventory(p ? p : mc->thePlayer));
        return;
    }
    const bool legacyCreativeEnabled = (mc == nullptr || mc->gameSettings == nullptr || mc->gameSettings->legacyCreative);
#if PLATFORM_PS2
    const int pIdx = getOwnerPlayerIndex();
    const Ps2PadSnapshot &pad = ps2PadGetSnapshot(pIdx);
    if (pad.connected)
    {
        if (legacyCreativeEnabled && (pad.pressed & PS2_PAD_L1))
        {
            ContainerCreative *container = static_cast<ContainerCreative *>(inventorySlots);
            if (container != nullptr)
            {
                setCategory((container->getCategory() + 5) % 6);
                if (mc != nullptr && mc->sndManager != nullptr)
                    mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
            }
        }
        else if (legacyCreativeEnabled && (pad.pressed & PS2_PAD_R1))
        {
            ContainerCreative *container = static_cast<ContainerCreative *>(inventorySlots);
            if (container != nullptr)
            {
                setCategory((container->getCategory() + 1) % 6);
                if (mc != nullptr && mc->sndManager != nullptr)
                    mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
            }
        }

        // Square button clears held item stack on cursor
        if (pad.pressed & PS2_PAD_SQUARE)
        {
            EntityPlayer *p = getContainerPlayer();
            if (p == nullptr && mc != nullptr) p = mc->thePlayer;
            if (p != nullptr && p->inventory != nullptr && p->inventory->getItemStack() != nullptr)
            {
                delete p->inventory->getItemStack();
                p->inventory->setItemStack(nullptr);
                if (mc != nullptr && mc->sndManager != nullptr)
                    mc->sndManager->playSoundFX("random.pop", 0.6f, 0.8f);
            }
        }
    }
#else
    const PlatformTextInputSnapshot pad = platformTextInputSnapshot(getOwnerPlayerIndex());
    if (pad.connected)
    {
        if (legacyCreativeEnabled && (pad.pressed & PLATFORM_TEXT_PREV_PAGE))
        {
            ContainerCreative *container = static_cast<ContainerCreative *>(inventorySlots);
            if (container != nullptr)
            {
                setCategory((container->getCategory() + 5) % 6);
                if (mc != nullptr && mc->sndManager != nullptr)
                    mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
            }
        }
        else if (legacyCreativeEnabled && (pad.pressed & PLATFORM_TEXT_NEXT_PAGE))
        {
            ContainerCreative *container = static_cast<ContainerCreative *>(inventorySlots);
            if (container != nullptr)
            {
                setCategory((container->getCategory() + 1) % 6);
                if (mc != nullptr && mc->sndManager != nullptr)
                    mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
            }
        }

        // Clear cursor item (X/Y on Classic/GameCube, 2/C on Wiimote/Nunchuk)
        if (pad.pressed & (PLATFORM_TEXT_SPACE | PLATFORM_TEXT_SHIFT))
        {
            EntityPlayer *p = getContainerPlayer();
            if (p == nullptr && mc != nullptr) p = mc->thePlayer;
            if (p != nullptr && p->inventory != nullptr && p->inventory->getItemStack() != nullptr)
            {
                delete p->inventory->getItemStack();
                p->inventory->setItemStack(nullptr);
                if (mc != nullptr && mc->sndManager != nullptr)
                    mc->sndManager->playSoundFX("random.pop", 0.6f, 0.8f);
            }
        }
    }
#endif
    GuiContainer::updateScreen();
}

void GuiContainerCreative::handleMouseClick(Slot *slot, int_t slotId, int_t button, bool shift)
{
    EntityPlayer *p = getContainerPlayer();
    if (p == nullptr && mc != nullptr) p = mc->thePlayer;
    if (p == nullptr) return;
    InventoryPlayer *playerInventory = p->inventory;
    if (playerInventory == nullptr) return;

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

        inventorySlots->slotClick(slot->slotNumber, button, shift, p);
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
        p->dropPlayerItem(held);
        mc->playerController->sendPacketDropItem(held);
    }
    else if (button == 1)
    {
        ItemStack *dropped = held->splitStack(1);
        p->dropPlayerItem(dropped);
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
        EntityPlayer *p = getContainerPlayer();
        if (p == nullptr && mc != nullptr) p = mc->thePlayer;
        if (mc != nullptr && mc->isSplitScreenActive())
            mc->displayPlayerScreen(getOwnerPlayerIndex(), new GuiInventory(p ? p : mc->thePlayer));
        else
            mc->displayGuiScreen(new GuiInventory(p ? p : mc->thePlayer));
        return;
    }

    GuiContainer::initGui();
    for (GuiButton *button : controlList)
        delete button;
    controlList.clear();

    if (s_tabIcons[0] == nullptr && Block::brick != nullptr)
        s_tabIcons[0] = new ItemStack(Block::brick);
    if (s_tabIcons[1] == nullptr && Block::plantRed != nullptr)
        s_tabIcons[1] = new ItemStack(Block::plantRed);
    if (s_tabIcons[2] == nullptr && Item::redstone != nullptr)
        s_tabIcons[2] = new ItemStack(Item::redstone);
    if (s_tabIcons[3] == nullptr && Item::ingotIron != nullptr)
        s_tabIcons[3] = new ItemStack(Item::ingotIron);
    if (s_tabIcons[4] == nullptr && Item::appleRed != nullptr)
        s_tabIcons[4] = new ItemStack(Item::appleRed);
    if (s_tabIcons[5] == nullptr && Item::swordDiamond != nullptr)
        s_tabIcons[5] = new ItemStack(Item::swordDiamond);
}

void GuiContainerCreative::setCategory(int_t tabIndex)
{
    ContainerCreative *container = static_cast<ContainerCreative *>(inventorySlots);
    if (container != nullptr)
    {
        container->setCategory(tabIndex);
        currentScroll = 0.0f;
    }
}

void GuiContainerCreative::mouseClicked(int_t mouseX, int_t mouseY, int_t button)
{
    const bool legacyCreativeEnabled = (mc == nullptr || mc->gameSettings == nullptr || mc->gameSettings->legacyCreative);
    int_t guiLeft = (width - xSize) / 2;
    int_t guiTop = (height - ySize) / 2;
    int_t tabStartX = guiLeft + 7;
    int_t tabY = guiTop - 22;

    if (legacyCreativeEnabled && button == 0 && mouseY >= tabY && mouseY < guiTop)
    {
        for (int_t t = 0; t < 6; ++t)
        {
            int_t tabX = tabStartX + t * 27;
            if (mouseX >= tabX && mouseX < tabX + 27)
            {
                setCategory(t);
                if (mc != nullptr && mc->sndManager != nullptr)
                    mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
                return;
            }
        }
    }

    GuiContainer::mouseClicked(mouseX, mouseY, button);
}

void GuiContainerCreative::keyTyped(char_t c, int_t key)
{
    if (key == 1 || (mc != nullptr && mc->gameSettings != nullptr && mc->gameSettings->keyBindInventory != nullptr && key == mc->gameSettings->keyBindInventory->keyCode))
    {
        EntityPlayer *p = getContainerPlayer();
        if (p == nullptr && mc != nullptr) p = mc->thePlayer;
        if (p != nullptr)
            p->closeScreen();
        return;
    }

    const bool legacyCreativeEnabled = (mc == nullptr || mc->gameSettings == nullptr || mc->gameSettings->legacyCreative);
    if (legacyCreativeEnabled &&
        (key == lwjgl::Keyboard::KEY_PRIOR || key == lwjgl::Keyboard::KEY_Q
#if defined(CTR_PLATFORM)
        || key == DS_KEY_L
#endif
        ))
    {
        ContainerCreative *container = static_cast<ContainerCreative *>(inventorySlots);
        if (container != nullptr)
        {
            setCategory((container->getCategory() + 5) % 6);
            if (mc != nullptr && mc->sndManager != nullptr)
                mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
            return;
        }
    }
    else if (legacyCreativeEnabled &&
        (key == lwjgl::Keyboard::KEY_NEXT || key == lwjgl::Keyboard::KEY_TAB
#if defined(CTR_PLATFORM)
        || key == DS_KEY_R
#endif
        ))
    {
        ContainerCreative *container = static_cast<ContainerCreative *>(inventorySlots);
        if (container != nullptr)
        {
            setCategory((container->getCategory() + 1) % 6);
            if (mc != nullptr && mc->sndManager != nullptr)
                mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
            return;
        }
    }
    else if (key == lwjgl::Keyboard::KEY_X || key == lwjgl::Keyboard::KEY_DELETE)
    {
        EntityPlayer *p = getContainerPlayer();
        if (p == nullptr && mc != nullptr) p = mc->thePlayer;
        if (p != nullptr && p->inventory != nullptr && p->inventory->getItemStack() != nullptr)
        {
            delete p->inventory->getItemStack();
            p->inventory->setItemStack(nullptr);
            if (mc != nullptr && mc->sndManager != nullptr)
                mc->sndManager->playSoundFX("random.pop", 0.6f, 0.8f);
            return;
        }
    }

    GuiContainer::keyTyped(c, key);
}

void GuiContainerCreative::handleMouseInput()
{
    GuiContainer::handleMouseInput();
    int_t wheel = lwjgl::Mouse::getEventDWheel();
    if (wheel == 0)
        return;

    scrollRows(wheel > 0 ? -1 : 1);
}

Slot *GuiContainerCreative::getControllerNavigationTarget(Slot *selected, int_t dirX, int_t dirY)
{
#if PLATFORM_PS2 || PLATFORM_WII
    if (selected == nullptr || inventorySlots == nullptr)
        return nullptr;

    if (selected->getInventory() != &inventory)
    {
        // Hotbar slot navigating up into creative grid
        if (dirY < 0)
        {
            int_t col = selected->slotNumber - 72;
            if (col < 0) col = 0;
            if (col > 7) col = 7;
            return inventorySlots->slots[8 * 8 + col];
        }
        return nullptr;
    }

    const int_t selectedIndex = selected->slotNumber;
    if (selectedIndex < 0 || selectedIndex >= 72)
        return nullptr;

    const int_t column = selectedIndex % 8;
    const int_t row = selectedIndex / 8;
    if (dirX != 0)
    {
        const int_t targetColumn = column + dirX;
        if (targetColumn < 0 || targetColumn >= 8)
            return selected;
        return inventorySlots->slots[row * 8 + targetColumn];
    }

    if (dirY < 0)
    {
        if (row > 0)
            return inventorySlots->slots[(row - 1) * 8 + column];
        scrollRows(-1);
        return selected;
    }

    if (dirY > 0)
    {
        if (row < 8)
            return inventorySlots->slots[(row + 1) * 8 + column];
        const int_t hotbarIndex = 72 + column;
        if (hotbarIndex < (int_t)inventorySlots->slots.size())
            return inventorySlots->slots[hotbarIndex];
        return selected;
    }
#endif
    return nullptr;
}

bool GuiContainerCreative::scrollRows(int_t direction)
{
    if (direction == 0 || inventorySlots == nullptr)
        return false;

    ContainerCreative *container = static_cast<ContainerCreative *>(inventorySlots);
    const int_t rows = (int_t)container->itemList.size() / 8 - 8 + 1;
    if (rows <= 0)
        return false;

    const int_t firstRow = (int_t)((double)(currentScroll * (float_t)rows) + 0.5);
    int_t targetRow = firstRow + direction;
    if (targetRow < 0) targetRow = 0;
    if (targetRow > rows) targetRow = rows;
    if (targetRow == firstRow)
        return false;

    currentScroll = (float_t)targetRow / (float_t)rows;
    container->scrollTo(currentScroll);
    return true;
}

void GuiContainerCreative::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
    bool pointerScrollingAllowed = true;
#if PLATFORM_PS2
    pointerScrollingAllowed = mc == nullptr || mc->gameSettings == nullptr || !mc->gameSettings->legacyUI;
#endif
    bool clicking = pointerScrollingAllowed && lwjgl::Mouse::isButtonDown(0);
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

    // Draw tab tooltip if hovering over tabs
    const bool legacyCreativeEnabled = (mc == nullptr || mc->gameSettings == nullptr || mc->gameSettings->legacyCreative);
    int_t tabStartX = guiLeft + 7;
    int_t tabY = guiTop - 22;
    if (legacyCreativeEnabled && mouseY >= tabY && mouseY < guiTop)
    {
        for (int_t t = 0; t < 6; ++t)
        {
            int_t tabX = tabStartX + t * 27;
            if (mouseX >= tabX && mouseX < tabX + 27)
            {
                drawCreativeTabTooltip(s_creativeTabNames[t], mouseX, mouseY);
                break;
            }
        }
    }

    renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    renderDisable(RenderCapability::Lighting);
}

void GuiContainerCreative::drawGuiContainerForegroundLayer()
{
    ContainerCreative *container = static_cast<ContainerCreative *>(inventorySlots);
    const bool legacyCreativeEnabled = (mc == nullptr || mc->gameSettings == nullptr || mc->gameSettings->legacyCreative);
    const int_t curTab = container ? container->getCategory() : 0;
    const char *title = (legacyCreativeEnabled && curTab >= 0 && curTab < 6) ? s_creativeTabNames[curTab] : "Creative";
    fontRenderer->drawString(title, 8, 6, 0x404040);
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

    drawCategoryTabs(guiLeft, guiTop);
}

void GuiContainerCreative::drawCategoryTabs(int_t guiLeft, int_t guiTop)
{
    if (mc != nullptr && mc->gameSettings != nullptr && !mc->gameSettings->legacyCreative)
        return;

    ContainerCreative *container = static_cast<ContainerCreative *>(inventorySlots);
    const int_t selectedTab = container ? container->getCategory() : 0;
    const int_t tabStartX = guiLeft + 7;

    for (int_t t = 0; t < 6; ++t)
    {
        const int_t tabX = tabStartX + t * 27;
        const bool active = (t == selectedTab);
        const int_t tabY = active ? (guiTop - 21) : (guiTop - 18);
        const int_t tabH = active ? 23 : 19;

        renderDisable(RenderCapability::Lighting);
        // Outer dark border
        drawRect(tabX, tabY, tabX + 26, tabY + tabH, 0xff343434);
        // Fill
        drawRect(tabX + 1, tabY + 1, tabX + 25, tabY + tabH, active ? 0xffc6c6c6 : 0xff9c9c9c);
        // Top and Left highlight
        drawRect(tabX + 1, tabY + 1, tabX + 25, tabY + 2, active ? 0xffffffff : 0xffb8b8b8);
        drawRect(tabX + 1, tabY + 2, tabX + 2, tabY + tabH, active ? 0xffffffff : 0xffb8b8b8);
        // Right shadow
        drawRect(tabX + 24, tabY + 2, tabX + 25, tabY + tabH, active ? 0xff858585 : 0xff505050);

        if (active)
        {
            // Bridge the tab seamlessly into the container top border
            drawRect(tabX + 1, guiTop, tabX + 25, guiTop + 2, 0xffc6c6c6);
        }

        renderEnable(RenderCapability::Lighting);
        if (s_tabIcons[t] != nullptr)
        {
            RenderHelper::enableGUIStandardItemLighting();
            creativeItemRenderer->renderItemIntoGUI(fontRenderer, mc->renderEngine, s_tabIcons[t],
                                                    tabX + 5, tabY + (active ? 3 : 2));
            RenderHelper::disableStandardItemLighting();
        }
        renderDisable(RenderCapability::Lighting);
    }

    // Tab shoulder hints
#if PLATFORM_PS2
    fontRenderer->drawStringWithShadow("L1", tabStartX - 13, guiTop - 14, 0xffe0e0e0);
    fontRenderer->drawStringWithShadow("R1", tabStartX + 6 * 27 + 2, guiTop - 14, 0xffe0e0e0);
#elif PLATFORM_3DS
    // The shoulders arrive as DS_KEY_L/DS_KEY_R (see the keyTyped handlers
    // above): in menus L/R cannot be SPACE/SHIFT, which the on-screen
    // keyboard and the container slot navigator already own.
    fontRenderer->drawStringWithShadow("L", tabStartX - 10, guiTop - 14, 0xffe0e0e0);
    fontRenderer->drawStringWithShadow("R", tabStartX + 6 * 27 + 3, guiTop - 14, 0xffe0e0e0);
#elif PLATFORM_WII
    fontRenderer->drawStringWithShadow("L", tabStartX - 10, guiTop - 14, 0xffe0e0e0);
    fontRenderer->drawStringWithShadow("R", tabStartX + 6 * 27 + 2, guiTop - 14, 0xffe0e0e0);
#else
    fontRenderer->drawStringWithShadow("Q", tabStartX - 9, guiTop - 14, 0xffe0e0e0);
    fontRenderer->drawStringWithShadow("Tab", tabStartX + 6 * 27 + 2, guiTop - 14, 0xffe0e0e0);
#endif
}

void GuiContainerCreative::drawCreativeTabTooltip(const char *text, int_t x, int_t y)
{
    if (text == nullptr || fontRenderer == nullptr)
        return;

    int_t textW = fontRenderer->getStringWidth(text);
    int_t tx = x + 10;
    int_t ty = y - 12;
    if (tx + textW + 6 > width)
        tx = width - textW - 6;
    if (ty < 4)
        ty = 4;

    drawGradientRect(tx - 3, ty - 3, tx + textW + 3, ty + 11, 0xf0100010, 0xf0100010);
    drawGradientRect(tx - 4, ty - 2, tx - 3, ty + 10, 0x505000ff, 0x5028007f);
    drawGradientRect(tx + textW + 3, ty - 2, tx + textW + 4, ty + 10, 0x505000ff, 0x5028007f);
    drawGradientRect(tx - 3, ty - 4, tx + textW + 3, ty - 3, 0x505000ff, 0x505000ff);
    drawGradientRect(tx - 3, ty + 10, tx + textW + 3, ty + 11, 0x5028007f, 0x5028007f);
    fontRenderer->drawStringWithShadow(text, tx, ty, 0xffffffff);
}

void GuiContainerCreative::actionPerformed(GuiButton *button)
{
    if (button->id == 0)
        mc->displayGuiScreen(new GuiAchievements(mc->statFileWriter));
    else if (button->id == 1)
        mc->displayGuiScreen(new GuiStats(this, mc->statFileWriter));
}
