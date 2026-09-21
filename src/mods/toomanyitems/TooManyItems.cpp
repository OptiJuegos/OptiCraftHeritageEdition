#include "TooManyItems.h"

#include "Minecraft.h"
#include "GuiContainer.h"
#include "RenderItem.h"
#include "RenderEngine.h"
#include "FontRenderer.h"
#include "SoundManager.h"
#include "EntityPlayer.h"
#include "EntityPlayerSP.h"
#include "InventoryPlayer.h"
#include "ItemStack.h"
#include "Item.h"
#include "Block.h"
#include "World.h"
#include "WorldInfo.h"
#include "FoodStats.h"
#include "Slot.h"
#include "Container.h"
#include "platform/RenderAPI.h"
#include "pc/lwjgl/Keyboard.h"

#if PLATFORM_PS2
#include "ps2/input/Ps2PadState.h"
#endif

bool TooManyItems::s_enabled = true;
bool TooManyItems::s_deleteMode = false;
int_t TooManyItems::s_currentPage = 0;
int_t TooManyItems::s_totalPages = 1;
std::vector<ItemStack*> TooManyItems::s_items;
bool TooManyItems::s_initialized = false;
std::vector<ItemStack*> TooManyItems::s_savedStates[4];
bool TooManyItems::s_stateSaved[4] = { false, false, false, false };

static const int TMI_COLS = 6;
static const int TMI_ROWS = 7;
static const int TMI_ITEMS_PER_PAGE = TMI_COLS * TMI_ROWS; // 42

void TooManyItems::init()
{
    if (s_initialized) return;
    populateItems();
    s_initialized = true;
}

bool TooManyItems::isEnabled()
{
    return s_enabled;
}

void TooManyItems::setEnabled(bool enabled)
{
    s_enabled = enabled;
}

void TooManyItems::toggle()
{
    s_enabled = !s_enabled;
}

bool TooManyItems::isDeleteMode()
{
    return s_deleteMode;
}

void TooManyItems::setDeleteMode(bool del)
{
    s_deleteMode = del;
}

void TooManyItems::populateItems()
{
    for (ItemStack *st : s_items)
        delete st;
    s_items.clear();

    // 1. Bloques y variantes de metadatos
    for (int id = 1; id < 256; ++id)
    {
        Block *b = Block::blocksList[id];
        if (b == nullptr) continue;

        if (Block::cloth && b->blockID == Block::cloth->blockID)
        {
            for (int meta = 0; meta < 16; ++meta)
                s_items.push_back(new ItemStack(b, 1, meta));
        }
        else if ((Block::wood && b->blockID == Block::wood->blockID) ||
                 (Block::leaves && b->blockID == reinterpret_cast<Block*>(Block::leaves)->blockID) ||
                 (Block::sapling && b->blockID == reinterpret_cast<Block*>(Block::sapling)->blockID) ||
                 (Block::planks && b->blockID == Block::planks->blockID))
        {
            for (int meta = 0; meta < 4; ++meta)
                s_items.push_back(new ItemStack(b, 1, meta));
        }
        else if (Block::sandStone && b->blockID == Block::sandStone->blockID)
        {
            for (int meta = 0; meta < 3; ++meta)
                s_items.push_back(new ItemStack(b, 1, meta));
        }
        else if (Block::stoneBrick && b->blockID == Block::stoneBrick->blockID)
        {
            for (int meta = 0; meta < 4; ++meta)
                s_items.push_back(new ItemStack(b, 1, meta));
        }
        else if (Block::stairSingle && b->blockID == Block::stairSingle->blockID)
        {
            for (int meta = 0; meta < 6; ++meta)
                s_items.push_back(new ItemStack(b, 1, meta));
        }
        else if (Block::tallGrass && b->blockID == reinterpret_cast<Block*>(Block::tallGrass)->blockID)
        {
            for (int meta = 1; meta <= 2; ++meta)
                s_items.push_back(new ItemStack(b, 1, meta));
        }
        else
        {
            s_items.push_back(new ItemStack(b, 1, 0));
        }
    }

    // 2. Todos los Items registrados
    for (int id = 256; id < Item::ITEM_LIST_SIZE; ++id)
    {
        Item *item = Item::itemsList[id];
        if (item == nullptr) continue;

        if (Item::dyePowder && item->shiftedIndex == Item::dyePowder->shiftedIndex)
        {
            for (int meta = 0; meta < 16; ++meta)
                s_items.push_back(new ItemStack(item, 1, meta));
        }
        else if (Item::coal && item->shiftedIndex == Item::coal->shiftedIndex)
        {
            s_items.push_back(new ItemStack(item, 1, 0));
            s_items.push_back(new ItemStack(item, 1, 1));
        }
        else if (Item::potion && item->shiftedIndex == Item::potion->shiftedIndex)
        {
            const int potionMeta[] = { 0, 16, 8193, 8194, 8195, 8196, 8201, 8202, 8225, 8226, 16385, 16386, 16393, 16394 };
            for (int meta : potionMeta)
                s_items.push_back(new ItemStack(item, 1, meta));
        }
        else
        {
            s_items.push_back(new ItemStack(item, 1, 0));
        }
    }

    s_totalPages = (static_cast<int>(s_items.size()) + TMI_ITEMS_PER_PAGE - 1) / TMI_ITEMS_PER_PAGE;
    if (s_totalPages < 1) s_totalPages = 1;
}

void TooManyItems::setTime(World *world, int64_t time)
{
    if (world != nullptr)
        world->setWorldTime(time);
}

void TooManyItems::toggleRain(World *world)
{
    if (world != nullptr && world->getWorldInfo() != nullptr)
    {
        bool r = !world->getWorldInfo()->getRaining();
        world->getWorldInfo()->setRaining(r);
        world->getWorldInfo()->setThundering(r);
    }
}

void TooManyItems::toggleCreative(Minecraft *mc)
{
    if (mc != nullptr && mc->thePlayer != nullptr)
    {
        bool c = !mc->thePlayer->capabilities.isCreativeMode;
        mc->thePlayer->capabilities.isCreativeMode = c;
        mc->thePlayer->capabilities.allowFlying = c;
        mc->thePlayer->capabilities.disableDamage = c;
    }
}

void TooManyItems::healPlayer(EntityPlayer *player)
{
    if (player != nullptr)
    {
        player->setEntityHealth(20);
        player->fire = 0;
        if (player->getFoodStats() != nullptr)
        {
            player->getFoodStats()->setFoodLevel(20);
            player->getFoodStats()->setFoodSaturationLevel(20.0f);
        }
    }
}

void TooManyItems::saveInventory(EntityPlayer *player, int slot)
{
    if (player == nullptr || player->inventory == nullptr || slot < 0 || slot >= 4)
        return;

    for (ItemStack *st : s_savedStates[slot])
        delete st;
    s_savedStates[slot].clear();

    for (int i = 0; i < 36; ++i)
    {
        ItemStack *st = player->inventory->mainInventory[i];
        s_savedStates[slot].push_back(st ? ItemStack::copyItemStack(st) : nullptr);
    }
    for (int i = 0; i < 4; ++i)
    {
        ItemStack *st = player->inventory->armorInventory[i];
        s_savedStates[slot].push_back(st ? ItemStack::copyItemStack(st) : nullptr);
    }
    s_stateSaved[slot] = true;
}

void TooManyItems::loadInventory(EntityPlayer *player, int slot)
{
    if (player == nullptr || player->inventory == nullptr || slot < 0 || slot >= 4)
        return;
    if (!s_stateSaved[slot] || s_savedStates[slot].size() < 40)
        return;

    for (int i = 0; i < 36; ++i)
    {
        delete player->inventory->mainInventory[i];
        player->inventory->mainInventory[i] = s_savedStates[slot][i] ? ItemStack::copyItemStack(s_savedStates[slot][i]) : nullptr;
    }
    for (int i = 0; i < 4; ++i)
    {
        delete player->inventory->armorInventory[i];
        player->inventory->armorInventory[i] = s_savedStates[slot][36 + i] ? ItemStack::copyItemStack(s_savedStates[slot][36 + i]) : nullptr;
    }
    player->inventory->inventoryChanged = true;
}

void TooManyItems::updateInput(GuiContainer *gui)
{
    (void)gui;
#if PLATFORM_PS2
    const Ps2PadSnapshot &pad = ps2PadGetSnapshot(0);
    if (pad.pressed & PS2_PAD_R3)
    {
        toggle();
        if (gui && gui->mc && gui->mc->sndManager)
            gui->mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
    }
    if (s_enabled)
    {
        if (pad.pressed & PS2_PAD_L1)
        {
            s_currentPage = (s_currentPage - 1 + s_totalPages) % s_totalPages;
            if (gui && gui->mc && gui->mc->sndManager)
                gui->mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
        }
        else if (pad.pressed & PS2_PAD_R1)
        {
            s_currentPage = (s_currentPage + 1) % s_totalPages;
            if (gui && gui->mc && gui->mc->sndManager)
                gui->mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
        }
    }
#endif
}

void TooManyItems::draw(GuiContainer *gui, int_t mouseX, int_t mouseY)
{
    init();
    updateInput(gui);

    if (gui == nullptr || gui->mc == nullptr) return;

    renderDisable(RenderCapability::Lighting);
    renderDisable(RenderCapability::DepthTest);

    // Botón Toggle [TMI] en la esquina superior derecha del inventario
    int tmiBtnX = gui->guiLeft + gui->xSize - 26;
    int tmiBtnY = gui->guiTop - 13;
    bool tmiBtnHover = (mouseX >= tmiBtnX && mouseX < tmiBtnX + 24 && mouseY >= tmiBtnY && mouseY < tmiBtnY + 11);
    
    gui->drawRect(tmiBtnX, tmiBtnY, tmiBtnX + 24, tmiBtnY + 11, 
                  s_enabled ? (tmiBtnHover ? 0xd0209020 : 0xb0186018) 
                            : (tmiBtnHover ? 0xd0606060 : 0xb0383838));
    gui->drawRect(tmiBtnX, tmiBtnY, tmiBtnX + 24, tmiBtnY + 1, 0xff707070);
    gui->drawCenteredString(gui->fontRenderer, "TMI", tmiBtnX + 12, tmiBtnY + 2, 
                            s_enabled ? 0xaaffaa : 0xdddddd);

    if (!s_enabled)
    {
        renderEnable(RenderCapability::Lighting);
        renderEnable(RenderCapability::DepthTest);
        return;
    }

    // =========================================================================
    // 1. PANEL IZQUIERDO: HERRAMIENTAS Y CHEATS
    // =========================================================================
    int leftW = 74;
    int leftH = 158;
    int leftX = gui->guiLeft - leftW - 4;
    if (leftX < 2) leftX = 2;
    int leftY = 6;

    // Fondo y borde
    gui->drawGradientRect(leftX, leftY, leftX + leftW, leftY + leftH, 0xd0101010, 0xe0181818);
    gui->drawRect(leftX, leftY, leftX + leftW, leftY + 1, 0xff505050);
    gui->drawRect(leftX, leftY + leftH - 1, leftX + leftW, leftY + leftH, 0xff505050);
    gui->drawRect(leftX, leftY, leftX + 1, leftY + leftH, 0xff505050);
    gui->drawRect(leftX + leftW - 1, leftY, leftX + leftW, leftY + leftH, 0xff505050);

    gui->drawCenteredString(gui->fontRenderer, "TMI Cheats", leftX + leftW / 2, leftY + 4, 0xffff80);

    int btnY = leftY + 16;
    int btnH = 12;
    int btnSpacing = 2;

    auto drawTmiButton = [&](int bx, int by, int bw, int bh, const std::string &txt, int col, int hoverCol) {
        bool hov = (mouseX >= bx && mouseX < bx + bw && mouseY >= by && mouseY < by + bh);
        gui->drawRect(bx, by, bx + bw, by + bh, hov ? hoverCol : col);
        gui->drawRect(bx, by, bx + bw, by + 1, 0x60ffffff);
        gui->drawCenteredString(gui->fontRenderer, txt, bx + bw / 2, by + 2, hov ? 0xffffaa : 0xffffff);
        return hov;
    };

    // 1. Trash / Delete Mode
    drawTmiButton(leftX + 4, btnY, leftW - 8, btnH, 
                  s_deleteMode ? "Trash: ON" : "Trash: OFF", 
                  s_deleteMode ? 0xc0aa2020 : 0x80383838, 
                  s_deleteMode ? 0xe0dd3030 : 0xa0585858);
    btnY += btnH + btnSpacing;

    // 2. Time: Day / Noon
    int halfW = (leftW - 10) / 2;
    drawTmiButton(leftX + 4, btnY, halfW, btnH, "Day", 0x80383838, 0xa0585858);
    drawTmiButton(leftX + 6 + halfW, btnY, halfW, btnH, "Noon", 0x80383838, 0xa0585858);
    btnY += btnH + btnSpacing;

    // 3. Time: Night / Rain
    drawTmiButton(leftX + 4, btnY, halfW, btnH, "Night", 0x80383838, 0xa0585858);
    bool isRain = (gui->mc->theWorld && gui->mc->theWorld->getWorldInfo() && gui->mc->theWorld->getWorldInfo()->getRaining());
    drawTmiButton(leftX + 6 + halfW, btnY, halfW, btnH, isRain ? "Rain:ON" : "Rain", isRain ? 0xc0205090 : 0x80383838, 0xa03070b0);
    btnY += btnH + btnSpacing;

    // 4. Mode: Creative / Survival
    bool isCreat = (gui->mc->thePlayer && gui->mc->thePlayer->capabilities.isCreativeMode);
    drawTmiButton(leftX + 4, btnY, leftW - 8, btnH, isCreat ? "Mode: Creat" : "Mode: Surv", 
                  isCreat ? 0xc0308030 : 0x80383838, 0xa040a040);
    btnY += btnH + btnSpacing;

    // 5. Heal & Feed
    drawTmiButton(leftX + 4, btnY, leftW - 8, btnH, "Heal & Feed", 0x80702020, 0xa0903030);
    btnY += btnH + btnSpacing + 4;

    // 6. Save / Load states (Slots 1 y 2)
    gui->drawCenteredString(gui->fontRenderer, "States", leftX + leftW / 2, btnY, 0xcccccc);
    btnY += 10;

    drawTmiButton(leftX + 4, btnY, halfW, btnH, "Save 1", 0x80384838, 0xa0486848);
    drawTmiButton(leftX + 6 + halfW, btnY, halfW, btnH, s_stateSaved[0] ? "Load 1" : "-", 
                  s_stateSaved[0] ? 0x80383858 : 0x40202020, 0xa0484878);
    btnY += btnH + btnSpacing;

    drawTmiButton(leftX + 4, btnY, halfW, btnH, "Save 2", 0x80384838, 0xa0486848);
    drawTmiButton(leftX + 6 + halfW, btnY, halfW, btnH, s_stateSaved[1] ? "Load 2" : "-", 
                  s_stateSaved[1] ? 0x80383858 : 0x40202020, 0xa0484878);

    // =========================================================================
    // 2. PANEL DERECHO: GRILLA DE ITEMS PAGINADA
    // =========================================================================
    int panelW = TMI_COLS * 18 + 4; // 112
    int panelH = TMI_ROWS * 18 + 22; // 148
    int panelX = gui->guiLeft + gui->xSize + 4;
    if (panelX + panelW > gui->width - 2)
        panelX = gui->width - panelW - 2;
    int panelY = 6;

    gui->drawGradientRect(panelX, panelY, panelX + panelW, panelY + panelH, 0xd0101010, 0xe0181818);
    gui->drawRect(panelX, panelY, panelX + panelW, panelY + 1, 0xff505050);
    gui->drawRect(panelX, panelY + panelH - 1, panelX + panelW, panelY + panelH, 0xff505050);
    gui->drawRect(panelX, panelY, panelX + 1, panelY + panelH, 0xff505050);
    gui->drawRect(panelX + panelW - 1, panelY, panelX + panelW, panelY + panelH, 0xff505050);

    // Barra de Navegación de Páginas
    drawTmiButton(panelX + 3, panelY + 3, 14, 12, "<", 0x80383838, 0xa0585858);
    std::string pageStr = std::to_string(s_currentPage + 1) + "/" + std::to_string(s_totalPages);
    gui->drawCenteredString(gui->fontRenderer, pageStr, panelX + panelW / 2, panelY + 5, 0xffffff);
    drawTmiButton(panelX + panelW - 17, panelY + 3, 14, 12, ">", 0x80383838, 0xa0585858);

    // Grilla de Ítems
    int gridStartX = panelX + 2;
    int gridStartY = panelY + 18;
    ItemStack *hoveredStack = nullptr;

    for (int r = 0; r < TMI_ROWS; ++r)
    {
        for (int c = 0; c < TMI_COLS; ++c)
        {
            int slotX = gridStartX + c * 18;
            int slotY = gridStartY + r * 18;
            int idx = s_currentPage * TMI_ITEMS_PER_PAGE + (r * TMI_COLS + c);

            bool isSlotHovered = (mouseX >= slotX && mouseX < slotX + 18 && 
                                  mouseY >= slotY && mouseY < slotY + 18);

            gui->drawRect(slotX, slotY, slotX + 18, slotY + 18, 
                          isSlotHovered ? 0x70ffffff : 0x25000000);

            if (idx < static_cast<int>(s_items.size()))
            {
                ItemStack *stack = s_items[idx];
                if (stack != nullptr)
                {
                    renderEnable(RenderCapability::Lighting);
                    renderEnable(RenderCapability::DepthTest);
                    GuiContainer::itemRenderer->renderItemIntoGUI(gui->fontRenderer, gui->mc->renderEngine, stack, slotX + 1, slotY + 1);
                    GuiContainer::itemRenderer->renderItemOverlayIntoGUI(gui->fontRenderer, gui->mc->renderEngine, stack, slotX + 1, slotY + 1);
                    renderDisable(RenderCapability::Lighting);
                    renderDisable(RenderCapability::DepthTest);

                    if (isSlotHovered)
                        hoveredStack = stack;
                }
            }
        }
    }

    // Tooltip sobre el ítem si el cursor está encima
    if (hoveredStack != nullptr)
    {
        std::vector<std::string> info = hoveredStack->getItemNameandInformation();
        if (!info.empty())
        {
            int tipX = mouseX + 8;
            int tipY = mouseY - 12;
            int tipW = 0;
            for (const auto &str : info)
            {
                int sw = gui->fontRenderer->getStringWidth(str);
                if (sw > tipW) tipW = sw;
            }
            int tipH = 8 + static_cast<int>(info.size()) * 10;
            if (tipX + tipW > gui->width - 4) tipX = gui->width - tipW - 4;
            if (tipY < 4) tipY = 4;

            gui->drawGradientRect(tipX - 3, tipY - 3, tipX + tipW + 3, tipY + tipH, 0xf0100010, 0xf0100010);
            for (size_t i = 0; i < info.size(); ++i)
            {
                gui->fontRenderer->drawStringWithShadow(info[i], tipX, tipY + static_cast<int>(i) * 10, i == 0 ? 0xffffff : 0xaaaaaa);
            }
        }
    }

    renderEnable(RenderCapability::Lighting);
    renderEnable(RenderCapability::DepthTest);
}

bool TooManyItems::mouseClicked(GuiContainer *gui, int_t x, int_t y, int_t button)
{
    if (gui == nullptr || gui->mc == nullptr) return false;

    // Click en botón [TMI]
    int tmiBtnX = gui->guiLeft + gui->xSize - 26;
    int tmiBtnY = gui->guiTop - 13;
    if (x >= tmiBtnX && x < tmiBtnX + 24 && y >= tmiBtnY && y < tmiBtnY + 11)
    {
        toggle();
        if (gui->mc->sndManager)
            gui->mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
        return true;
    }

    if (!s_enabled) return false;

    // =========================================================================
    // Click en Panel Izquierdo (Cheats)
    // =========================================================================
    int leftW = 74;
    int leftH = 158;
    int leftX = gui->guiLeft - leftW - 4;
    if (leftX < 2) leftX = 2;
    int leftY = 6;

    if (x >= leftX && x < leftX + leftW && y >= leftY && y < leftY + leftH)
    {
        int btnY = leftY + 16;
        int btnH = 12;
        int btnSpacing = 2;
        int halfW = (leftW - 10) / 2;

        auto isClicked = [&](int bx, int by, int bw, int bh) {
            return (x >= bx && x < bx + bw && y >= by && y < by + bh);
        };

        // 1. Trash
        if (isClicked(leftX + 4, btnY, leftW - 8, btnH))
        {
            s_deleteMode = !s_deleteMode;
            if (gui->mc->thePlayer && gui->mc->thePlayer->inventory && gui->mc->thePlayer->inventory->getItemStack())
            {
                delete gui->mc->thePlayer->inventory->getItemStack();
                gui->mc->thePlayer->inventory->setItemStack(nullptr);
            }
            if (gui->mc->sndManager) gui->mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
            return true;
        }
        btnY += btnH + btnSpacing;

        // 2. Day / Noon
        if (isClicked(leftX + 4, btnY, halfW, btnH))
        {
            setTime(gui->mc->theWorld, 1000);
            if (gui->mc->sndManager) gui->mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
            return true;
        }
        if (isClicked(leftX + 6 + halfW, btnY, halfW, btnH))
        {
            setTime(gui->mc->theWorld, 6000);
            if (gui->mc->sndManager) gui->mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
            return true;
        }
        btnY += btnH + btnSpacing;

        // 3. Night / Rain
        if (isClicked(leftX + 4, btnY, halfW, btnH))
        {
            setTime(gui->mc->theWorld, 13000);
            if (gui->mc->sndManager) gui->mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
            return true;
        }
        if (isClicked(leftX + 6 + halfW, btnY, halfW, btnH))
        {
            toggleRain(gui->mc->theWorld);
            if (gui->mc->sndManager) gui->mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
            return true;
        }
        btnY += btnH + btnSpacing;

        // 4. Creative
        if (isClicked(leftX + 4, btnY, leftW - 8, btnH))
        {
            toggleCreative(gui->mc);
            if (gui->mc->sndManager) gui->mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
            return true;
        }
        btnY += btnH + btnSpacing;

        // 5. Heal & Feed
        if (isClicked(leftX + 4, btnY, leftW - 8, btnH))
        {
            healPlayer(gui->mc->thePlayer);
            if (gui->mc->sndManager) gui->mc->sndManager->playSoundFX("random.pop", 1.0f, 1.0f);
            return true;
        }
        btnY += btnH + btnSpacing + 14;

        // 6. Save/Load 1
        if (isClicked(leftX + 4, btnY, halfW, btnH))
        {
            saveInventory(gui->mc->thePlayer, 0);
            if (gui->mc->sndManager) gui->mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
            return true;
        }
        if (isClicked(leftX + 6 + halfW, btnY, halfW, btnH))
        {
            loadInventory(gui->mc->thePlayer, 0);
            if (gui->mc->sndManager) gui->mc->sndManager->playSoundFX("random.pop", 1.0f, 1.0f);
            return true;
        }
        btnY += btnH + btnSpacing;

        // 7. Save/Load 2
        if (isClicked(leftX + 4, btnY, halfW, btnH))
        {
            saveInventory(gui->mc->thePlayer, 1);
            if (gui->mc->sndManager) gui->mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
            return true;
        }
        if (isClicked(leftX + 6 + halfW, btnY, halfW, btnH))
        {
            loadInventory(gui->mc->thePlayer, 1);
            if (gui->mc->sndManager) gui->mc->sndManager->playSoundFX("random.pop", 1.0f, 1.0f);
            return true;
        }

        return true;
    }

    // =========================================================================
    // Click en Panel Derecho (Grilla de Ítems)
    // =========================================================================
    int panelW = TMI_COLS * 18 + 4;
    int panelH = TMI_ROWS * 18 + 22;
    int panelX = gui->guiLeft + gui->xSize + 4;
    if (panelX + panelW > gui->width - 2)
        panelX = gui->width - panelW - 2;
    int panelY = 6;

    if (x >= panelX && x < panelX + panelW && y >= panelY && y < panelY + panelH)
    {
        // Botones de página < y >
        if (x >= panelX + 3 && x < panelX + 17 && y >= panelY + 3 && y < panelY + 15)
        {
            s_currentPage = (s_currentPage - 1 + s_totalPages) % s_totalPages;
            if (gui->mc->sndManager) gui->mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
            return true;
        }
        if (x >= panelX + panelW - 17 && x < panelX + panelW - 3 && y >= panelY + 3 && y < panelY + 15)
        {
            s_currentPage = (s_currentPage + 1) % s_totalPages;
            if (gui->mc->sndManager) gui->mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
            return true;
        }

        // Selección de Ítems en la Grilla
        int gridStartX = panelX + 2;
        int gridStartY = panelY + 18;

        for (int r = 0; r < TMI_ROWS; ++r)
        {
            for (int c = 0; c < TMI_COLS; ++c)
            {
                int slotX = gridStartX + c * 18;
                int slotY = gridStartY + r * 18;

                if (x >= slotX && x < slotX + 18 && y >= slotY && y < slotY + 18)
                {
                    int idx = s_currentPage * TMI_ITEMS_PER_PAGE + (r * TMI_COLS + c);
                    if (idx < static_cast<int>(s_items.size()) && s_items[idx] != nullptr)
                    {
                        ItemStack *src = s_items[idx];
                        ItemStack *spawn = ItemStack::copyItemStack(src);
                        if (spawn != nullptr)
                        {
                            // Botón 0 (Izquierdo / X): stack completo de 64
                            // Botón 1 (Derecho / Cuadrado): 1 unidad
                            spawn->stackSize = (button == 0) ? spawn->getMaxStackSize() : 1;

                            if (gui->mc->thePlayer && gui->mc->thePlayer->inventory)
                            {
                                gui->mc->thePlayer->inventory->addItemStackToInventory(spawn);
                            }
                            if (gui->mc->sndManager)
                                gui->mc->sndManager->playSoundFX("random.pop", 1.0f, 1.0f);
                        }
                    }
                    return true;
                }
            }
        }
        return true;
    }

    // =========================================================================
    // Modo Papelera (Delete Mode): Si está activo, clickear un slot lo destruye
    // =========================================================================
    if (s_deleteMode && (button == 0 || button == 1))
    {
        Slot *sl = gui->getSlotAtPosition(x, y);
        if (sl != nullptr && sl->getHasStack())
        {
            delete sl->decrStackSize(sl->getStack()->stackSize);
            sl->putStack(nullptr);
            sl->onSlotChanged();
            if (gui->mc->sndManager)
                gui->mc->sndManager->playSoundFX("random.break", 1.0f, 1.0f);
            return true;
        }
    }

    return false;
}

bool TooManyItems::keyTyped(char_t c, int_t key)
{
    if (key == lwjgl::Keyboard::KEY_O || c == 'o' || c == 'O')
    {
        toggle();
        return true;
    }
    return false;
}
